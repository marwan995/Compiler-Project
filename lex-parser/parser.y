%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    
    #include "y.tab.h"
    #include "parser.h" // Include the header
    #include "symbol_table.c"
    #include "quadruples.c"
    #include "checkers.c"
    #include "utils.h"

    extern FILE *yyin;
    extern int yylineno;
    extern char *yytext;
    int yylex(void);
    void yywarn(char *s,int line); // Declare yywarn
    void yyerror(char *s);
    int isError = 0;
%}

%union {
    int iValue;          /* integer value */
    float fValue;        /* float value */
    int bValue;          /* boolean value */
    char cValue;         /* char value */
    char *sValue;        /* string value */
    struct ArgList *argList; /* argument list for function calls */
    struct Node *nPtr;
};

%token <iValue> INT_VALUE
%token <fValue> FLOAT_VALUE
%token <bValue> BOOL_VALUE
%token <cValue> CHAR_VALUE
%token <sValue> STRING_VALUE
%type <argList> argument_list


%token INT FLOAT STRING CHAR BOOL CONSTANT VOID VARIABLE

%token FOR WHILE DO BREAK CONTINUE IF ELSE

%token GT LT GE LE EQ NE AND OR NOT ASSIGN

%token INC DEC

%token ADD SUB MUL DIV MOD SEMICOLON

%token FUNCTION RETURN PRINT

%token SWITCH CASE DEFAULT

%nonassoc IFX
%nonassoc ELSE
%nonassoc NOT

%left OR
%left AND
%left EQ NE
%left GT LT GE LE
%left ADD SUB
%left MUL DIV MOD

%type<sValue> type VARIABLE VOID function_type

%type <nPtr> statement_list statement var_declare params expression const_value function_declare declare_list declare operation_expressions  unary_operations
%type <nPtr> non_default_params default_params assign_expression

%%
/*--------------------------------------------------------------------------*/
/* The grammar rules for the parser. */
/*--------------------------------------------------------------------------*/

program:
        declare_list                { }
        | /* NULL */
        ;

/*--------------------------------------------------------------------------*/
/* Declaration */
/*--------------------------------------------------------------------------*/

declare_list: 
     declare
    | declare_list declare
    ;
    
declare: var_declare SEMICOLON 
    | function_declare
    | SEMICOLON   {  };  /* empty statment */
    ;
    
statement_list:
    /* empty */                     {}
    | statement_list statement
    ;

statement:
    var_declare SEMICOLON {}
    | expression SEMICOLON {  }
    | PRINT '(' expression ')' SEMICOLON { quadPrint(); }
    | if_statement {}
    | WHILE { quadLoopInit(); } '(' expression ')' { quadLoopBegin(); } block_structure { quadLoopExit(); }
    | DO { quadLoopInit(); } block_structure WHILE '(' expression ')' { quadLoopBegin(); } SEMICOLON { quadLoopExit(); }
    | FOR '(' for_loop_init SEMICOLON { quadLoopInit(); } expression SEMICOLON { quadLoopBegin(); } for_loop_expression ')' block_structure { quadLoopExit(); }
    | SEMICOLON   {  }  /* empty statment */
    | return_statement { quadJumpCall("_call_"); }
    | BREAK SEMICOLON { quadIsInLoop() ? quadJumpFalseLabel(loopLabels[loopIndex]) : yyerror("break statement not in loop"); }
    | CONTINUE SEMICOLON { quadIsInLoop() ? quadJump(loopLabels[loopIndex - 1]) : yyerror("continue statement not in loop"); }
    | assign_expression SEMICOLON {  }
    | SWITCH '(' expression ')' '{' case_list '}' { printf("switch\n"); }
    | block_structure {}
    ;  

if_statement:
    IF '(' expression ')' { ifLabels[++ifIndex] = labelCounter++; quadJumpFalse(ifLabels[ifIndex]); } block_structure { quadJump(ifLabels[ifIndex]); quadFalseLabel(ifLabels[ifIndex]);} else_block { quadLabel(ifLabels[ifIndex--]); }

else_block:
    ELSE {  } block_structure {  }
    | ELSE { } if_statement { }
    | { } 
    ;

case_list: 
    case_list CASE const_value ':' block_structure {  }
    | case_list DEFAULT ':' block_structure {  }
    |                           {}
    ;


/*--------------------------------------------------------------------------*/
/* Block Structure */
/*--------------------------------------------------------------------------*/

block_structure: 
    '{' { enterScope(yylineno); printParms();} 
    statement_list  
    '}' {checkForUnusedVars(); exitScope(yylineno);  }
    ;
    
/*--------------------------------------------------------------------------*/
/* For loop */
/*--------------------------------------------------------------------------*/

for_loop_init:
    VARIABLE ASSIGN expression { validateAssignmentType(getSymbolDataType($1), $3); validateNotConst($1); insertForLoopVar($1, "var", NULL, yylineno);}
    | type VARIABLE ASSIGN expression /*int x = 1;*/ { validateAssignmentType($1, $4) ? insertForLoopVar($2, "var", $1, yylineno): yyerror("Type mismatch in assignment");  }
    ;

for_loop_expression:
    assign_expression {  }
    | unary_operations {  }
    ;

return_statement:
    RETURN expression SEMICOLON { validateReturnType($2->dataType, yylineno); markFunctionReturnType(yylineno); }
    | RETURN SEMICOLON { validateReturnType("void", yylineno); markFunctionReturnType(yylineno); }
    ;

/*--------------------------------------------------------------------------*/
/* Function Declaration */
/*--------------------------------------------------------------------------*/

function_declare:
    FUNCTION function_type VARIABLE { insertFunc($3, "func", $2, yylineno); quadFunctionLabel($3);}'(' params ')' { quadAddFunctionParams($3); } block_structure   { checkLastFunctionReturnType(yylineno); insideFunctionIdx = -1;}
    ;

params:
    /* empty */ {}
    | param_list {  }
;

param_list:
    non_default_params
    | non_default_params ',' default_params
    | default_params
;

non_default_params:
    type VARIABLE { insertParam($2, "param", $1, false, NULL, yylineno); }
    | non_default_params ',' type VARIABLE { insertParam($4, "param", $3, false, NULL, yylineno); }
;

default_params:
    type VARIABLE ASSIGN const_value { insertParam($2, "param", $1, true, $4, yylineno); }
    | default_params ',' type VARIABLE ASSIGN const_value { insertParam($4, "param", $3, true, $6, yylineno); }
;

/*--------------------------------------------------------------------------*/
/* Variable Declaration */
/*--------------------------------------------------------------------------*/

var_declare:
    type VARIABLE   /*int x;*/           { insertVarConst($2, "var", $1, false, yylineno);  }
    | CONSTANT type VARIABLE ASSIGN expression { 
                                                    validateAssignmentType($2, $5) ? insertVarConst($3, "const", $2, true,yylineno) : yyerror("Type mismatch in assignment");
                                                    quadPopVar($3);
                                                }
    | type VARIABLE ASSIGN expression { 
                                        validateAssignmentType($1, $4)? insertVarConst($2, "var", $1, true, yylineno) : yyerror("Type mismatch in assignment");
                                        quadPopVar($2);
                                      }
    ;

assign_expression:
    VARIABLE ASSIGN expression {validateAssignmentType(getSymbolDataType($1), $3); validateNotConst($1); setVarUsed($1); quadPopVar($1);}
    ;

expression:
    const_value { quadPushConst($1);}
    | VARIABLE { checkInitialized($1, yylineno);$$ = createVarNode(getSymbolDataType($1), "var", $1); setVarUsed($1); quadPushVar($1); }
    | operation_expressions 
;    

operation_expressions:
    NOT expression              { $$ = checkComparisonExpressionTypes($2,$2); quadOperation("not"); }
    | expression ADD expression         { $$ = checkArithmitcExpressionTypes($1, $3); quadOperation("add"); }
    | expression SUB expression         { $$ = checkArithmitcExpressionTypes($1, $3); quadOperation("sub"); }
    | expression MUL expression         { $$ = checkArithmitcExpressionTypes($1, $3); quadOperation("mul"); }
    | expression DIV expression         { $$ = checkArithmitcExpressionTypes($1, $3); quadOperation("div"); }
    | expression MOD expression         { $$ = checkArithmitcExpressionTypes($1, $3); quadOperation("mod"); }

    | expression LT expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("lt"); }
    | expression GT expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("gt"); }
    | expression GE expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("ge"); }
    | expression LE expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("le"); }
    | expression EQ expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("eq"); }
    | expression NE expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("ne"); }

    | expression AND expression         { $$= checkComparisonExpressionTypes($1, $3); quadOperation("and"); }
    | expression OR expression          { $$= checkComparisonExpressionTypes($1, $3); quadOperation("or"); }
    | '(' expression ')'          {$$ = $2; }
    | VARIABLE '(' argument_list ')' { 
                                        validateFunctionCall($1,$3->types,$3->count);
                                        $$ = createNode(getSymbolDataType($1), "func");
                                        quadFunctionCall($1, $3->count);
                                    }
    | unary_operations
    ;

unary_operations:
    INC VARIABLE { Node* node = createNode(getSymbolDataType($2), "var"); $$ = checkUnaryOperationTypes(node); quadPrefix($2, "add"); checkInitialized($2, yylineno); validateNotConst($2);}
    | DEC VARIABLE { Node* node = createNode(getSymbolDataType($2), "var"); $$ = checkUnaryOperationTypes(node); quadPrefix($2, "sub"); checkInitialized($2, yylineno); validateNotConst($2); }
    | VARIABLE INC { Node* node = createNode(getSymbolDataType($1), "var"); $$ = checkUnaryOperationTypes(node); quadPostfix($1, "add"); checkInitialized($1, yylineno); validateNotConst($1); }
    | VARIABLE DEC { Node* node = createNode(getSymbolDataType($1), "var"); $$ = checkUnaryOperationTypes(node); quadPostfix($1, "sub"); checkInitialized($1, yylineno); validateNotConst($1);}
    ;


argument_list:
    expression {
        $$ = createArgList();
        addArgType($$, $1->dataType);
    }
    | argument_list ',' expression {
        addArgType($1, $3->dataType);
        $$ = $1;    
    }
    | { $$ = createArgList();}
    ;

/*--------------------------------------------------------------------------*/

type:
    INT                     { }
    | FLOAT                 {}
    | STRING                { }
    | CHAR                  { }
    | BOOL                  { }
    ;

function_type:
    type                { }
    | VOID                  {}
    ;

const_value:
    INT_VALUE                 { $$ = createIntNode(yylval.iValue); }
    | FLOAT_VALUE             { $$ = createFloatNode(yylval.fValue); }
    | BOOL_VALUE              { $$ = createBoolNode(yylval.bValue); }
    | CHAR_VALUE              { $$ = createCharNode(yylval.cValue); }
    | STRING_VALUE            { $$ = createStringNode(yylval.sValue);  } 
    ;

%%
void yyerror(char *s) {
    char *token = yytext;
    long file_pos = ftell(yyin);
    
    rewind(yyin);
    
    char line[1024];
    int current_line = 1;
    while (current_line <= yylineno && fgets(line, sizeof(line), yyin)) {
        if (current_line == yylineno) {
            line[strcspn(line, "\n")] = 0;
            fprintf(syntaxErrorsFileHandler.filePointer, "Error at line %d: %s near '%s'\n", yylineno, s, token);
            fprintf(syntaxErrorsFileHandler.filePointer, "Line: %s\n", line);
            fprintf(stderr, "Error at line %d: %s near '%s'\n", yylineno, s, token);
            fprintf(stderr, "Line: %s\n", line);
            break;
        }
        current_line++;
    }
    
    fseek(yyin, file_pos, SEEK_SET);
    if(strcmp(s, "syntax error") == 0) {

        exit(1); 
    }
    isError = 1;
}
void yywarn(char *s, int line) {
    
    int warning_line = (line > 0) ? line : yylineno;    
    fprintf(warningFileHandler.filePointer, "Warning at line %d: %s near token '%s'\n", warning_line, s, yytext ? yytext : "<none>");
    printf("Warning at line %d: %s near token '%s'\n", warning_line, s, yytext ? yytext : "<none>");
    
    if (yyin && !feof(yyin)) {
        long file_pos = ftell(yyin);
        rewind(yyin);
        
        char line[256];
        int current_line = 1;
        while (current_line <= warning_line && fgets(line, sizeof(line), yyin)) {
            if (current_line == warning_line) {
                line[strcspn(line, "\n")] = 0;

                fprintf(warningFileHandler.filePointer, "Line: %s\n", line);
                printf("Line: %s\n", line);

                break;
            }
            current_line++;
        }
        
        fseek(yyin, file_pos, SEEK_SET);
    }
}

int main(int argc, char *argv[]) {
    setFiles();
    

    initSymbolTable();
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            perror("Could not open file");
            return 1;
        }
        yyin = file;
    }
    int result = yyparse();
    if (argc > 1) {
        fclose(yyin);
    }

    cleanUpFiles();
    printSymbolTable(); 
    if (isError) {
        exit(1); 
    }
    return result;
}