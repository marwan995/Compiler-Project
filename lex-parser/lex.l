%{
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "y.tab.h"
void yyerror(char *);
%}

%option yylineno

%%
#[\s\t]*.*[\s\t]* { /* ignore comments */ }
    /*Types of data*/
"int"       { yylval.sValue = strdup("int"); return INT; }
"float"     { yylval.sValue = strdup("float"); return FLOAT; }
"string"    { yylval.sValue = strdup("string"); return STRING; }
"char"      { yylval.sValue = strdup("char"); return CHAR; }
"bool"      { yylval.sValue = strdup("bool"); return BOOL; }
"const"     { yylval.sValue = strdup("const"); return CONSTANT; }
"void"      { yylval.sValue = strdup("void"); return VOID; }

    /*loops*/
"for"           return FOR;
"while"         return WHILE;
"do"            return DO;
"break"         return BREAK;
"continue"      return CONTINUE;

    /*relational*/
">"             return GT;
"<"             return LT;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
    /*logical*/
"&&"            return AND;
"||"            return OR;
"!"             return NOT;
    /*unary operations*/
"++"            return INC;
"--"            return DEC;
    /*mathematical*/
"-"             return SUB;
"+"             return ADD;
"="             return ASSIGN;
"*"             return MUL;
"/"             return DIV;
"%"             return MOD;
";"             return SEMICOLON;


    /*condtions*/
"if"            return IF;
"else"          return ELSE;
"switch"        return SWITCH;
"case"          return CASE;
"default"       return DEFAULT;
    /*Functions*/    
"func"          return FUNCTION;
"return"        return RETURN;
    /*other keywords and patterns*/
"print"         return PRINT;
"true"          { yylval.bValue = 1; return BOOL_VALUE; }
"false"         { yylval.bValue = 0;return BOOL_VALUE; }   

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sValue = strdup(yytext) ; return VARIABLE; }

[-]?[0-9][0-9]*             { yylval.iValue = atoi(yytext);  return INT_VALUE; }
[-]?[0-9]+\.[0-9]*          { yylval.fValue = atof(yytext); return FLOAT_VALUE; }
[-]?[0-9]+\.[0-9]*[eE][-+]?[0-9]+    { yylval.fValue = atof(yytext); return FLOAT_VALUE; }

\'.\'         { yylval.cValue = yytext[1]; return CHAR_VALUE; }
\"[^"\n]*\"   { yytext[yyleng - 1] = '\0'; yylval.sValue = strdup(yytext + 1); return STRING_VALUE; }
[(){},:]      return *yytext;

[ \t\n]+        { /* ignore whitespace */}

.              {printf("Found char: %s", *yytext); return *yytext;}

%%
int yywrap(void) {
    return 1;
}
