#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__
#include "AST.h"
#include "symbolTable.h"
#include "type.h"
#include "common.h"

void semanticCheck(TreeNode* root);

void visitProgram(TreeNode* root);

void visitExtDefList(TreeNode* root);

void visitExtDef(TreeNode* root);

Type* visitSpecifier(TreeNode* root);

FieldList* visitDefList(TreeNode* root, bool inStruct);

FieldList* visitDef(TreeNode* root, bool inStruct);

FieldList* visitVarDec(TreeNode* root, Type* type, bool inStruct);

Type* visitFunDec(TreeNode* root, Type* type);

void visitCompSt(TreeNode* root, Type* type);

void visitStmt(TreeNode* root, Type* returnType);

Type* visitFunDec(TreeNode* root, Type* type);

void visitExtDecList(TreeNode* root, Type* type);

Type* visitExp(TreeNode* root);

bool isSameType(Type* type1, Type* type2);

char* typeToStr(Type* type);

char* funcArgsToStr(Type* funcType);
#endif
