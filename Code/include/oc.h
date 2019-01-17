#ifndef __OC_H__
#define __OC_H__

#include "AST.h"
#include "ir_structure.h"
#include "ir_generator.h"
#include "common.h"

typedef struct {
	char* name;     //局部变量名
    int off;        //离帧指针$fp的偏移量
} LocalVarAddr;


typedef struct {
    char name[6];  //寄存器名
    LocalVarAddr *lva; //寄存器描述符，占用该寄存器变量
    bool unused;  //是否被占用
} Reg;

struct LvaList_ {
    LocalVarAddr *lva;
    struct LvaList_ *next;
};
typedef struct LvaList_ LvaList;

void generate_oc(char* filename);
void gen_data_seg();
void gen_global_seg();
void gen_text_seg(InterCodes* ics);
void gen_read_func();
void gen_write_func();
void init_reg();
Reg* get_reg(Operand opd, bool isAddrLeft);
Reg* get_free_reg();
void free_reg(Reg* r);
void spill_reg(Reg* r);
LocalVarAddr* get_lva(Operand opd);
LocalVarAddr* add_lva(Operand opd);
void add_param2lva(Operand opd);
void add_array2lva(Operand opd, int size);
void clear_lvas();
void gen_prologue();
void gen_epilogue();
void gen_addr(Reg* r, Operand opd);

#endif
