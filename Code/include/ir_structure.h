#ifndef IR_STRUCTURE_H_
#define IR_STRUCTURE_H_

struct Operand_
{
	/*变量，常量，地址，引用，临时变量，跳转标签*/
	enum { VARIABLE, CONSTANT, ADDRESS, REFERENCE, TEMP, LABELNO} kind;
	union
	{
		char* var_name;
		int value;
		char* addr_name;
		char* refer_name;
		int temp_no;
		int label_no;
	} u;
};
typedef struct Operand_* Operand;

struct InterCode
{
	/*标签，函数，赋值，加法，减法，乘法，除法，GOTO语句，条件语句，RETURN，变量声明语句，传人参数，调用语句，声明参数，read函数，write函数*/
	enum {ILABEL, IFUNCTION, IASSIGN, IADD, ISUB, IMUL, IDIV, IGOTO, IIFSTMT, IRETURN, IDEC, IARG, ICALL, IPARAM, IREAD, IWRITE } kind;
	union
	{
		Operand label;
		Operand func;
		struct {Operand left, right; } assign;
		struct {Operand result, op1, op2; } binop;
		Operand go_to;
		struct {Operand left, right; char relop[3]; Operand label;} ifstmt;
		Operand return_val;
		struct {Operand left; int size;} dec;
		Operand arg;
		struct {Operand left, right;} callfunc;
		Operand param;
		Operand read_val;
		Operand write_val;
	} u;
};
typedef struct InterCode InterCode;

struct InterCodes
{
	InterCode code;
	struct InterCodes* prev;
	struct InterCodes* next;
};
typedef struct InterCodes InterCodes;

extern InterCodes* ir_head;
extern InterCodes* ir_tail;

struct ArgListNode
{
	Operand operand;
	struct ArgListNode* next;
};
typedef struct ArgListNode ArgListNode;

Operand mallocOperand(int kind, char* name);
InterCodes* mallocInterCodes();
InterCodes* mergeInterCodes(InterCodes* code1, InterCodes* code2);
void writeInterCodes(char* filename);
char* getOperandName(Operand operand);




#endif
