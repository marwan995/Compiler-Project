%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    
    #include "y.tab.h"
    #include "symbol_table.c"

    extern FILE *yyin;
    extern int yylineno;

    int yylex(void);

    void yyerror(char *s);
%}

%union {
    int iValue;          /* integer value */
    float fValue;        /* float value */
    int bValue;          /* boolean value */
    char cValue;         /* char value */
    char *sValue;        /* string value */

    struct nodeType *nPtr;
};

%token <iValue> INT_VALUE
%token <fValue> FLOAT_VALUE
%token <bValue> BOOL_VALUE
%token <cValue> CHAR_VALUE
%token <sValue> STRING_VALUE

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

%type <nPtr> statement_list statement var_declare params expression const_value function_declare declare_list declare operation_expressions argument_list unary_operations
%type <nPtr> non_default_params default_params assign_expression

%%
/*--------------------------------------------------------------------------*/
/* The grammar rules for the parser. */
/*--------------------------------------------------------------------------*/

program:
        declare_list                { initSymbolTable(yylineno); }
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
    '{' { enterScope(yylineno); } statement_list '}' { exitScope(yylineno); }
    ;
    
/*--------------------------------------------------------------------------*/
/* For loop */
/*--------------------------------------------------------------------------*/

for_loop_init:
   assign_expression {  }
    | var_declare {  }
    ;

for_loop_expression:
    assign_expression {  }
    | unary_operations {  }
    ;

return_statement:
    RETURN expression SEMICOLON {  }
    | RETURN SEMICOLON {  }
    ;

/*--------------------------------------------------------------------------*/
/* Function Declaration */
/*--------------------------------------------------------------------------*/

function_declare:
    FUNCTION VOID VARIABLE { insertSymbol($3, "func", $2, false, false, yylineno); } '(' params ')' block_structure {  }
    | FUNCTION type VARIABLE { insertSymbol($3, "func", $2, false, false, yylineno); }'(' params ')' block_structure   {  }
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
    type VARIABLE { insertSymbol($2, "param", $1, false, true, yylineno); }
    | non_default_params ',' type VARIABLE { insertSymbol($4, "param", $3, false, true, yylineno); }
;

default_params:
    type VARIABLE ASSIGN const_value { insertSymbol($2, "param", $1, true, true, yylineno); }
    | default_params ',' type VARIABLE ASSIGN const_value { insertSymbol($4, "param", $3, true, true, yylineno); }
;

/*--------------------------------------------------------------------------*/
/* Variable Declaration */
/*--------------------------------------------------------------------------*/

var_declare:
    type VARIABLE   /*int x;*/           { insertSymbol($2, "var", $1, false, false, yylineno); }
    | CONSTANT type VARIABLE ASSIGN expression /*int const x = 1*/ { insertSymbol($3, "const", $2, true, false,yylineno); }
    | type VARIABLE ASSIGN expression /*int x = 1;*/ { insertSymbol($2, "var", $1, true, false, yylineno); }
    ;

assign_expression:
    VARIABLE ASSIGN expression {  }
    ;

expression:
    const_value {}
    | VARIABLE {}
    | operation_expressions
;    

operation_expressions:
    NOT expression              {  }
    | expression ADD expression         { }
    | expression SUB expression         { }
    | expression MUL expression         { }
    | expression DIV expression         { }
    | expression MOD expression         { }
    | expression LT expression          { }
    | expression GT expression          {  }
    | expression GE expression          {  }
    | expression LE expression          { }
    | expression EQ expression          {  }
    | expression NE expression          {}
    | expression AND expression         { }
    | expression OR expression          { }
    | '(' expression ')'          {}
    | VARIABLE '(' argument_list ')' {  }
    | unary_operations
    ;

unary_operations:
    INC VARIABLE {  }
    | DEC VARIABLE {  }
    | VARIABLE INC {  }
    | VARIABLE DEC {  }
    ;


argument_list:
    /* empty */                   
     expression                  { }
    | argument_list ',' expression     {  }
    ;

/*--------------------------------------------------------------------------*/

type:
    INT                     { }
    | FLOAT            {  }
    | STRING                {}
    | CHAR                  {  }
    | BOOL                  {  }
    ;

const_value:
    INT_VALUE                 { }
    | FLOAT_VALUE                 { }
    | BOOL_VALUE            { }
    | CHAR_VALUE            {  }
    | STRING_VALUE          { }
    ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(1);
}

int main(int argc, char *argv[]) {
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
    return result;
}