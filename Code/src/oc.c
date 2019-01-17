#include "oc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


LvaList *lva_list = NULL;
int lva_off = 0, param_off = 0;
Reg t_regs[10];
static FILE* fp;

void generate_oc(char* filename) {
	fp = fopen(filename, "w");
	if(fp == NULL)
	{
		printf("Can't open the file to write IR!\n");
		return;
	}
	gen_data_seg();
    gen_global_seg();
    InterCodes *ics = translate_Program(ASTroot);
	gen_text_seg(ics);
}

void gen_data_seg() {
    fprintf(fp, ".data\n");
    fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(fp, "_ret: .asciiz \"\\n\"\n");
}

void gen_global_seg() {
    fprintf(fp,".globl main\n");
}

void gen_read_func() {
    fprintf(fp, "\n");
    fprintf(fp, "read:\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _prompt\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 5\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  jr $ra\n");
}

void gen_write_func() {
    fprintf(fp,"\n");
    fprintf(fp, "write:\n");
    fprintf(fp, "  li $v0, 1\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  li $v0, 4\n");
    fprintf(fp, "  la $a0, _ret\n");
    fprintf(fp, "  syscall\n");
    fprintf(fp, "  move $v0, $0\n");
    fprintf(fp, "  jr $ra\n");
}

void gen_text_seg(InterCodes* ics) {
    init_reg();
    fprintf(fp, ".text\n");
    gen_read_func();
    gen_write_func();
    InterCodes* ic = ics;
    for (; ic != NULL; ic = ic->next) {
        switch (ic->code.kind) {
            case ILABEL: {
                fprintf(fp,"label%d:\n", ic->code.u.label->u.label_no);
                break;
            }
            case IFUNCTION: {
                fprintf(fp, "\n");
                fprintf(fp, "%s:\n", getOperandName(ic->code.u.func));
                clear_lvas();
                gen_prologue();
                break;
            }
            case IASSIGN: {
				bool isAddrLeft = (ic->code.u.assign.left->kind == ADDRESS);
                Reg* rr = get_reg(ic->code.u.assign.left, isAddrLeft);
                Reg* r1 = get_reg(ic->code.u.assign.right, false);
                fprintf(fp,"  move %s, %s\n", rr->name, r1->name);
                free_reg(r1);
				if(ic->code.u.assign.left->kind == ADDRESS){
					Reg* r3 = get_reg(ic->code.u.binop.result, isAddrLeft);
					fprintf(fp, "  sw %s, 0(%s)\n", rr->name, r3->name);
					free_reg(rr);
					free_reg(r3);
				}
				else
					spill_reg(rr);
                break;
            }
            case IADD: {
				bool isAddrLeft = (ic->code.u.assign.left->kind == ADDRESS);
                Reg* rr = get_reg(ic->code.u.binop.result, isAddrLeft);
                Reg* r1 = get_reg(ic->code.u.binop.op1, false);
                Reg* r2 = get_reg(ic->code.u.binop.op2, false);
                fprintf(fp, "  add %s, %s, %s\n", rr->name, r1->name, r2->name);
                free_reg(r1);
                free_reg(r2);
				if(ic->code.u.binop.result->kind == ADDRESS){
					Reg* r3 = get_reg(ic->code.u.binop.result, isAddrLeft);
					fprintf(fp, "  sw %s, 0(%s)\n", rr->name, r3->name);
					free_reg(rr);
					free_reg(r3);
				}
				else
					spill_reg(rr);
                break;
            }
            case ISUB: {
				bool isAddrLeft = (ic->code.u.assign.left->kind == ADDRESS);
                Reg* rr = get_reg(ic->code.u.binop.result, isAddrLeft);
                Reg* r1 = get_reg(ic->code.u.binop.op1, false);
                Reg* r2 = get_reg(ic->code.u.binop.op2, false);
                fprintf(fp, "  sub %s, %s, %s\n", rr->name, r1->name, r2->name);
                free_reg(r1);
                free_reg(r2);
				if(ic->code.u.binop.result->kind == ADDRESS){
					Reg* r3 = get_reg(ic->code.u.binop.result, isAddrLeft);
					fprintf(fp, "  sw %s, 0(%s)\n", rr->name, r3->name);
					free_reg(rr);
					free_reg(r3);
				}
				else
					spill_reg(rr);
                break;
            }
            case IMUL: {
				bool isAddrLeft = (ic->code.u.assign.left->kind == ADDRESS);
                Reg* rr = get_reg(ic->code.u.binop.result, isAddrLeft);
                Reg* r1 = get_reg(ic->code.u.binop.op1, false);
                Reg* r2 = get_reg(ic->code.u.binop.op2, false);
                fprintf(fp, "  mul %s, %s, %s\n", rr->name, r1->name, r2->name);
                
                free_reg(r1);
                free_reg(r2);
				if(ic->code.u.binop.result->kind == ADDRESS){
					Reg* r3 = get_reg(ic->code.u.binop.result, isAddrLeft);
					fprintf(fp,"  sw %s, 0(%s)\n", rr->name, r3->name);
					free_reg(rr);
					free_reg(r3);
				}
				else
					spill_reg(rr);
                break;
            }
            case IDIV: {
				bool isAddrLeft = (ic->code.u.assign.left->kind == ADDRESS);
                Reg* rr = get_reg(ic->code.u.binop.result, isAddrLeft);
                Reg* r1 = get_reg(ic->code.u.binop.op1, false);
                Reg* r2 = get_reg(ic->code.u.binop.op2,false);
                fprintf(fp, "  div %s, %s\n", r1->name, r2->name);
                fprintf(fp,"  mflo %s\n", rr->name);
                free_reg(r1);
                free_reg(r2);
				if(ic->code.u.binop.result->kind == ADDRESS){
					Reg* r3 = get_reg(ic->code.u.binop.result, isAddrLeft);
					fprintf(fp,"  sw %s, 0(%s)\n", rr->name, r3->name);
					free_reg(rr);
					free_reg(r3);
				}
				else
					spill_reg(rr);
                break;
            }
            case IGOTO: {
                fprintf(fp, "  j label%d\n", ic->code.u.label->u.label_no);
                break;
            }
            case IIFSTMT: {
                char *ins;
				char *relop = ic->code.u.ifstmt.relop;
				if(!strcmp(relop, "==")) ins = "beq";
				else if(!strcmp(relop, "!=")) ins = "bne";
				else if(!strcmp(relop, ">")) ins = "bgt";
				else if(!strcmp(relop, "<")) ins = "blt";
				else if(!strcmp(relop, ">=")) ins = "bge";
				else if(!strcmp(relop, "<=")) ins = "ble";
				else assert(0);
		
                Reg* r1 = get_reg(ic->code.u.ifstmt.left, false);
                Reg* r2 = get_reg(ic->code.u.ifstmt.right, false);
                fprintf(fp,"  %s %s, %s, label%d\n", ins, r1->name, r2->name,
                    ic->code.u.ifstmt.label->u.label_no);
                free_reg(r1);
                free_reg(r2);
                break;
            }
            case IRETURN: {
                Reg* rr = get_reg(ic->code.u.return_val, false);
                fprintf(fp, "  move $v0, %s\n", rr->name);
                // invoke gen_epilogue() after get_reg(...)
                // avoid to reset $fp early
                gen_epilogue();
                fprintf(fp, "  jr $ra\n");
                free_reg(rr);
                break;
            }
            case IDEC: {
                add_array2lva(ic->code.u.dec.left, ic->code.u.dec.size);
                break;
            }
            case IARG: {
                // put all args on stack
                Reg* rr = get_reg(ic->code.u.arg, false);
                fprintf(fp,"  addi $sp, $sp, -4\n");
                fprintf(fp,"  sw %s, 0($sp)\n", rr->name);
                free_reg(rr);
                break;
            }
            case ICALL: {
                fprintf(fp,"  addi $sp, $sp, -4\n");
                fprintf(fp,"  sw $ra, 0($sp)\n");
                fprintf(fp,"  jal %s\n", getOperandName(ic->code.u.callfunc.right));
                fprintf(fp,"  lw $ra, 0($sp)\n");
                fprintf(fp,"  addi $sp, $sp, 4\n");
                // don't invoke get_reg(...) before 'jal ...'
                // avoid to allocate stack between ARG... and CALL...
                Reg* rr = get_reg(ic->code.u.callfunc.left, false);
                fprintf(fp,"  move %s, $v0\n", rr->name);
                spill_reg(rr);
                break;
            }
            case IPARAM: {
                add_param2lva(ic->code.u.param);
                break;
            }
            case IREAD: {
                Reg* rr = get_reg(ic->code.u.read_val, false);
                fprintf(fp,"  addi $sp, $sp, -4\n");
                fprintf(fp,"  sw $ra, 0($sp)\n");
                fprintf(fp,"  jal read\n");
                fprintf(fp,"  lw $ra, 0($sp)\n");
                fprintf(fp,"  addi $sp, $sp, 4\n");
                fprintf(fp,"  move %s, $v0\n", rr->name);
                spill_reg(rr);
                break;
            }
            case IWRITE: {
                Reg* rr = get_reg(ic->code.u.write_val, false);
                fprintf(fp,"  move $a0, %s\n", rr->name);
                free_reg(rr);

                fprintf(fp,"  addi $sp, $sp, -4\n");
                fprintf(fp,"  sw $ra, 0($sp)\n");
                fprintf(fp,"  jal write\n");
                fprintf(fp,"  lw $ra, 0($sp)\n");
                fprintf(fp,"  addi $sp, $sp, 4\n");
                break;
            }
            default: assert(0);
        }
    }
}

Reg* get_reg(Operand opd, bool isAddrLeft) {
    for (int i = 0; i < 10; i ++) {
        if (t_regs[i].unused) {
            t_regs[i].unused = false;
            if (opd->kind == CONSTANT) {
                fprintf(fp,"   li %s, %d\n", t_regs[i].name, opd->u.value);
            }
			else if(opd->kind == REFERENCE){
				t_regs[i].lva = get_lva(opd);
				fprintf(fp,"  la %s, %d($fp)\n", t_regs[i].name, t_regs[i].lva->off);
			}
			else if(opd->kind == ADDRESS){
				t_regs[i].lva = get_lva(opd);
				if(isAddrLeft){
					fprintf(fp,"  lw %s, %d($fp)\n", t_regs[i].name, t_regs[i].lva->off);
				}
				else{
					Reg* r_free = get_free_reg();
					fprintf(fp,"  lw %s, %d($fp)\n", r_free->name, t_regs[i].lva->off);
					fprintf(fp,"  lw %s, 0(%s)\n", t_regs[i].name, r_free->name);
					free_reg(r_free);
				}
			}
            else {
                t_regs[i].lva = get_lva(opd);
                fprintf(fp,"  lw %s, %d($fp)\n", t_regs[i].name, t_regs[i].lva->off);
            }
            return t_regs + i;
        }
    }
    assert(0);
}

Reg* get_free_reg(){
	for(int i = 0; i < 10; ++i){
		if(t_regs[i].unused)
			return t_regs+i;
	}
	assert(0);
}

void spill_reg(Reg* r) {
    int off = r->lva->off;
    fprintf(fp,"  sw %s, %d($fp)\n", r->name, off);
    free_reg(r);
}

void free_reg(Reg* r) {
    r->unused = true;
    r->lva = NULL;
}

void init_reg() {
    for (int i = 0; i < 10; i ++) {
        t_regs[i].name[0] = '$';
        t_regs[i].name[1] = 't';
        t_regs[i].name[2] = '0' + i;
        t_regs[i].unused = true;
        t_regs[i].lva = NULL;
    }
}

LocalVarAddr* get_lva(Operand opd) {
    LvaList *node = lva_list;
    for (; node != NULL; node = node->next) {
        if (strcmp(node->lva->name, getOperandName(opd)) == 0) {
            return node->lva;
        }
		else if(opd->kind == REFERENCE && strcmp(node->lva->name, opd->u.refer_name) == 0){  //for &
				return node->lva;
		}
		else if(opd->kind == ADDRESS && strcmp(node->lva->name, get_temp_name(opd->u.temp_no))==0){
				return node->lva;
		}
    }
    return add_lva(opd);        
}

LocalVarAddr* add_lva(Operand opd) {
    LocalVarAddr* lva = (LocalVarAddr*)malloc(sizeof(LocalVarAddr));
	lva->name = getOperandName(opd);
    lva->off = (lva_off -= 4);
    LvaList *node = (LvaList*)malloc(sizeof(LvaList));
    node->lva = lva;
    node->next = lva_list;
    lva_list = node;
    fprintf(fp,"  addi $sp, $sp, -4\n"); // allocate
    return lva;
}

void add_array2lva(Operand opd, int size) {
    //assert(opd->kind == OP_TEMP);
    LocalVarAddr* lva = (LocalVarAddr*)malloc(sizeof(LocalVarAddr));
	lva->name = getOperandName(opd);
    lva->off = (lva_off -= size);
    LvaList *node = (LvaList*)malloc(sizeof(LvaList));
    node->lva = lva;
    node->next = lva_list;
    lva_list = node;
    fprintf(fp,"  addi $sp, $sp, -%d\n", size); // allocate
}

void add_param2lva(Operand opd) {
    //assert(opd->kind == OP_VARIABLE);
    LocalVarAddr* lva = (LocalVarAddr*)malloc(sizeof(LocalVarAddr));
	lva->name = getOperandName(opd);
    lva->off = (param_off += 4);
    LvaList *node = (LvaList*)malloc(sizeof(LvaList));
    node->lva = lva;
    node->next = lva_list;
    lva_list = node;
}

void clear_lvas() {
    lva_list = NULL;
    lva_off = 0;
    param_off = 4;
}

void gen_prologue() {
    fprintf(fp,"  addi $sp, $sp, -4\n");
    fprintf(fp,"  sw $fp, 0($sp)\n");
    fprintf(fp,"  move $fp, $sp\n");
}

void gen_epilogue() {
    fprintf(fp,"  move $sp, $fp\n");
    fprintf(fp,"  lw $fp, 0($sp)\n");
    fprintf(fp,"  addi $sp, $sp, 4\n");
}

void gen_addr(Reg* r, Operand opd) {
    LocalVarAddr* lva = get_lva(opd);
    fprintf(fp,"  la %s, %d($fp)\n", r->name, lva->off);
}

