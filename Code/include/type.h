#ifndef __TYPE_H__
#define __TYPE_H__
typedef struct Type_ Type;
typedef struct FieldList_ FieldList;

struct Type_
{
enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
union
{
	// 基本类型
	int basic;
	// 数组类型信息包括元素类型与数组大小构成
	struct { Type* elem; int size; } array;
	// 结构体类型信息是一个链表
	FieldList* structure;
	} u;
};

struct FieldList_
{
	char* name; // 域的名字
	Type* type; // 域的类型
	FieldList* tail; // 下一个域
};

#endif
