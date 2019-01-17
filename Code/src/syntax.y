%{
#include <stdio.h>
#include "AST.h"
#include "lex.yy.c"
#include "common.h"

extern bool isError;
void yyerror(char *msg);
%}

/*%locations*/
%define api.value.type {struct TreeNode*}

/* declared tokens */
%token INT FLOAT
%token LC RC SEMI COMMA
%token TYPE STRUCT RETURN WHILE IF ID

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE


%%

/* High-level Definitions */
Program : ExtDefList                { $$ = ASTroot = createNode(AST_Program, @$.first_line); addNode($$, 1, $1); }/*$$->child = ASTroot->child = $1; $1->parent = ASTroot; }*/
    ;

ExtDefList : ExtDef ExtDefList      { $$ = createNode(AST_ExtDefList, @$.first_line); addNode($$, 2, $1, $2); }
    | /* empty */                   { $$ = createNode(AST_ExtDefList, @$.first_line); }
    ;

ExtDef : Specifier ExtDecList SEMI  { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Specifier SEMI                { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 2, $1, $2); }
    | Specifier FunDec CompSt       { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Specifier FunDec SEMI         { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | error SEMI                    { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 2, createNode(AST_Error, @$.first_line), $2); yyerrok; }
    | Specifier error SEMI          { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 3, $1, createNode(AST_Error, @$.first_line), $3); yyerrok; }
    | Specifier error               { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 2, $1, createNode(AST_Error, @$.first_line)); yyerrok; }
    | Specifier ExtDecList error    { $$ = createNode(AST_ExtDef, @$.first_line); addNode($$, 3, $1, $2, createNode(AST_Error, @$.first_line)); yyerrok; }
    ;

ExtDecList : VarDec                 { $$ = createNode(AST_ExtDecList, @$.first_line); addNode($$, 1, $1); }
    | VarDec COMMA ExtDecList       { $$ = createNode(AST_ExtDecList, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | error COMMA ExtDecList        { $$ = createNode(AST_ExtDecList, @$.first_line); addNode($$, 3, createNode(AST_Error, @$.first_line), $2, $3); yyerrok; }
    | VarDec COMMA error            { $$ = createNode(AST_ExtDecList, @$.first_line); addNode($$, 3, $1, $2, createNode(AST_Error, @$.first_line)); yyerrok; }
    ;

/* Specifiers */
Specifier : TYPE                    { $$ = createNode(AST_Specifier, @$.first_line); addNode($$, 1, $1); }
    | StructSpecifier               { $$ = createNode(AST_Specifier, @$.first_line); addNode($$, 1, $1); }
    ;

StructSpecifier : STRUCT OptTag LC DefList RC   { $$ = createNode(AST_StructSpecifier, @$.first_line); addNode($$, 5, $1, $2, $3, $4, $5); }
    | STRUCT Tag                                { $$ = createNode(AST_StructSpecifier, @$.first_line); addNode($$, 2, $1, $2); }
    | STRUCT OptTag LC error RC                 { $$ = createNode(AST_StructSpecifier, @$.first_line); addNode($$, 5, $1, $2, $3, createNode(AST_Error, @$.first_line), $5); yyerrok; }
    ;

OptTag : ID                         { $$ = createNode(AST_OptTag, @$.first_line); addNode($$, 1, $1); }
    | /* empty */                   { $$ = createNode(AST_OptTag, @$.first_line); }
    ;

Tag : ID                            { $$ = createNode(AST_Tag, @$.first_line); addNode($$, 1, $1); }
    ;

/* Declarators */
VarDec : ID                         { $$ = createNode(AST_VarDec, @$.first_line); addNode($$, 1, $1); }
    | VarDec LB INT RB              { $$ = createNode(AST_VarDec, @$.first_line); addNode($$, 4, $1, $2, $3, $4); }
    ;

FunDec : ID LP VarList RP           { $$ = createNode(AST_FunDec, @$.first_line); addNode($$, 4, $1, $2, $3, $4); }
    | ID LP RP                      { $$ = createNode(AST_FunDec, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | ID LP error RP                { $$ = createNode(AST_FunDec, @$.first_line); addNode($$, 4, $1, $2, createNode(AST_Error, @$.first_line), $4); yyerrok; }
    ;

VarList : ParamDec COMMA VarList    { $$ = createNode(AST_VarList, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | ParamDec                      { $$ = createNode(AST_VarList, @$.first_line); addNode($$, 1, $1); }
    ;

ParamDec : Specifier VarDec         { $$ = createNode(AST_ParamDec, @$.first_line); addNode($$, 2, $1, $2); }
    ;

/* Statements */
CompSt : LC DefList StmtList RC     { $$ = createNode(AST_CompSt, @$.first_line); addNode($$, 4, $1, $2, $3, $4); }
    ;

StmtList : Stmt StmtList            { $$ = createNode(AST_StmtList, @$.first_line); addNode($$, 2, $1, $2); }
    | /* empty */                   { $$ = createNode(AST_StmtList, @$.first_line); }
    ;

Stmt : Exp SEMI                                             { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 2, $1, $2); }
    | CompSt                                                { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 1, $1); }
    | RETURN Exp SEMI                                       { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | IF LP Exp RP Stmt             %prec LOWER_THEN_ELSE   { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 5, $1, $2, $3, $4, $5); }
    | IF LP Exp RP Stmt ELSE Stmt                           { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 7, $1, $2, $3, $4, $5, $6, $7); }
    | WHILE LP Exp RP Stmt                                  { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 5, $1, $2, $3, $4, $5); }
    | error SEMI                                            { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 2, createNode(AST_Error, @$.first_line), $2); yyerrok; }
    | Exp error                                             { $$ = createNode(AST_Stmt, @$.first_line); addNode($$, 2, $1, createNode(AST_Error, @$.first_line)); yyerrok; }

/* Local Definitions */
DefList : Def DefList               { $$ = createNode(AST_DefList, @$.first_line); addNode($$, 2, $1, $2); }
    | /* empty */                   { $$ = createNode(AST_DefList, @$.first_line); }
    ;

Def : Specifier DecList SEMI        { $$ = createNode(AST_Def, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Specifier error SEMI          { $$ = createNode(AST_Def, @$.first_line); addNode($$, 3, $1, createNode(AST_Error, @$.first_line), $3); yyerrok; }
    | Specifier DecList error       { $$ = createNode(AST_Def, @$.first_line); addNode($$, 3, $1, $2, createNode(AST_Error, @$.first_line)); yyerrok; }
    ;

DecList : Dec                       { $$ = createNode(AST_DecList, @$.first_line); addNode($$, 1, $1); }
    | Dec COMMA DecList             { $$ = createNode(AST_DecList, @$.first_line); addNode($$, 3, $1, $2, $3); }
    ;

Dec : VarDec                        { $$ = createNode(AST_Dec, @$.first_line); addNode($$, 1, $1); }
    | VarDec ASSIGNOP Exp           { $$ = createNode(AST_Dec, @$.first_line); addNode($$, 3, $1, $2, $3); }
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp              { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp AND Exp                   { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp OR Exp                    { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp RELOP Exp                 { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp PLUS Exp                  { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp MINUS Exp                 { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp STAR Exp                  { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp DIV Exp                   { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | LP Exp RP                     { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | MINUS Exp                     { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 2, $1, $2); }
    | NOT Exp                       { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 2, $1, $2); }
    | ID LP Args RP                 { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 4, $1, $2, $3, $4); }
    | ID LP RP                      { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp LB Exp RB                 { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 4, $1, $2, $3, $4); }
    | Exp DOT ID                    { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | ID                            { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 1, $1); }
    | INT                           { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 1, $1); }
    | FLOAT                         { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 1, $1); }
    | Exp LB error RB               { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 4, $1, $2, createNode(AST_Error, @$.first_line), $4); yyerrok; }
    | ID LP error RP                { $$ = createNode(AST_Exp, @$.first_line); addNode($$, 4, $1, $2, createNode(AST_Error, @$.first_line), $4); yyerrok; }
    ;

Args : Exp COMMA Args               { $$ = createNode(AST_Args, @$.first_line); addNode($$, 3, $1, $2, $3); }
    | Exp                           { $$ = createNode(AST_Args, @$.first_line); addNode($$, 1, $1); }
    ;

%%
void yyerror(char *msg) {
  isError = true;
  printf("Error type B at Line %d: %s.\n", yylineno, msg);
}
