// parser.h
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

void yyerror(char *s); // Declare yyerror
void yywarn(char *s);  // New declaration for warnings
extern int yylineno;   // Declare yylineno for line number access
extern char *yytext;   // Declare yytext for token context (if needed)

#endif