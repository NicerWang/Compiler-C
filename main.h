#include <iostream>
#include <string>
#include <vector>
using namespace std;
enum SubType{
	INT_t,
	NUMBER_t,
	ID_t,
	IF_t,
	FOR_t,
	WHILE_t,
	ASSIGN_t,
	DECLARE_t,
	CALL_t,	
	OUT_t,
	IN_t,
	OP_PP_t,
	OP_MM_t,
	OP_ASSIGN_t,
	OP_ADD_t,
	OP_SUB_t,
	OP_MUL_t,
	OP_DIV_t,
	OP_MOD_t,
	LT_t,
	LE_t,
	GT_t,
	GE_t,
	EQ_t,
	NE_t,
	AND_t,
	OR_t,
	NOT_t,
    SINGLE_t,
    TOTAL_t,
    NONE_t
};

enum NodeType
{
	STMT_t,
	VAR_t,
	OP_t,
	FUNC_t,
	MAIN_t,
	TYPE_t,
	ARGU_t,
	PARAM_t,
	ROOT_t,
    EXPR_t,
};

class Node {
public:
	NodeType type;
	SubType subType = NONE_t;
	vector<Node*> children;
	int intValue;
	string strValue;
	Node(){};
};





