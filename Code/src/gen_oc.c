#include "semantic.h"
#include "symbolTable.h"
#include "AST.h"
#include "type.h"
#include "ir_generator.h"
#include "oc.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>

extern void yyrestart(FILE *);
extern int yyparse();
extern bool isError;
extern int yylineno;

int main(int argc, char **argv){
	if(argc < 2)
		exit(-1);
	
	yylineno = 1;
	isError=false;
	FILE *fin = fopen(argv[1], "r");
	if (!fin) {
		perror(argv[1]);
		exit(-1);
	}	

	yyrestart(fin);
	yyparse();
	initSymbolTable();
	semanticCheck(ASTroot);
	generate_oc(argv[2]);

	freeSymbolTable();
	fclose(fin);
	return 0;

}
