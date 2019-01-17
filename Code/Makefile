# 编译器设定和编译选项
CC := gcc
CFLAGS := -lfl -ly -I./include -std=gnu11 -g

genAST:
	bison -d src/syntax.y -o src/syntax.tab.c
	flex -o src/lex.yy.c src/lexical.l
	$(CC) src/genAST.c src/syntax.tab.c src/AST.c $(CFLAGS) -o genAST
gen_semantic:
	bison -d src/syntax.y -o src/syntax.tab.c
	flex -o src/lex.yy.c src/lexical.l
	$(CC) src/gen_semantic.c src/symbolTable.c src/semantic.c src/syntax.tab.c src/AST.c $(CFLAGS) -o gen_semantic
gen_ir:
	bison -d src/syntax.y -o src/syntax.tab.c
	flex -o src/lex.yy.c src/lexical.l
	$(CC) src/gen_ir.c src/symbolTable.c src/semantic.c src/syntax.tab.c src/AST.c src/ir_structure.c src/ir_generator.c $(CFLAGS) -o gen_ir
parser:
	bison -d src/syntax.y -o src/syntax.tab.c
	flex -o src/lex.yy.c src/lexical.l
	$(CC) src/gen_oc.c src/oc.c src/symbolTable.c src/semantic.c src/syntax.tab.c src/AST.c src/ir_structure.c src/ir_generator.c $(CFLAGS) -o parser
testAST:
	./genAST `find ../Test/ -name "*.cmm" | sort -t '/' -k3.5n`
test:
	./parser `find ../Test/ -name "*.cmm" | sort -t '/' -k3.5n`
.PHONY: clean
clean:
	rm -f genAST gen_semantic gen_ir parser src/lex.yy.c src/syntax.tab.c src/syntax.tab.h


