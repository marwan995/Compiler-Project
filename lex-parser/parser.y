%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "node.h"
extern FILE *yyin;
/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(int i);
nodeType *con(int value);
void freeNode(nodeType *p);
int yylex(void);

void yyerror(char *s);
%}

%union {
    int iValue;          /* integer value */
    float fValue;        /* float value */
    int bValue;          /* boolean value */
    char cValue;         /* char value */
    char *sValue;        /* string value */
    char *id;        /* symbol name */
    nodeType *nPtr;      /* node pointer */
};

%token <iValue> INT_VALUE
%token <fValue> FLOAT_VALUE
%token <bValue> BOOL_VALUE
%token <cValue> CHAR_VALUE
%token <sValue> STRING_VALUE

%token <id> VARIABLE

%token INT FLOAT STRING CHAR BOOL CONSTANT VOID

%token FOR WHILE DO BREAK CONTINUE IF ELSE

%token GT LT GE LE EQ NE AND OR NOT ASSIGN

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

%type <nPtr> statement_list statement type var_declare params expression const_value function_declare declare_list declare operation_expressions argument_list

%%
/*--------------------------------------------------------------------------*/
/* The grammar rules for the parser. */
/*--------------------------------------------------------------------------*/

program:
        declare_list                {  }
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
    | IF '(' expression ')' '{' statement_list '}' %prec IFX {  }
    | IF '(' expression ')' '{' statement_list '}' ELSE else_block {  }
    | IF '(' expression ')'  statement %prec IFX {  }
    | IF '(' expression ')'  statement ELSE statement {  }
    | WHILE '(' expression ')' statement {  }
    | WHILE '(' expression ')' '{' statement_list '}' {  }
    | DO '{' statement_list '}' WHILE '(' expression ')' SEMICOLON {  }
    | FOR '(' for_loop_init SEMICOLON expression SEMICOLON assign_expression ')' '{' statement_list '}' { printf( "for loop\n"); }
    | SEMICOLON   {  }  /* empty statment */
    | return_statement {}
    | BREAK SEMICOLON {  }
    | CONTINUE SEMICOLON {  }
    | assign_expression SEMICOLON {  }
    | SWITCH '(' expression ')' '{' case_list '}' { printf("switch\n"); }
    | block_structure {  } /* block statement */
    ;  

else_block:
    block_structure
    | statement
    ;
    
block_structure: 
    '{' statement_list '}' {}
    ;

case_list: 
     case_list CASE const_value ':' statement {  }
    | case_list DEFAULT ':' statement {  }
    | case_list CASE const_value ':' '{' statement_list '}' {  }
    | case_list DEFAULT ':' '{' statement_list '}' {  }
    |                           {}
    ;
    
for_loop_init:
   assign_expression {  }
    | var_declare {  }
    ;
return_statement:
    RETURN expression SEMICOLON {  }
    | RETURN SEMICOLON {  }
    ;

/*--------------------------------------------------------------------------*/
/* Function Declaration */
/*--------------------------------------------------------------------------*/

function_declare:
    FUNCTION VOID VARIABLE '(' params ')' '{' statement_list '}' {}
    | FUNCTION type VARIABLE '(' params ')' '{' statement_list '}'   {}
    ;

params:
    /* empty */                     {}
    | param_list                    {}
    ;

param_list:
    type VARIABLE                   {}
    | type VARIABLE ',' param_list {}
    ;
/*--------------------------------------------------------------------------*/
/* Variable Declaration */
/*--------------------------------------------------------------------------*/

var_declare:
    type VARIABLE   /*int x;*/           {}
    | CONSTANT type assign_expression /*int const x = 1*/ {}
    | type assign_expression /*int x = 1;*/ {}
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
    INT_VALUE                 { printf("Integer value: %d\n", $1); }
    | FLOAT_VALUE                 { printf("Float value: %f\n", $1); }
    | BOOL_VALUE            { printf("Boolean value: %i\n", $1); }
    | CHAR_VALUE            { printf("Character value: %c\n", $1); }
    | STRING_VALUE          { printf("String value: %s\n", $1); }
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