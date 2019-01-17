#include "semantic.h"
#include "symbolTable.h"
#include "AST.h"
#include "type.h"
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
	
	for(int i=1; i < argc; ++i){
		yylineno = 1;
		isError=false;
		printf("======================The %dth file: %s=====================\n", i, argv[i]);
		FILE *fin = fopen(argv[i], "r");
		if (!fin) {
			perror(argv[i]);
			exit(-1);
		}	

		yyrestart(fin);
		yyparse();
		initSymbolTable();
		semanticCheck(ASTroot);
		freeSymbolTable();

		fclose(fin);
	}
	return 0;

}
