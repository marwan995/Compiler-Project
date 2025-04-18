// parser.h
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

void yyerror(char *s); // Declare yyerror
extern int yylineno;   // Declare yylineno for line number access
extern char *yytext;   // Declare yytext for token context (if needed)

#endif