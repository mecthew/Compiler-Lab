#include <malloc.h>
#include <string.h>
#include "symbolTable.h"
#include "common.h"
#include "type.h"
#include "syntax.tab.h"

SymbolNode** symbolTable;

unsigned int hashIndex(char* name)
{
	unsigned int val = 0, i;
	for(; *name; ++name)
	{
		val = (val << 2) + *name;
		if(i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

void initSymbolTable()
{
	/*calloc function initializes all space to 0*/
	symbolTable = (SymbolNode**)calloc(0x3fff, sizeof(SymbolNode*));
	
	Type* p1 = (Type*)malloc(sizeof(Type));
	p1->kind = FUNCTION;
	FieldList* q1 = (FieldList*)malloc(sizeof(FieldList));
	q1->name = "read";
	q1->type = (Type*)malloc(sizeof(Type));
	q1->type->kind = BASIC;
	q1->type->u.basic = INT;
	q1->tail = NULL;
	p1->u.structure = q1;
	insertSymbolTable("read", p1);

	Type* p2 = (Type*)malloc(sizeof(Type));
	p2->kind = FUNCTION;
	FieldList* q2 = (FieldList*)malloc(sizeof(FieldList));
	q2->name = "write";
	q2->type = (Type*)malloc(sizeof(Type));
	q2->type->kind = BASIC;
	q2->type->u.basic = INT;
	FieldList* q3 = (FieldList*)malloc(sizeof(FieldList));
	q3->name = NULL;
	q3->type = (Type*)malloc(sizeof(Type));
	q3->type->kind = BASIC;
	q3->type->u.basic = INT;
	q3->tail = NULL;
	q2->tail = q3;
	p2->u.structure = q2;
	insertSymbolTable("write", p2);
}

SymbolNode* insertSymbolTable(char* name, Type* type)
{
	SymbolNode* node = (SymbolNode*)malloc(sizeof(SymbolNode));
	strcpy(node->name, name);
	node->type = type;
	unsigned int hash_num = hashIndex(name);
	SymbolNode* nodePoint = symbolTable[hash_num];

	while(nodePoint && strcmp(nodePoint->name,name))
		nodePoint=nodePoint->next;
	if(nodePoint != NULL)
		return NULL;
	node->next = symbolTable[hash_num];
	symbolTable[hash_num] = node;
	return symbolTable[hash_num];
}

SymbolNode* searchSymbolTable(char* name)
{
	unsigned int hash_num = hashIndex(name);
	SymbolNode* point = symbolTable[hash_num];
	while(point && strcmp(point->name, name))
		point = point->next;
	return point;
}

void freeSymbolTable(){
	for(int i=0; i < 0x3fff; ++i){
		SymbolNode* cur = symbolTable[i];
		while(cur){
			SymbolNode* tmp = cur;
			cur = cur->next;
			free(tmp);
		}
	}
	free(symbolTable);
}
