#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "AST.h"
#include "common.h"

const char *const ASTNodeTypeName[] = {
    "INT",        "FLOAT",      "SEMI",
    "COMMA",      "ASSIGNOP",   "RELOP",
    "PLUS",       "MINUS",      "STAR",
    "DIV",        "AND",        "OR",
    "DOT",        "NOT",        "LP",
    "RP",         "LB",         "RB",
    "LC",         "RC",         "TYPE",
    "STRUCT",     "RETURN",     "IF",
    "ELSE",       "WHILE",      "ID",

    "Error",
    "Program",    "ExtDefList", "ExtDef",
    "ExtDecList", "Specifier",  "StructSpecifier",
    "OptTag",     "Tag",        "VarDec",
    "FunDec",     "VarList",    "ParamDec",
    "CompSt",     "StmtList",   "Stmt",
    "DefList",    "Def",        "DecList",
    "Dec",        "Exp",        "Args"
};

struct TreeNode *createNode(ASTNodeType type, int lineno) {
    TreeNode *p = (TreeNode *)malloc(sizeof(TreeNode));
    p->child = p->sibling = p->parent = NULL;
    p->type = type;
    p->lineno = lineno;
    return p;
}

int addNode(TreeNode *parent, int count, ...) {
    if (count < 1)
        return -1;

    va_list argp;
    va_start(argp, count);

    TreeNode *p, *q;
    parent->child = p = va_arg(argp, TreeNode *);
    p->parent = parent;
    for (int i = 1; i < count; i++) {
        q = va_arg(argp, TreeNode *);
        q->parent = parent;
        p->sibling = q;
        p = q;
    }

    va_end(argp);
    return 0;
}

void printfTree(TreeNode *parent, int layer) {
    if (!parent || parent->type > AST_Program && parent->child == NULL)
        return;

    for (int i = 0; i < layer * 2; i++) {
        printf(" ");
    }
    printf("%s", ASTNodeTypeName[parent->type]);

    switch(parent->type) {
        case AST_TYPE: case AST_ID:
            printf(": %s\n", parent->val.c); break;
        case AST_INT:
            printf(": %d\n", parent->val.i); break;
        case AST_FLOAT:
            printf(": %lf\n", parent->val.d); break;

        case AST_Program:    case AST_ExtDefList: case AST_ExtDef:
        case AST_ExtDecList: case AST_Specifier:  case AST_StructSpecifier:
        case AST_OptTag:     case AST_Tag:        case AST_VarDec:
        case AST_FunDec:     case AST_VarList:    case AST_ParamDec:
        case AST_CompSt:     case AST_StmtList:   case AST_Stmt:
        case AST_DefList:    case AST_Def:        case AST_DecList:
        case AST_Dec:        case AST_Exp:        case AST_Args:
        case AST_Error:
            printf(" (%d)\n", parent->lineno); break;

        default : printf("\n");
    }

    for (TreeNode *p = parent->child; p != NULL; p = p->sibling) {
        printfTree(p, layer + 1);
    }
}
