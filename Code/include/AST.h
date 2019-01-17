#ifndef AST_H
#define AST_H
typedef enum ASTNodeType ASTNodeType;
typedef struct TreeNode TreeNode;
enum ASTNodeType {
    AST_INT = 0,    AST_FLOAT,      AST_SEMI,
    AST_COMMA,      AST_ASSIGNOP,   AST_RELOP,
    AST_PLUS,       AST_MINUS,      AST_STAR,
    AST_DIV,        AST_AND,        AST_OR,
    AST_DOT,        AST_NOT,        AST_LP,
    AST_RP,         AST_LB,         AST_RB,
    AST_LC,         AST_RC,         AST_TYPE,
    AST_STRUCT,     AST_RETURN,     AST_IF,
    AST_ELSE,       AST_WHILE,      AST_ID,
    AST_Error,
    AST_Program,    AST_ExtDefList, AST_ExtDef,
    AST_ExtDecList, AST_Specifier,  AST_StructSpecifier,
    AST_OptTag,     AST_Tag,        AST_VarDec,
    AST_FunDec,     AST_VarList,    AST_ParamDec,
    AST_CompSt,     AST_StmtList,   AST_Stmt,
    AST_DefList,    AST_Def,        AST_DecList,
    AST_Dec,        AST_Exp,        AST_Args
};

struct TreeNode {
    enum ASTNodeType type;
    int lineno;
    struct TreeNode *child, *sibling, *parent;
    struct {
        int i;
		float d;
        char c[32];
    } val;
};

struct TreeNode* createNode(enum ASTNodeType type, int lineno);
int addNode(struct TreeNode *parent, int count, ...);
void printfTree(struct TreeNode *root, int layer);
extern TreeNode *ASTroot;

#endif
