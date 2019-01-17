#include <stdio.h>
#include <string.h>
#include "AST.h"
#include "common.h"

extern void yyrestart(FILE *);
extern int yyparse(void);
extern bool isError;
extern int yylineno;

int main(int argc, char **argv) {
    if (argc < 2) return 1;

	int i = 1;
	for(; i < argc; ++i){
		isError = false;
		yylineno = 1;
		printf("======================The %dth file: %s=====================\n", i, argv[i]);
    	FILE *fin = fopen(argv[i], "r");
    	if (!fin) {
        	perror(argv[i]);
        	return 1;
    	}
	
		yyrestart(fin);
		yyparse();
		if(!isError)
    		printfTree(ASTroot, 0);

    	fclose(fin);
	}
    return 0;
}
