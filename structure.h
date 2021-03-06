#ifndef STRUCTURE_H
#define STRUCTURE_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <sstream>
using namespace std;
enum SubType
{
	INT_t,
	INT_STAR_t,
	NUMBER_t,
	STRING_t,
	LEFT_VALUE_t,
	ID_t,
	IF_t,
	FOR_t,
	WHILE_t,
	ASSIGN_t,
	DECLARE_t,
	COMPOUND_t,
	CALL_t,
	RET_t,
	OP_ADDRESS_t,
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

enum SymType
{
	SYM_INT_t,
	SYM_INT_STAR_t,
	SYM_FUNC_t,
};

class Symbol
{
public:
	string id;
	SymType type;
	string value;
	vector<SymType> sub;
	bool isAssigned = false;
	int assignCnt = 0;
};

class Node
{
public:
	NodeType type;
	SubType subType;
	vector<Node *> children;
	long long intValue;
	string strValue;
	int lineno;
	SymType symType;
	bool isAssigned = false;
	Node()
	{
		this->subType = NONE_t;
	}
};
#endif