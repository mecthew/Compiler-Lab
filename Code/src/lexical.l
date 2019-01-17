%option yylineno

%{
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "syntax.tab.h"
#include "AST.h"
#include "common.h"
int str2i(const char* str);

TreeNode *ASTroot = NULL;
int yycolumn = 1;
bool isError = false;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}


int     (0|[1-9][0-9]*|0[0-7]+|0[Xx][0-9A-Fa-f]+)
float   ([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([Ee][+-]?[0-9]+)?
id      [_a-zA-Z][_a-zA-Z0-9]{0,30}
ws      [ \t\r\n]+
relop   ">"|"<"|">="|"<="|"=="|"!="

%%

{int}           { yylval = createNode(AST_INT, yylloc.first_line);       yylval->val.i = str2i(yytext); strcpy(yylval->val.c,yytext); return INT; }
{float}         { yylval = createNode(AST_FLOAT, yylloc.first_line);     yylval->val.d = atof(yytext); strcpy(yylval->val.c,yytext); return FLOAT; }
";"             { yylval = createNode(AST_SEMI, yylloc.first_line);      return SEMI; }
","             { yylval = createNode(AST_COMMA, yylloc.first_line);     return COMMA; }
"="             { yylval = createNode(AST_ASSIGNOP, yylloc.first_line);  return ASSIGNOP; }
{relop}         { yylval = createNode(AST_RELOP, yylloc.first_line);     strcpy(yylval->val.c, yytext); return RELOP; }
"+"             { yylval = createNode(AST_PLUS, yylloc.first_line);      return PLUS; }
"-"             { yylval = createNode(AST_MINUS, yylloc.first_line);     return MINUS; }
"*"             { yylval = createNode(AST_STAR, yylloc.first_line);      return STAR; }
"/"             { yylval = createNode(AST_DIV, yylloc.first_line);       return DIV; }
"&&"            { yylval = createNode(AST_AND, yylloc.first_line);       return AND; }
"||"            { yylval = createNode(AST_OR, yylloc.first_line);        return OR; }
"."             { yylval = createNode(AST_DOT, yylloc.first_line);       return DOT; }
"!"             { yylval = createNode(AST_NOT, yylloc.first_line);       return NOT; }
"("             { yylval = createNode(AST_LP, yylloc.first_line);        return LP; }
")"             { yylval = createNode(AST_RP, yylloc.first_line);        return RP; }
"["             { yylval = createNode(AST_LB, yylloc.first_line);        return LB; }
"]"             { yylval = createNode(AST_RB, yylloc.first_line);        return RB; }
"{"             { yylval = createNode(AST_LC, yylloc.first_line);        return LC; }
"}"             { yylval = createNode(AST_RC, yylloc.first_line);        return RC; }
"int"|"float"   { yylval = createNode(AST_TYPE, yylloc.first_line);      strcpy(yylval->val.c, yytext); return TYPE; }
"struct"        { yylval = createNode(AST_STRUCT, yylloc.first_line);    return STRUCT; }
"return"        { yylval = createNode(AST_RETURN, yylloc.first_line);    return RETURN; }
"if"            { yylval = createNode(AST_IF, yylloc.first_line);        return IF; }
"else"          { yylval = createNode(AST_ELSE, yylloc.first_line);      return ELSE; }
"while"         { yylval = createNode(AST_WHILE, yylloc.first_line);     return WHILE; }
{id}            { yylval = createNode(AST_ID, yylloc.first_line);        strcpy(yylval->val.c, yytext); return ID; }
\n         		{ yycolumn = 1; }
{ws}+           { }
.               { isError = true; printf("Error type A at Line %d: mysterious character \'%s\'.\n", yylineno, yytext); }

%%
int str2i(const char* str) {
  int res = 0;
  if (str[0] == '0') {
    if (str[1] == 'x' || str[1] == 'X') {
      sscanf(str + 2, "%x", &res);
    }
    else if (str[1] != '\0') {
      sscanf(str + 1, "%o", &res);
    }
  }
  else {
    sscanf(str, "%d", &res);
  }
  return res;
}



