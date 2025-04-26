%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    
    #include "y.tab.h"
    #include "parser.h" // Include the header
    #include "symbol_table.c"
    #include "quadruples.c"
    #include "assembly.c"
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
    | PRINT '(' expression ')' SEMICOLON { assemblyPrint(); }
    | if_statement {}
    | WHILE { assemblyLoopInit(); } '(' expression ')' { assemblyLoopBegin(); } block_structure { assemblyLoopExit(); }
    | DO { assemblyLoopInit(); } block_structure WHILE '(' expression ')' { assemblyLoopBegin(); } SEMICOLON { assemblyLoopExit(); }
    | FOR '(' for_loop_init SEMICOLON { assemblyLoopInit(); } expression SEMICOLON { assemblyLoopBegin(); } for_loop_expression ')' block_structure { assemblyLoopExit(); }
    | SEMICOLON   {  }  /* empty statment */
    | return_statement { assemblyJumpCall("_call_"); }
    | BREAK SEMICOLON { assemblyIsInLoop() ? assemblyJumpFalseLabel(loopLabels[loopIndex]) : yyerror("break statement not in loop"); }
    | CONTINUE SEMICOLON { assemblyIsInLoop() ? assemblyJump(loopLabels[loopIndex - 1]) : yyerror("continue statement not in loop"); }
    | assign_expression SEMICOLON {  }
    | SWITCH '(' expression ')' '{' case_list '}' { printf("switch\n"); }
    | block_structure {}
    ;  

if_statement:
    IF '(' expression ')'
        { 
            ifLabels[++ifIndex] = labelCounter++; 
            assemblyJumpFalse(ifLabels[ifIndex]); 
            quadJumpIfFalse($3, ifLabels[ifIndex]);
        }
    block_structure 
        { 
            assemblyJump(ifLabels[ifIndex]); 
            assemblyFalseLabel(ifLabels[ifIndex]); 
            quadJump(ifLabels[ifIndex]);
            quadFalseLabel(ifLabels[ifIndex]);
        } 
    else_block 
        { 
            assemblyLabel(ifLabels[ifIndex]); 
            quadLabel(ifLabels[ifIndex--]);
        }

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
    FUNCTION function_type VARIABLE { insertFunc($3, "func", $2, yylineno); assemblyFunctionLabel($3);}'(' params ')' { assemblyAddFunctionParams($3); } block_structure   { checkLastFunctionReturnType(yylineno); insideFunctionIdx = -1;}
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
                                                    assemblyPopVar($3);
                                                    quadAssign($3, $5);
                                                }
    | type VARIABLE ASSIGN expression { 
                                        validateAssignmentType($1, $4)? insertVarConst($2, "var", $1, true, yylineno) : yyerror("Type mismatch in assignment");
                                        assemblyPopVar($2);
                                        quadAssign($2, $4);
                                      }
    ;

assign_expression:
    VARIABLE ASSIGN expression { 
            validateAssignmentType(getSymbolDataType($1), $3);
            validateNotConst($1);
            setVarUsed($1);
            assemblyPopVar($1);
            quadAssign($1, $3);
        }
    ;

expression:
    const_value { assemblyPushConst($1);}
    | VARIABLE { checkInitialized($1, yylineno);$$ = createVarNode(getSymbolDataType($1), "var", $1); setVarUsed($1); assemblyPushVar($1); }
    | operation_expressions 
;    

operation_expressions:
    NOT expression              { $$ = checkComparisonExpressionTypes($2,$2); assemblyOperation("not"); }
    | expression ADD expression         { $$ = checkArithmitcExpressionTypes($1, $3); assemblyOperation("add"); $$ = quadOperation("add", $1, $3); }
    | expression SUB expression         { $$ = checkArithmitcExpressionTypes($1, $3); assemblyOperation("sub"); $$ = quadOperation("sub", $1, $3); }
    | expression MUL expression         { $$ = checkArithmitcExpressionTypes($1, $3); assemblyOperation("mul"); $$ = quadOperation("mul", $1, $3); }
    | expression DIV expression         { $$ = checkArithmitcExpressionTypes($1, $3); assemblyOperation("div"); $$ = quadOperation("div", $1, $3); }
    | expression MOD expression         { $$ = checkArithmitcExpressionTypes($1, $3); assemblyOperation("mod"); $$ = quadOperation("mod", $1, $3); }

    | expression LT expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("lt"); $$ = quadOperation("lt", $1, $3); }
    | expression GT expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("gt"); $$ = quadOperation("gt", $1, $3); }
    | expression GE expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("ge"); $$ = quadOperation("ge", $1, $3); }
    | expression LE expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("le"); $$ = quadOperation("le", $1, $3); }
    | expression EQ expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("eq"); $$ = quadOperation("eq", $1, $3); }
    | expression NE expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("ne"); $$ = quadOperation("ne", $1, $3); }

    | expression AND expression         { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("and"); $$ = quadOperation("and", $1, $3); }
    | expression OR expression          { $$= checkComparisonExpressionTypes($1, $3); assemblyOperation("or"); $$ = quadOperation("or", $1, $3); }
    | '(' expression ')'          {$$ = $2; }
    | VARIABLE '(' argument_list ')' { 
                                        validateFunctionCall($1,$3->types,$3->count);
                                        $$ = createNode(getSymbolDataType($1), "func");
                                        assemblyFunctionCall($1, $3->count);
                                    }
    | unary_operations
    ;

unary_operations:
    INC VARIABLE { 
        Node* node = createNode(getSymbolDataType($2), "var"); 
        $$ = checkUnaryOperationTypes(node); 
        checkInitialized($2, yylineno); 
        validateNotConst($2);
        assemblyPrefix($2, "add"); 
        $$ = quadPrefixIncrement($2); 
    }
    | DEC VARIABLE {
        Node* node = createNode(getSymbolDataType($2), "var"); 
        $$ = checkUnaryOperationTypes(node); 
        checkInitialized($2, yylineno); 
        validateNotConst($2);
        assemblyPrefix($2, "sub"); 
        $$ = quadPrefixDecrement($2); 
    }
    | VARIABLE INC { 
        Node* node = createNode(getSymbolDataType($1), "var"); 
        $$ = checkUnaryOperationTypes(node); 
        checkInitialized($1, yylineno); 
        validateNotConst($1);
        assemblyPostfix($1, "add"); 
        $$ = quadPostfixIncrement($1); 
    }
    | VARIABLE DEC { 
        Node* node = createNode(getSymbolDataType($1), "var"); 
        $$ = checkUnaryOperationTypes(node); 
        checkInitialized($1, yylineno); 
        validateNotConst($1);
        assemblyPostfix($1, "sub"); 
        $$ = quadPostfixDecrement($1); 
    }
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