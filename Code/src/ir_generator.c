#include <stdlib.h>
#include<stdio.h>
#include <string.h>

#include "symbolTable.h"
#include "semantic.h"
#include "type.h"
#include "ir_generator.h"
#include "ir_structure.h"
#include "AST.h"

extern TreeNode *ASTroot;
int temp_index = -1;
int label_index = -1;

int getSize(Type* p)
{
	if(p->kind == BASIC)
		return 4;
	else if(p->kind == ARRAY)
	{
		return p->u.array.size*getSize(p->u.array.elem);
	}
	else
	{
		printf("Other types are not implemented in getSize()!\n");
		return 0;
	}
}

char* get_temp_name(int index)
{
	char* result;
	result = (char*)malloc(32);
	result[0] = 't';
	sprintf(result+1 ,"%d", index);
	return result;
}

char* get_label_name(int index)
{
	char* result;
	result = (char*)malloc(32);
	result[0] = 'L';
	sprintf(result+1, "%d", index);
	return result;
}

void generate_ir(char* filename)
{
	ir_head = translate_Program(ASTroot);
	printf("Complete translating the Program\n");
	writeInterCodes(filename);
}

InterCodes* translate_Program(TreeNode* root)
{
	if(root == NULL)
		return NULL;
	else if(root->type == AST_Program)
		return translate_Program(root->child);
	else if(root->type == AST_ExtDefList){
		TreeNode* sibling = root->child ? root->child->sibling : NULL;
		return mergeInterCodes(translate_Program(root->child), translate_Program(sibling));
	}
	else if(root->type == AST_ExtDef)
	{
		TreeNode* child = root->child->sibling;
		if(child->type == AST_FunDec)
		{
			if(child->sibling->type != AST_CompSt)
			{
				printf("Error in translate_Program()!\n");
				exit(-1);
			}
			InterCodes* fundec = translate_FunDec(child);
			InterCodes* compst = translate_CompSt(child->sibling);
			return mergeInterCodes(fundec, compst);
		}
		else	
			return NULL;
	}
	printf("Shouldn't reach here in translate_Program()\n");
	return NULL;
}

//FunDec --> ID LP VarList RP | ID LP RP
InterCodes* translate_FunDec(TreeNode* root)
{
	if(root == NULL)
	{
		printf("Error in translate_FunDec()!\n");
		exit(-1);
	}
	TreeNode* child = root->child;
	InterCodes* id_code = mallocInterCodes();
	id_code->code.kind = IFUNCTION;
	Operand id_operand = mallocOperand(VARIABLE, child->val.c);
	id_code->code.u.func = id_operand;
	child = child->sibling->sibling;
	
	//VarList --> ParamDec COMMA VarList | ParamDec
	if(child->type == AST_VarList)
	{
		InterCodes* tail = id_code;
		child = child->child;
		while(true)
		{
			TreeNode* temp_node = child->child->sibling;
			temp_node = temp_node->child;
			while(temp_node->type != AST_ID)
				temp_node = temp_node->child;
			InterCodes* temp_code = mallocInterCodes();
			temp_code->prev = tail;
			tail->next = temp_code;
			tail = temp_code;
			temp_code->code.kind = IPARAM;
			Operand temp_operand = mallocOperand(VARIABLE, temp_node->val.c);
			temp_code->code.u.param = temp_operand;
			child = child->sibling;
			if(child == NULL)
				break;
			else
				child = child->sibling->child;
		}
	}
	return id_code;
}

InterCodes* translate_CompSt(TreeNode* root)
{
	TreeNode* child = root->child->sibling;
	InterCodes* code1 = translate_DefList(child);
	InterCodes* code2 = translate_StmtList(child->sibling);
	return mergeInterCodes(code1, code2);
}

//DefList --> Def DefList | Empty
InterCodes* translate_DefList(TreeNode* root)
{
	if(root == NULL)
		return NULL;
	InterCodes* deflist_code = NULL;
	TreeNode* child = root->child;
	while(true && child != NULL)
	{ 
		//Def --> Specifier DecList SEMI
		TreeNode* dec = child->child->sibling->child;
		TreeNode* specifier = child->child;
		Type* type = visitSpecifier(specifier);

		while(true)
		{
			TreeNode* vardec = dec->child;
			//Dec --> VarDec ASSIGNOP Exp
			if(vardec->sibling != NULL)
			{
				Operand place = mallocOperand(VARIABLE, vardec->child->val.c);
				Operand t1 = (Operand)malloc(sizeof(struct Operand_));
				t1->kind = TEMP;
				temp_index++;
				t1->u.temp_no = temp_index;
				InterCodes* temp_code = translate_Exp(vardec->sibling->sibling, t1);
				InterCodes* code2 = mallocInterCodes();
				code2->code.kind = IASSIGN;
				code2->code.u.assign.left = place;
				code2->code.u.assign.right = t1;
				deflist_code = mergeInterCodes(mergeInterCodes(deflist_code, temp_code), code2);
			}
			//VarDec --> VarDec LB INT RB
			else if(vardec->child->type != AST_ID)
			{
				TreeNode* id = vardec;
				int size = getSize(type);
				while(id->type != AST_ID)
				{
					id = id->child;
					if(id->type != AST_ID)
						size *= id->sibling->sibling->val.i;
				}
				InterCodes* temp_code = mallocInterCodes();
				temp_code->code.kind = IDEC;
				temp_code->code.u.dec.left = mallocOperand(VARIABLE, id->val.c);
				temp_code->code.u.dec.size = size;
				deflist_code = mergeInterCodes(deflist_code, temp_code);
			}
			if(dec->sibling == NULL)
				break;
			else
				dec = dec ->sibling->sibling->child;
		}
		if(child->sibling == NULL)
			break;
		else
			child = child->sibling->child;
	}
	return deflist_code;
}

InterCodes* translate_StmtList(TreeNode* root)
{
	if(root == NULL || root->child == NULL)
		return NULL;
	else
	{
		TreeNode* stmt = root->child;
		return mergeInterCodes(translate_Stmt(stmt), translate_StmtList(stmt->sibling));
	}
}

InterCodes* translate_Stmt(TreeNode* root)
{
	InterCodes* result = NULL;
	TreeNode* child = root->child;
	if(child->type == AST_Exp)
	{
		Operand place = (Operand)malloc(sizeof(struct Operand_));
		return translate_Exp(child, place);
	}
	else if(child->type == AST_CompSt)
	{
		result = translate_CompSt(child);
	}
	else if(child->type ==  AST_RETURN)
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		InterCodes* code1 = translate_Exp(child->sibling, t1);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = IRETURN;
		code2->code.u.return_val = t1;
		result = mergeInterCodes(code1, code2);
	}
	else if(child->type == AST_IF)
	{
		TreeNode* stmt1 = child->sibling->sibling->sibling->sibling;
		if(stmt1->sibling == NULL)
		{
			InterCodes* label1 = mallocInterCodes();
			label1->code.kind = ILABEL;
			Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
			temp_l1->kind = LABELNO;
			label_index++;
			temp_l1->u.label_no = label_index;
			label1->code.u.label = temp_l1;	

			InterCodes* label2 = mallocInterCodes();
			label2->code.kind = ILABEL;
			Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
			temp_l2->kind = LABELNO;
			label_index++;
			temp_l2->u.label_no = label_index;
			label2->code.u.label = temp_l2;

			InterCodes* code1 = translate_Cond(child->sibling->sibling, label1, label2);
			InterCodes* code2 = translate_Stmt(stmt1);
			result = mergeInterCodes(code1, label1);
			result = mergeInterCodes(result, code2);
			result = mergeInterCodes(result, label2);
		}
		else
		{
			InterCodes* label1 = mallocInterCodes();
			label1->code.kind = ILABEL;
			Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
			temp_l1->kind = LABELNO;
			label_index++;
			temp_l1->u.label_no = label_index;
			label1->code.u.label = temp_l1;

			InterCodes* label2 = mallocInterCodes();
			label2->code.kind = ILABEL;
			Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
			temp_l2->kind = LABELNO;
			label_index++;
			temp_l2->u.label_no = label_index;
			label2->code.u.label = temp_l2;

			InterCodes* label3 = mallocInterCodes();	
			label3->code.kind = ILABEL;
			Operand temp_l3 = (Operand)malloc(sizeof(struct Operand_));
			temp_l3->kind = LABELNO;
			label_index++;
			temp_l3->u.label_no = label_index;
			label3->code.u.label = temp_l3;

			InterCodes* goto_code = mallocInterCodes();
			goto_code->code.kind = IGOTO;
			Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
			temp_ll->kind = LABELNO;
			temp_ll->u.label_no = label_index;
			goto_code->code.u.go_to = temp_ll;

			InterCodes* code1 = translate_Cond(child->sibling->sibling, label1, label2);
			InterCodes* code2 = translate_Stmt(stmt1);
			TreeNode* stmt2 = stmt1->sibling->sibling;
			InterCodes* code3 = translate_Stmt(stmt2);
			result = mergeInterCodes(code1, label1);
			result = mergeInterCodes(result, code2);
			result = mergeInterCodes(result, goto_code);
			result = mergeInterCodes(result, label2);
			result = mergeInterCodes(result, code3);
			result = mergeInterCodes(result, label3);
		}
	}
	else if(child->type == AST_WHILE)
	{
		InterCodes* label1 = mallocInterCodes();
		label1->code.kind = ILABEL;
		Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
		temp_l1->kind = LABELNO;
		label_index++;
		temp_l1->u.label_no = label_index;
		label1->code.u.label = temp_l1;

		InterCodes* label2 = mallocInterCodes();
		label2->code.kind = ILABEL;
		Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
		temp_l2->kind = LABELNO;
		label_index++;
		temp_l2->u.label_no = label_index;
		label2->code.u.label = temp_l2;

		InterCodes* label3 = mallocInterCodes();
		label3->code.kind = ILABEL;
		Operand temp_l3 = (Operand)malloc(sizeof(struct Operand_));
		temp_l3->kind = LABELNO;
		label_index++;
		temp_l3->u.label_no = label_index;
		label3->code.u.label = temp_l3;

		TreeNode* exp = child->sibling->sibling;
		InterCodes* code1 = translate_Cond(exp, label2, label3);
		InterCodes* code2 = translate_Stmt(exp->sibling->sibling);

		InterCodes* goto_code = mallocInterCodes();
		goto_code->code.kind = IGOTO;
		/*Operand temp_l4 = (Operand)malloc(sizeof(struct Operand_));
		temp_l4->kind = LABELNO;
		temp_l4->u.label_no = temp_11->u.label_no;*/
		goto_code->code.u.go_to = temp_l1;

		result = mergeInterCodes(label1, code1);
		result = mergeInterCodes(result, label2);
		result = mergeInterCodes(result, code2);
		result = mergeInterCodes(result, goto_code);
		result = mergeInterCodes(result, label3);
	}
	else
	{
		printf("Unexpected node in translate_Stmt()!\n");
		exit(-1);
	}
	return result;
}

//Args --> Exp COMMA Args | Exp
InterCodes* translate_Args(TreeNode* root, ArgListNode* arg_list)
{
	TreeNode* child = root->child;
	temp_index++;
	Operand t1 = (Operand)malloc(sizeof(struct Operand_));
	t1->kind = TEMP;
	t1->u.temp_no = temp_index;
	InterCodes* code1 = translate_Exp(child, t1);
	ArgListNode* new_arg = (ArgListNode*)malloc(sizeof(ArgListNode));
	new_arg->operand = t1;
	new_arg->next = arg_list->next;
	arg_list->next = new_arg;

	if(child->sibling == NULL)
		return code1;
	else
	{
		InterCodes* code2 = translate_Args(child->sibling->sibling, arg_list);
		return mergeInterCodes(code1, code2);
	}
}

InterCodes* translate_Exp(TreeNode* root, Operand place)
{
	TreeNode* child = root->child;
	TreeNode* op = child->sibling;
	InterCodes* exp_code = mallocInterCodes();
	//Exp --> LP Exp RP
	if(child->type == AST_LP)
	{
		return translate_Exp(child->sibling, place);
	}
	//Exp --> INT
	else if(child->type == AST_INT)
	{
		place->kind = CONSTANT;
		place->u.value = child->val.i;
		return NULL;
	}
	//Exp --> ID | ID LP Args RP | ID LP RP
	else if(child->type == AST_ID)
	{
		if(child->sibling == NULL)
		{
			SymbolNode* table = searchSymbolTable(child->val.c);
			if(table->type->kind == BASIC)
			{	
				place->kind = VARIABLE;
				place->u.var_name = child->val.c;
			}
			else
			{
				place->kind = REFERENCE;
				place->u.refer_name = child->val.c;
			}
			return NULL;
		}
		else
		{
			TreeNode* args = child->sibling->sibling;
			temp_index++;
			Operand new_temp = (Operand)malloc(sizeof(struct Operand_));
			new_temp->kind = TEMP;
			new_temp->u.temp_no = temp_index;
			if(args->type == AST_Args)
			{
				ArgListNode* arg_list = (ArgListNode*)malloc(sizeof(ArgListNode));
				arg_list->next = NULL;
				InterCodes* code1 = translate_Args(args, arg_list);
				if(args->sibling == NULL)
				{
					printf("Error in translate_Exp()!\n");
					exit(-1);
				}
				if(strcmp(child->val.c, "write") == 0)
				{
					/*InterCodes* mid_code = mallocInterCodes();
					mid_code->code.kind = IASSIGN;
					Operand temp = (Operand)malloc(sizeof(struct Operand_));
					temp->kind = VARIABLE;
					char* temp_var = (char*)malloc(32);
					temp_var[0] = 'v';
					temp_var[1] = 'v';
					temp_var[2] = 'v';
					temp_var[3] = '\0';
					temp->u.var_name = temp_var;
					mid_code->code.u.assign.left = temp;
					mid_code->code.u.assign.right = arg_list->next->operand;*/
					exp_code->code.kind = IWRITE;
					exp_code->code.u.write_val = arg_list->next->operand;
					return mergeInterCodes(code1, exp_code);
				}
				else
				{
					ArgListNode* arg_node = arg_list->next;
					InterCodes* code2 = NULL;
					while(arg_node != NULL)
					{
						InterCodes* code_arg = (InterCodes*)malloc(sizeof(InterCodes));
						code_arg->prev = NULL;
						code_arg->next = NULL;
						code_arg->code.kind = IARG;
						code_arg->code.u.arg = arg_node->operand;
						code2 = mergeInterCodes(code2, code_arg);
						arg_node = arg_node->next;
					}
					InterCodes* code_temp = mallocInterCodes();
					code_temp->code.kind = ICALL;
					code_temp->code.u.callfunc.left = new_temp;
					Operand callfunc_right = mallocOperand(VARIABLE, child->val.c);
					code_temp->code.u.callfunc.right = callfunc_right;

					InterCodes* code3 = mallocInterCodes();
					code3->code.kind = IASSIGN;
					code3->code.u.assign.left = place;
					code3->code.u.assign.right = new_temp;
					code3 = mergeInterCodes(code_temp, code3);
					return mergeInterCodes(mergeInterCodes(code1, code2), code3);
				}
			}
			else
			{
				if(strcmp(child->val.c, "read") == 0)
				{
					exp_code->code.kind = IREAD;
					exp_code->code.u.read_val = new_temp;
				}
				else
				{
					exp_code->code.kind = ICALL;
					exp_code->code.u.callfunc.left = new_temp;
					Operand func_name = mallocOperand(VARIABLE, child->val.c);
					exp_code->code.u.callfunc.right = func_name;
				}
				InterCodes* code2 = mallocInterCodes();
				code2->code.kind = IASSIGN;
				code2->code.u.assign.left = place;
				code2->code.u.assign.right = new_temp;
				return mergeInterCodes(exp_code, code2);
			}
		}
	}
	else if(child->type == AST_FLOAT)
	{
		printf("There shouldn't be float in translate_Exp()!\n");
		exit(-1);
	}
	//Exp --> Exp ASSIGNOP Exp
	else if(op->type == AST_ASSIGNOP)
	{
		InterCodes* code1 = translate_Exp(child, place);
		Operand temp_p = (Operand)malloc(sizeof(struct Operand_));
		temp_p->kind = TEMP;
		temp_index++;
		temp_p->u.temp_no = temp_index;
		InterCodes* code2 = translate_Exp(op->sibling, temp_p);
		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IASSIGN;
		code3->code.u.assign.left = place;
		code3->code.u.assign.right = temp_p;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
	}
	//Exp -->Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
	else if(op->type == AST_PLUS || op->type == AST_MINUS || op->type == AST_STAR || op->type == AST_DIV)
	{
		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = TEMP;
		temp_index++;
		p1->u.temp_no = temp_index;
		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		p2->kind = TEMP;
		temp_index++;
		p2->u.temp_no = temp_index;
		InterCodes* code1 = translate_Exp(child, p1);
		InterCodes* code2 = translate_Exp(op->sibling, p2);
		InterCodes* code3 = mallocInterCodes();
		if(op->type == AST_PLUS)
			code3->code.kind = IADD;
		else if(op->type == AST_MINUS)
			code3->code.kind = ISUB;
		else if(op->type == AST_STAR)
			code3->code.kind = IMUL;
		else
			code3->code.kind = IDIV;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = p1;
		code3->code.u.binop.op2 = p2;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
	}
	//Exp --> MINUS Exp
	else if(child->type == AST_MINUS)
	{
		Operand new_temp = (Operand)malloc(sizeof(struct Operand_));
		new_temp->kind = TEMP;
		temp_index++;
		new_temp->u.temp_no = temp_index;
		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = CONSTANT;
		p1->u.value = 0;
		InterCodes* code1 = translate_Exp(child->sibling, new_temp);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = ISUB;
		code2->code.u.binop.result = place;
		code2->code.u.binop.op1 = p1;
		code2->code.u.binop.op2 = new_temp;
		return mergeInterCodes(code1, code2);

	}
	//Exp --> NOT Exp | Exp AND Exp | Exp OR Exp | Exp RELOP Exp
	else if(child->type == AST_NOT || op->type == AST_RELOP || op->type == AST_AND || op->type == AST_OR)
	{
		InterCodes* label1 = mallocInterCodes();
		InterCodes* label2 = mallocInterCodes();
		label1->code.kind = ILABEL;
		label2->code.kind = ILABEL;
		Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
		temp_l1->kind = LABELNO;
		label_index++;
		temp_l1->u.label_no = label_index;
		Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
		temp_l2->kind = LABELNO;
		label_index++;
		temp_l2->u.label_no = label_index;
		label1->code.u.label = temp_l1;
		label2->code.u.label = temp_l2;

		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = CONSTANT;
		p1->u.value = 0;
		InterCodes* code0 = mallocInterCodes();
		code0->code.kind = IASSIGN;
		code0->code.u.assign.left = place;
		code0->code.u.assign.right = p1;
		InterCodes* code1 = translate_Cond(root, label1, label2);
		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		p2->kind = CONSTANT;
		p2->u.value = 1;
		InterCodes* code_temp = mallocInterCodes();
		code_temp->code.kind = IASSIGN;
		code_temp->code.u.assign.left = place;
		code_temp->code.u.assign.right = p2;
		InterCodes* code2 = mergeInterCodes(label1, code_temp);
		return mergeInterCodes(mergeInterCodes(mergeInterCodes(code0, code1), code2), label2);
	}
	//Exp --> Exp LB Exp RB
	else if(child->type == AST_Exp && op->type == AST_LB)
	{
		TreeNode* exp1 = child->child;
		InterCodes* code1 = NULL;
		Operand p1 = NULL;
		if(exp1->type == AST_ID)
			p1 = mallocOperand(REFERENCE, exp1->val.c);
		else
		{
			p1 = (Operand)malloc(sizeof(struct Operand_));
			p1->kind = TEMP;
			temp_index++;
			p1->u.temp_no = temp_index;
			code1 = translate_Exp(child, p1);
		}

		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		p2->kind = TEMP;
		temp_index++;
		p2->u.temp_no = temp_index;
		InterCodes* code2 = translate_Exp(op->sibling, p2);

		Type* root_type = visitExp(root);
		InterCodes* offset_code = mallocInterCodes();
		offset_code->code.kind = IMUL;
		Operand temp_result = (Operand)malloc(sizeof(struct Operand_));
		temp_result->kind = TEMP;
		temp_index++;
		temp_result->u.temp_no = temp_index;
		offset_code->code.u.binop.result = temp_result;
		Operand temp_op1 = (Operand)malloc(sizeof(struct Operand_));
		temp_op1->kind = CONSTANT;
		temp_op1->u.value = getSize(root_type);
		offset_code->code.u.binop.op1 = temp_op1;
		offset_code->code.u.binop.op2 = p2;

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IADD;
		if(code1 == NULL)
			code3->code.u.binop.op1 = p1;
		else
		{
			Operand temp_op3 = (Operand)malloc(sizeof(struct Operand_));
			temp_op3->kind = TEMP;
			temp_op3->u.temp_no = p1->u.temp_no;
			code3->code.u.binop.op1 = temp_op3;
		}
		code3->code.u.binop.op2 = offset_code->code.u.binop.result;
		temp_index++;
		place->u.temp_no = temp_index;
		Operand temp_op4 = (Operand)malloc(sizeof(struct Operand_));
		temp_op4->kind = TEMP;
		temp_op4->u.temp_no = place->u.temp_no;
		code3->code.u.binop.result = temp_op4;
		place->kind = ADDRESS;
		return mergeInterCodes(mergeInterCodes(code1, code2), mergeInterCodes(offset_code, code3));
	}
	//Exp --> Exp DOT ID
	else if(op->type == AST_DOT)
	{
		printf("Struct is not supported in translate_Exp()!\n");
		exit(-1);
	}
	else
	{
		printf("Unexpected expression in translate_Exp()!\n");
		return NULL;
	}
	return exp_code;
}

InterCodes* translate_Cond(TreeNode* root, InterCodes* label1, InterCodes* label2)
{
	TreeNode* exp1 = root->child;
	TreeNode* op = exp1->sibling;
	if(op->type == AST_RELOP)
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = TEMP;
		temp_index++;
		t2->u.temp_no = temp_index;

		InterCodes* code1 = translate_Exp(exp1, t1);
		InterCodes* code2 = translate_Exp(op->sibling, t2);

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IIFSTMT;
		strcpy(code3->code.u.ifstmt.relop, op->val.c);
		code3->code.u.ifstmt.left = t1;
		code3->code.u.ifstmt.right = t2;
		code3->code.u.ifstmt.label = label1->code.u.label;

		InterCodes* code4 = mallocInterCodes();
		code4->code.kind = IGOTO;
		code4->code.u.go_to = label2->code.u.label;

		return mergeInterCodes(mergeInterCodes(mergeInterCodes(code1, code2), code3), code4);
	}
	else if(exp1->type == AST_NOT)
	{
		return translate_Cond(exp1->sibling, label2, label1);
	}
	else if(op->type == AST_AND)
	{
		InterCodes* temp_label = mallocInterCodes();
		temp_label->code.kind = ILABEL;
		label_index++;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label_index;
		temp_label->code.u.label = temp_ll;
		InterCodes* code1 = translate_Cond(exp1, temp_label, label2);
		InterCodes* code2 = translate_Cond(op->sibling, label1, label2);
		return mergeInterCodes(mergeInterCodes(code1, temp_label), code2);
	}
	else if(op->type == AST_OR)
	{
		InterCodes* temp_label = mallocInterCodes();
		temp_label->code.kind = ILABEL;
		label_index++;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label_index;
		temp_label->code.u.label = temp_ll;
		InterCodes* code1 = translate_Cond(exp1, label1, temp_label);
		InterCodes* code2 = translate_Cond(op->sibling, label1, label2);
		return mergeInterCodes(mergeInterCodes(code1, temp_label), code2);
	}
	else
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = CONSTANT;
		t2->u.value = 0;

		InterCodes* code1 = translate_Exp(root, t1);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = IIFSTMT;
		code2->code.u.ifstmt.relop[0] = '!';
		code2->code.u.ifstmt.relop[1] = '=';
		code2->code.u.ifstmt.relop[2] = '\0';
		code2->code.u.ifstmt.left = t1;
		code2->code.u.ifstmt.right = t2;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label1->code.u.label->u.label_no;;
		code2->code.u.ifstmt.label = temp_ll;

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IGOTO;
		code3->code.u.go_to = label2->code.u.label;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
 
	}
}










