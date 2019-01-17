#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "semantic.h"
#include "syntax.tab.h"
#include "common.h"

void semanticCheck(TreeNode* root)
{
	visitProgram(root);
}

void visitProgram(TreeNode* root)
{
	TreeNode* child = root->child;
	//Program --> ExtDefList
	visitExtDefList(child);
}

void visitExtDefList(TreeNode* root)
{
	TreeNode* child = root->child;
	//ExtDefList --> Empty
	if(child == NULL)
		return;

	//ExtDefList --> ExtDef ExtDefList
	visitExtDef(child);
	visitExtDefList(child->sibling);
}

void visitExtDef(TreeNode* root)
{
	TreeNode* child = root->child;
	Type* specifier = visitSpecifier(child);
	if(specifier == NULL)
		return;
	child = child->sibling;
	//ExtDef --> Specifier ExtDecList SEMI
	if(child->type == AST_ExtDecList)
	{
		visitExtDecList(child, specifier);
	}
	//ExtDef --> Specifier SEMI
	else if(child->type == AST_SEMI)
	{
		return;
	}
	//ExtDef --> Specifier FunDec CompSt
	else if(child->type == AST_FunDec)
	{
		Type* type = visitFunDec(child, specifier);
		if(type == NULL)
			return;
		SymbolNode* temp_node = searchSymbolTable(type->u.structure->name);
		if(temp_node != NULL)
		{
			printf("Error type 4 at Line %d: Redefined function \"%s\".\n", child->lineno, temp_node->name);
			return;
		}
		else
		{
			insertSymbolTable(type->u.structure->name, type);
			child = child->sibling;
			visitCompSt(child, specifier);
		}

	}
	else
	{
		printf("Error in visitExtDef\n");
		exit(-1);
	}

}

Type* visitFunDec(TreeNode* root, Type* lastType)
{
	Type* type = (Type*)malloc(sizeof(Type));
	TreeNode* child = root->child;
	type->kind = FUNCTION;
	type->u.structure = (FieldList*)malloc(sizeof(FieldList));
	type->u.structure->name = child->val.c;
	type->u.structure->type = lastType;
	FieldList* field = type->u.structure;
	child = child->sibling->sibling;
	//FunDec --> ID LP RP
	if(child->type == AST_RP)
	{
		field->tail = NULL;
	}
	//FunDec --> ID LP VarList RP
	else
	{
		/*	
		VarList --> ParamDec COMMA VarList
				  | ParamDec
		ParamDec --> Specifier VarDec
		*/
		TreeNode* ParamDecNode = child->child;
		while(ParamDecNode){
			Type* specifierType = visitSpecifier(ParamDecNode->child);
			if(specifierType != NULL)
			{
				FieldList* tempfield = visitVarDec(ParamDecNode->child->sibling, specifierType, false);
				field->tail = tempfield;
				field = tempfield;
			}
			//VarList --> ParamDec
			if(ParamDecNode->sibling == NULL)
				break;
			//VarList --> ParamDec COMMA VarList
			else
				ParamDecNode = ParamDecNode->sibling->sibling->child;
		}
	}
	return type;
}

//CompSt --> LC DefList StmtList RC
void visitCompSt(TreeNode* root, Type* returnType)
{
	TreeNode* child = root->child->sibling;
	FieldList* head = visitDefList(child, false);
	child = child->sibling;
	while(true)
	{
		//StmtList --> empty
		if(child->child == NULL)
			break;
		//StmtList --> Stmt StmtList
		else
		{
			child = child->child;
			visitStmt(child, returnType);
			child = child->sibling;
		}
	}
}

void visitStmt(TreeNode* root, Type* returnType)
{
	TreeNode* child = root->child;
	//Stmt --> Exp SEMI
	if(child->type ==  AST_Exp)
		visitExp(child);
	//Stmt --> CompSt
	else if(child->type == AST_CompSt)
	{
		visitCompSt(child, returnType);
	}
	//Stmt --> RETURN Exp SEMI
	else if(child->type == AST_RETURN)
	{
		Type* exptype = visitExp(child->sibling);
		if(isSameType(returnType, exptype) == false)
		{
			printf("Error type 8 at Line %d: Type mismatched for return.\n", child->lineno);
			return;
		}
	}
	//Stmt --> IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
	else if(child->type == AST_IF)
	{
		child = child->sibling->sibling;
		Type* point = visitExp(child);
		if(point==NULL)
			return;
		if(!(point->kind==BASIC && point->u.basic==INT))
		{
			printf("Error type 7 at Line %d: Type mismatched for if.\n", child->lineno);
			return;
		}
		child = child->sibling->sibling;
		visitStmt(child, returnType);
		child = child->sibling;
		if(child != NULL)
			visitStmt(child->sibling, returnType);
	}
	//Stmt --> WHILE LP Exp RP Stmt
	else if(child->type == AST_WHILE)
	{
		child = child->sibling->sibling;
		Type* point = visitExp(child);
		if(point==NULL)
			return;
		if(!(point->kind==BASIC && point->u.basic==INT))
		{
			printf("Error type 7 at Line %d: Type mismatched for while.\n",child->lineno);
			return;
		}
		child = child->sibling->sibling;
		visitStmt(child, returnType);
	}
	else
	{
		printf("Error in visitStmt\n");
		exit(-1);
	}
}

//ExtDecList --> VarDec | VarDec COMMA ExtDecList
void visitExtDecList(TreeNode* root, Type* type)
{
	TreeNode* child = root->child;
	while(true)
	{
		visitVarDec(child, type, false);
		child = child->sibling;
		if(child == NULL)
			break;
		else
		{
			child = child->sibling->child;
		}
	}
}

Type* visitSpecifier(TreeNode* root)
{
	TreeNode* child = root->child;
	//Specifier --> TYPE
	if(child->type == AST_TYPE)
	{
		Type* type = (Type*)malloc(sizeof(Type));
		type->kind = BASIC;
		if(strcmp(child->val.c, "int")==0)
			type->u.basic = INT;
		else if(strcmp(child->val.c, "float")==0)
			type->u.basic = FLOAT;
		return type;
	}
	//Specifier --> StructSpecifier
	else if(child->type == AST_StructSpecifier)
	{
		child = child->child->sibling;
		//StructSpecifier --> STRUCT OptTag LC DefList RC
		if(child->type == AST_OptTag || child->child == NULL)
		{
			char* name = NULL;
			//OptTag --> ID | Empty
			if(child->type == AST_OptTag)
			{
				name = child->child->val.c;
				SymbolNode* temp_node = searchSymbolTable(name);
				if(temp_node != NULL)
				{
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", child->lineno, name);
					return NULL;
				}
			}
			child = child->sibling->sibling;
			FieldList* head = NULL;
			//DefList --> Def DefList
			if(child->child != NULL)
			{
				head = visitDefList(child, true);
			}
			Type* type = (Type*)malloc(sizeof(Type));
			type->kind = STRUCTURE;
			type->u.structure = head;
			if(name != NULL)
				insertSymbolTable(name, type);
			return type;
		}
		//StructSpecifier --> STRUCT Tag
		else if(child->type == AST_Tag)
		{
			child = child->child;
			char* temp_name = child->val.c;
			SymbolNode* temp_node = searchSymbolTable(temp_name);
			if(temp_node == NULL)
			{
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", child->lineno, child->val.c);
				return NULL;
			}	
			else
			{
				return temp_node->type;
			}
		}
		else
		{
			printf("Error in StructSpecifier\n");
			exit(-1);
		}
	}
	else
	{
		printf("Error in visitSpecifier\n");
		exit(-1);
	}
}

//DefList --> Def DefList | Empty
FieldList* visitDefList(TreeNode* root, bool inStruct)
{
	//DefList --> Empty
	if(root->child == NULL)
		return NULL;
	//DefList --> Def DefList
	FieldList* head = NULL;
	TreeNode* child = root->child;
	FieldList* field = visitDef(child, inStruct);
	head = field;
	
	child = child->sibling;
	FieldList* nexthead = visitDefList(child, inStruct);
	FieldList* temp_head = head;
	FieldList* temp_tail = head;
	for(; temp_head!=NULL; )
	{
		temp_tail = temp_head;
		temp_head = temp_head->tail;
	}
	if(temp_tail == NULL)
		head = nexthead;
	else
		temp_tail->tail = nexthead;
	return head;
}

//Def --> Specifier DecList SEMI
FieldList* visitDef(TreeNode* root, bool inStruct)
{
	TreeNode* child = root->child;
	Type* type = visitSpecifier(child);
	if(type == NULL)
		return NULL;
	FieldList* field = NULL;
	//DecList --> Dec | Dec COMMA DecList
	child = child->sibling->child;
	while(true)
	{
		//Dec --> VarDec | VarDec ASSIGNOP Exp
		TreeNode* varDec = child->child;
		FieldList* result = visitVarDec(varDec, type, inStruct);
		//Dec --> VarDec ASSIGNOP Exp
		if(varDec->sibling != NULL)
		{
			if(inStruct == true)
			{
				printf("Error type 15 ar Line %d: Illegal initialization in structure.\n", varDec->lineno);
			}
			else
			{
				Type* exptype = visitExp(varDec->sibling->sibling);
				if(isSameType(type, exptype) == false)
				{
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", varDec->lineno);
				}
			}
		}
		if(result != NULL)
		{
			FieldList* newField = (FieldList*)malloc(sizeof(FieldList));
			newField->name = result->name;
			newField->type = result->type;
			newField->tail = field;
			field = newField;
		}
		child = child->sibling;
		if(child != NULL)
			child = child->sibling->child;
		else
			break;

	}
	return field;
}

//VarDec --> ID | VarDec LB INT RB
FieldList* visitVarDec(TreeNode* root, Type* type, bool inStruct)
{
	Type* lastType = type;
	TreeNode* child = root->child;
	FieldList* result = (FieldList*)malloc(sizeof(FieldList));
	TreeNode* next;
	while(child->type == AST_VarDec)
	{
		Type* newtype = (Type*)malloc(sizeof(Type));
		newtype->kind = ARRAY;
		next = child->sibling->sibling;
		newtype->u.array.size = next->val.i;
		newtype->u.array.elem = lastType;
		lastType = newtype;
		//next = next->sibling;
		child = child->child;
	}

	result->name = child->val.c;
	result->type = lastType;
	result->tail = NULL;

	SymbolNode* temp_node = insertSymbolTable(result->name, result->type);
	if(temp_node == NULL)
	{
		if(inStruct)
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n", child->lineno, result->name);
		else
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", child->lineno, result->name);
	}
	return result;
}

Type* visitExp(TreeNode* root)
{
	TreeNode* child = root->child;
	if(child->type == AST_Exp)
	{
		//Exp --> Exp ASSIGNOP Exp
		if(child->sibling->type==AST_ASSIGNOP)
		{
			TreeNode* node = child->child;
			if(!((node->type==AST_ID && node->sibling == NULL) || (node->type==AST_Exp && node->sibling->type==AST_DOT) \
				|| (node->type==AST_Exp && node->sibling->type==AST_LB && node->sibling->sibling->type==AST_Exp)))
			{
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child->lineno);
				return NULL;
			}
			Type* type1 = visitExp(child);
			Type* type2 = visitExp(child->sibling->sibling);
			if(isSameType(type1,type2) == false)
			{
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp AND Exp | Exp OR Exp
		else if(child->sibling->type==AST_AND || child->sibling->type==AST_OR)
		{
			Type* type1 = visitExp(child);
			Type* type2 = visitExp(child->sibling->sibling);
			if(!(isSameType(type1, type2) && type1->kind==BASIC && type1->u.basic==INT))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp RELOP Exp
		else if(child->sibling->type==AST_RELOP)
		{
			Type* type1 = visitExp(child);
			Type* type2 = visitExp(child->sibling->sibling);
			if(!(isSameType(type1, type2) && type1->kind==BASIC))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			Type* newtype = (Type*)malloc(sizeof(Type));
			newtype->kind = BASIC;
			newtype->u.basic = INT;
			return newtype;
		}
		//Exp --> Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
		else if(child->sibling->type==AST_PLUS || child->sibling->type==AST_MINUS \
			|| child->sibling->type==AST_STAR || child->sibling->type==AST_DIV)
		{
			Type* type1 = visitExp(child);
			Type* type2 = visitExp(child->sibling->sibling);
			if(!(isSameType(type1, type2) && type1->kind==BASIC))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp LB Exp RB
		else if(child->sibling->type==AST_LB)
		{
			Type* type1 = visitExp(child);
			if(type1 == NULL)
				return NULL;
			else if(type1->kind != ARRAY)
			{
				TreeNode* lastChild = child;
				while(lastChild->child)
					lastChild = lastChild->child;
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n", lastChild->lineno, lastChild->val.c);
				return NULL;
			}
			else
			{
				child = child->sibling->sibling;
				Type* type2 = visitExp(child);
				if(type2 == NULL)
					return NULL;
				else if(!(type2->kind == BASIC && type2->u.basic == INT))
				{
					/*val.d may cause error*/
					TreeNode* lastChild = child;
					while(lastChild->child)
						lastChild = lastChild->child;
					printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", lastChild->lineno, lastChild->val.c);
					return NULL;
				}
				else
					return type1->u.array.elem;
			}
		}
		//Exp --> Exp DOT ID
		else if(child->sibling->type==AST_DOT)
		{
			Type* type1 = visitExp(child);
			if(type1 == NULL)
				return NULL;
			else if(type1->kind != STRUCTURE)
			{
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->lineno);
				return NULL;
			}
			else
			{
				child = child->sibling->sibling;
				FieldList* field = type1->u.structure;
				for(; field!=NULL; field=field->tail)
					if(strcmp(field->name, child->val.c) == 0)
						break;
				if(field == NULL)
				{
					printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child->lineno, child->val.c);
					return NULL;					
				}
				else
				{
					return field->type;
				}

			}
		}
		else
		{
			printf("Error in visitExp Exp\n");
			exit(-1);
		}

	}
	else if(child->type==AST_LP)
	{
		if(child->sibling->type==AST_Exp)
		{
			return visitExp(child->sibling);
		}
		else
		{
			printf("Error in visitExp LP\n");
			exit(-1);
		}
	}
	else if(child->type==AST_MINUS)
	{
		if(child->sibling->type==AST_Exp)
		{
			Type* temp_type = visitExp(child->sibling);
			if(temp_type==NULL)
				return NULL;
			if(temp_type->kind != BASIC)
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return temp_type;
		}
		else
		{
			printf("Error in visitExp MINUS\n");
			exit(-1);
		}
	}
	else if(child->type==AST_NOT)
	{
		if(child->sibling->type==AST_Exp)
		{
			Type* temp_type = visitExp(child->sibling);
			if(temp_type==NULL)
				return NULL;
			if(temp_type->kind != BASIC || temp_type->u.basic != INT)
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			Type* newtype = (Type*)malloc(sizeof(Type));
			newtype->kind = BASIC;
			newtype->u.basic = INT;
			return newtype;
		}
		else
		{
			printf("Error in visitExp NOT\n");
			exit(-1);
		}
	}
	else if(child->type==AST_ID)
	{
		//Exp --> ID LP RP | ID LP Args RP
		if(child->sibling != NULL)
		{
			SymbolNode* temp_node = searchSymbolTable(child->val.c);
			if(temp_node == NULL)
			{
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child->lineno, child->val.c);
				return NULL;
			}
			else if(temp_node->type->kind != FUNCTION)
			{
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", child->lineno, child->val.c);
				return NULL;
			}
			else
			{
				Type* temp_type = temp_node->type;
				//Exp --> ID LP RP
				if(child->sibling->sibling->type==AST_RP)
				{
					if(temp_type->u.structure->tail == NULL)
					{
						return temp_type->u.structure->type;
					}
					else
					{
						printf("Error type 9 at Line %d: Too few arguments for Function \"%s\".\n",child->lineno, funcArgsToStr(temp_type));
						return NULL;
					}
				}
				//Exp --> ID LP Args RP
				else
				{
					FieldList* args = temp_type->u.structure->tail;
					if(args == NULL)
					{
						printf("Error type 9 at Line %d: Too many arguments for Function \"%s\".\n", child->lineno, funcArgsToStr(temp_type));
						return NULL;
					}
					char* function_name = child->val.c;
					for(child = child->sibling->sibling->child; ;)
					{
						Type* child_type = visitExp(child);
						if(child_type == NULL)
							return NULL;
						if(isSameType(child_type, args->type) == false)
						{
							printf("Error type 9 at Line %d: Arguments type mismatch in Function \"%s\".\n", child->lineno, function_name);
							return NULL;
						}
						args = args->tail;
						if(args == NULL && child->sibling != NULL)
						{
							printf("Error type 9 at Line %d: Too many arguments for Function \"%s\".\n", child->lineno, funcArgsToStr(temp_type));
							return NULL;
						}
						if(args != NULL && child->sibling == NULL)
						{
							printf("Error type 9 at Line %d: Too few arguments for Function \"%s\".\n", child->lineno, funcArgsToStr(temp_type));
							return NULL;
						}
						if(args == NULL && child->sibling == NULL)
							return temp_node->type->u.structure->type;
						
						child = child->sibling->sibling->child;
					}

				}
			}
		}
		//Exp --> ID
		else
		{
			SymbolNode* temp_node = searchSymbolTable(child->val.c);
			if(temp_node == NULL)
			{
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child->lineno, child->val.c);
				return NULL;
			}
			else
			{
				return temp_node->type;
			}
		}
	}
	else if(child->type==AST_INT ||child->type==AST_FLOAT)
	{
		Type* newtype = (Type*)malloc(sizeof(Type));
		newtype->kind = BASIC;
		newtype->u.basic = child->type==AST_INT?INT:FLOAT;
		return newtype;
	}
	else
	{
		printf("Error in visitExp\n");
		exit(-1);
	}
}

bool isSameType(Type* type1, Type* type2)
{
	if(type1 == NULL || type2 == NULL || type1->kind != type2->kind)
		return false;
	
	if(type1->kind == BASIC)
		return type1->u.basic == type2->u.basic;
	else if(type1->kind == ARRAY)
		return isSameType(type1->u.array.elem, type2->u.array.elem);
	else if(type1->kind == STRUCTURE || type1->kind == FUNCTION)
	{
		FieldList* p1 = type1->u.structure;
		FieldList* p2 = type2->u.structure;
		while(p1 != NULL && p2 != NULL)
		{
			if(!isSameType(p1->type, p2->type))
				return false;
			p1 = p1->tail;
			p2 = p2->tail;
		}
		if(p1 != NULL || p2 != NULL)
			return false;
		return true;
	}
	else
		return false;
}

char* typeToStr(Type* type){
	char* finalStr = (char*)malloc(128);
	finalStr[0] = '\0';
	switch(type->kind){
		case BASIC:{
			if(type->u.basic==INT)
				strcat(finalStr,"int");
			else
				strcat(finalStr,"float");
			break;
		}
		case ARRAY:{
			char* subStr = typeToStr(type->u.array.elem);
			strcat(subStr, "[]");
			strcpy(finalStr, subStr);
			free(subStr);
			break;
		}
		case STRUCTURE:{
			char* name = type->u.structure->name;
			strcat(finalStr,"struct ");
			strcat(finalStr, name);
			break;
		}
		default:
			break;
	}
	return finalStr;
}

char* funcArgsToStr(Type* funcType){
	if(funcType->kind != FUNCTION)
		return NULL;
	char* finalStr = (char*)malloc(1024);
	finalStr[0] = '\0';
	char* funcName = funcType->u.structure->name;
	strcat(finalStr, funcName);
	strcat(finalStr, "(");
	FieldList* nextArgs = funcType->u.structure->tail;
	int argsCnt = 0;
	while(nextArgs){
		char* subStr = typeToStr(nextArgs->type);
		if(argsCnt)
			strcat(finalStr, ",");
		strcat(finalStr,subStr);
		free(subStr);
		argsCnt++;
		nextArgs = nextArgs->tail;
	}
	strcat(finalStr, ")");
	return finalStr;
}
