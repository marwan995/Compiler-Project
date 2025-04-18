%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    
    #include "y.tab.h"
    #include "parser.h" // Include the header
    #include "symbol_table.c"
    #include "quadruples.c"
    #include "checkers.c"
    

    extern FILE *yyin;
    extern int yylineno;
    extern char *yytext;
    int yylex(void);

    void yyerror(char *s);
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

%type<sValue> type VARIABLE VOID

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
    | PRINT '(' expression ')' SEMICOLON {  }
    | if_statement {}
    | WHILE '(' expression ')' block_structure {  }
    | DO block_structure WHILE '(' expression ')' SEMICOLON {  }
    | FOR '(' for_loop_init SEMICOLON expression SEMICOLON for_loop_expression ')' block_structure { printf( "for loop\n"); }
    | SEMICOLON   {  }  /* empty statment */
    | return_statement {}
    | BREAK SEMICOLON {  }
    | CONTINUE SEMICOLON {  }
    | assign_expression SEMICOLON {  }
    | SWITCH '(' expression ')' '{' case_list '}' { printf("switch\n"); }
    | block_structure {}
    ;  

if_statement:
    IF '(' expression ')' block_structure %prec IFX {  }
    | IF '(' expression ')' block_structure ELSE else_block {  };

else_block:
    block_structure {}
    | if_statement {  }
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
    '}' { exitScope(yylineno); }
    ;
    
/*--------------------------------------------------------------------------*/
/* For loop */
/*--------------------------------------------------------------------------*/

for_loop_init:
    VARIABLE ASSIGN expression { validateAssignmentType(getSymbolDataType($1, yylineno), $3); validateNotConst($1, yylineno); insertForLoopVar($1, "var", NULL, yylineno);}
    | type VARIABLE ASSIGN expression /*int x = 1;*/ { validateAssignmentType($1, $4) ? insertForLoopVar($2, "var", $1, yylineno): yyerror("Type mismatch in assignment");  }
    ;

for_loop_expression:
    assign_expression {  }
    | unary_operations {  }
    ;

return_statement:
    RETURN expression SEMICOLON { validateReturnType($2->type, yylineno); markFunctionReturnType(yylineno); }
    | RETURN SEMICOLON { validateReturnType("void", yylineno); markFunctionReturnType(yylineno); }
    ;

/*--------------------------------------------------------------------------*/
/* Function Declaration */
/*--------------------------------------------------------------------------*/

function_declare:
    FUNCTION VOID VARIABLE { insertFunc($3, "func", $2, yylineno); } '(' params ')' block_structure { checkLastFunctionReturnType(yylineno); }
    | FUNCTION type VARIABLE { insertFunc($3, "func", $2, yylineno); }'(' params ')' block_structure   { checkLastFunctionReturnType(yylineno); }
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
    type VARIABLE { insertParam($2, "param", $1, false, yylineno); }
    | non_default_params ',' type VARIABLE { insertParam($4, "param", $3, false, yylineno); }
;

default_params:
    type VARIABLE ASSIGN const_value { insertParam($2, "param", $1, true, yylineno); }
    | default_params ',' type VARIABLE ASSIGN const_value { insertParam($4, "param", $3, true, yylineno); }
;

/*--------------------------------------------------------------------------*/
/* Variable Declaration */
/*--------------------------------------------------------------------------*/

var_declare:
    type VARIABLE   /*int x;*/           { insertVarConst($2, "var", $1, false, yylineno);}
    | CONSTANT type VARIABLE ASSIGN expression /*int const x = 1*/ { insertVarConst($3, "const", $2, true,yylineno); }
    | type VARIABLE ASSIGN expression /*int x = 1;*/ { validateAssignmentType($1, $4)? insertVarConst($2, "var", $1, true, yylineno): yyerror("Type mismatch in assignment"); }
    ;

assign_expression:
    VARIABLE ASSIGN expression {validateAssignmentType(getSymbolDataType($1, yylineno), $3); validateNotConst($1, yylineno); }
    ;

expression:
    const_value { }
    | VARIABLE { checkInitialized($1, yylineno);$$ = createNode(getSymbolDataType($1, yylineno)); }
    | operation_expressions
;    

operation_expressions:
    NOT expression              { $$ = checkComparisonExpressionTypes($2,$2); handleOperation("NOT"); }
    | expression ADD expression         { $$ = checkArithmitcExpressionTypes($1, $3); handleOperation("ADD"); }
    | expression SUB expression         { $$ = checkArithmitcExpressionTypes($1, $3); handleOperation("SUB"); }
    | expression MUL expression         { $$ = checkArithmitcExpressionTypes($1, $3); handleOperation("MUL"); }
    | expression DIV expression         { $$ = checkArithmitcExpressionTypes($1, $3); handleOperation("DIV"); }
    | expression MOD expression         { $$ = checkArithmitcExpressionTypes($1, $3); handleOperation("MOD"); }

    | expression LT expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("LT"); }
    | expression GT expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("GT"); }
    | expression GE expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("GE"); }
    | expression LE expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("LE"); }
    | expression EQ expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("EQ"); }
    | expression NE expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("NE"); }

    | expression AND expression         { $$= checkComparisonExpressionTypes($1, $3); handleOperation("AND"); }
    | expression OR expression          { $$= checkComparisonExpressionTypes($1, $3); handleOperation("OR"); }
    | '(' expression ')'          {$$ = $2; }
    | VARIABLE '(' argument_list ')' { validateFunctionCall($1,$3->types,$3->count,yylineno); $$ = createNode(getSymbolDataType($1, yylineno)); }
    | unary_operations
    ;

unary_operations:
    INC VARIABLE { Node* node = createNode(getSymbolDataType($2, yylineno)); $$ = checkUnaryOperationTypes(node); handleOperation("INC"); checkInitialized($2, yylineno); validateNotConst($2, yylineno);}
    | DEC VARIABLE { Node* node = createNode(getSymbolDataType($2, yylineno)); $$ = checkUnaryOperationTypes(node); handleOperation("DEC"); checkInitialized($2, yylineno); validateNotConst($2, yylineno); }
    | VARIABLE INC { Node* node = createNode(getSymbolDataType($1, yylineno)); $$ = checkUnaryOperationTypes(node); handleOperation("INC"); checkInitialized($1, yylineno); validateNotConst($1, yylineno); }
    | VARIABLE DEC { Node* node = createNode(getSymbolDataType($1, yylineno)); $$ = checkUnaryOperationTypes(node); handleOperation("DEC"); checkInitialized($1, yylineno); validateNotConst($1, yylineno);}
    ;


argument_list:
    expression {
        $$ = createArgList();
        addArgType($$, $1->type);
    }
    | argument_list ',' expression {
        addArgType($1, $3->type);
        $$ = $1;    
    }

/*--------------------------------------------------------------------------*/

type:
    INT                     { }
    | FLOAT                 {}
    | STRING                { }
    | CHAR                  { }
    | BOOL                  { }
    ;

const_value:
    INT_VALUE                 { $$ = createIntNode(yylval.iValue); }
    | FLOAT_VALUE             { $$ = createFloatNode(yylval.fValue); }
    | BOOL_VALUE              { $$ = createBoolNode(yylval.bValue); }
    | CHAR_VALUE              { $$ = createCharNode(yylval.cValue); }
    | STRING_VALUE            { $$ = createStringNode(yylval.sValue); } 
    ;

%%

void yyerror(char *s) {
    // Get the current token from the lexer
    char *token = yytext;
    // Store the current file position to restore it later
    long file_pos = ftell(yyin);
    
    // Rewind to the beginning of the file to read the line
    rewind(yyin);
    
    // Read lines until we reach the error line
    char line[1024]; // Buffer for the line (adjust size as needed)
    int current_line = 1;
    while (current_line <= yylineno && fgets(line, sizeof(line), yyin)) {
        if (current_line == yylineno) {
            // Remove trailing newline for clean output
            line[strcspn(line, "\n")] = 0;
            fprintf(stderr, "Error at line %d: %s near '%s'\n", yylineno, s, token);
            fprintf(stderr, "Line: %s\n", line);
            break;
        }
        current_line++;
    }
    
    // Restore the file position
    fseek(yyin, file_pos, SEEK_SET);
    
    exit(1);
}

int main(int argc, char *argv[]) {
     set_file_path("quads.txt");
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
    close_file();
    printSymbolTable(); 
    return result;
}