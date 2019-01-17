#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include "type.h"

struct SymbolNode
{
	char name[32];
	Type* type;
	struct SymbolNode* next;
};
typedef struct SymbolNode SymbolNode;
extern SymbolNode** symbolTable;

unsigned int hashIndex(char* name);
void initSymbolTable();
SymbolNode* insertSymbolTable(char* name, Type* type);
SymbolNode* searchSymbolTable(char* name);
void freeSymbolTable();

#endif
