#include "structure.h"

#define PRE_PLUS 1
#define POST_PLUS 2
#define PRE_SUB 3
#define POST_SUB 4

extern string lexRes;
extern int yylineno;
int throwLine = -1;
Node *root;

vector<Symbol *> symbolTable;
vector<Symbol *> availNodes;
vector<Symbol *> funcNodes;
vector<vector<string>> intermediate;
vector<vector<string>> asmCode;
int availNodesCnt = 0;
int tempValCnt = 0;

void typeError(Node *node, string op)
{
	cout << "[error] Type error in line " << node->lineno << endl;
	if (op == "&=" || op == "=")
		cout << "Different types between = operator." << endl;
	else
		cout << "Wrong operands type to use operator" + op + "." << endl;
	exit(-1);
}

vector<string> formLine(string op, string var1, string var2, string ret)
{
	vector<string> temp;
	temp.push_back(op);
	temp.push_back(var1);
	temp.push_back(var2);
	temp.push_back(ret);
	return temp;
}

void undefinedError(Node *node, string op)
{
	cout << "[error] Undefined error in line " << node->lineno << endl;
	cout << "Use " + op + " before declare." << endl;
	cout << "Ignore Line " << throwLine << "." << endl;
}

void notAssignedError(Node *node)
{
	cout << "[error] Not assigned error in line " << node->lineno << endl;
	cout << "Use " + node->strValue + " before assgin a value." << endl;
}

void typeCheck(Node *node, SymType type, string op)
{
	for (int i = 0; i < node->children.size(); i++)
	{
		if (node->children[i]->type != OP_t)
		{
			if (node->children[i]->symType != type)
				typeError(node, op);
		}
	}
}

Symbol *formSymbol(string id, SymType type)
{
	Symbol *sym = new Symbol();
	sym->type = type;
	sym->id = id;
	return sym;
}

Node *formNode(NodeType type, SubType subType = NONE_t)
{
	Node *newNode = new Node();
	newNode->type = type;
	newNode->subType = subType;
	newNode->lineno = yylineno;
	return newNode;
}

void addChildren(Node *node, vector<Node *> *nodes)
{
	vector<Node *> arr = *nodes;
	for (int i = arr.size() - 1; i >= 0; i--)
	{
		node->children.push_back(arr[i]);
	}
}

void preProcess(Node *node)
{
	if (node->type == STMT_t && node->subType == DECLARE_t)
	{
		SymType type;
		if (node->children[0]->subType == INT_t)
		{
			type = SYM_INT_t;
		}
		else if (node->children[0]->subType == INT_STAR_t)
		{
			type = SYM_INT_STAR_t;
		}
		for (int i = 1; i < node->children.size(); i++)
		{
			availNodesCnt++;
			Symbol *sym;
			if (node->children[i]->subType == ASSIGN_t)
			{
				sym = formSymbol(node->children[i]->children[0]->strValue, type);
				node->children[i]->children[0]->isAssigned = true;
			}
			else
			{
				sym = formSymbol(node->children[i]->strValue, type);
				node->children[i]->isAssigned = true;
			};
			availNodes.push_back(sym);
		}
	}
	if (node->type == FUNC_t)
	{
		string arguments = "{";
		Symbol *func = formSymbol(node->children[0]->strValue, SYM_FUNC_t);

		if (node->children[0]->subType == INT_t)
			func->sub.push_back(SYM_INT_t);
		else if (node->children[0]->subType == INT_STAR_t)
			func->sub.push_back(SYM_INT_STAR_t);

		while (lexRes.find(node->children[0]->strValue + " ~") != -1)
		{
			lexRes = lexRes.replace(lexRes.find(node->children[0]->strValue + " ~") + node->children[0]->strValue.length() + 1, 1, to_string((long long)node));
		}
		for (int i = 0; i < node->children[1]->children.size(); i++)
		{
			availNodesCnt++;
			SymType type;
			if (node->children[1]->children[i]->intValue == INT_t)
			{
				type = SYM_INT_t;
				func->sub.push_back(SYM_INT_t);
			}
			else if (node->children[1]->children[i]->intValue == INT_STAR_t)
			{
				type = SYM_INT_STAR_t;
				func->sub.push_back(SYM_INT_STAR_t);
			}
			Symbol *sym = formSymbol(node->children[1]->children[i]->strValue, type);
			sym->isAssigned = true;
			sym->assignCnt = 2;
			availNodes.push_back(sym);
			int k;
			for (k = availNodes.size() - 1; k >= 0; k--)
				if (availNodes[k]->id == node->children[1]->children[k]->strValue)
					break;
			arguments += "#" + to_string((long long)availNodes[k]) + ",";
			lexRes = lexRes.replace(lexRes.find(node->children[1]->children[i]->strValue + " ~") + node->children[1]->children[i]->strValue.length() + 1, 1, to_string((long long)availNodes[k]));
		}
		funcNodes.push_back(func);
		arguments += "}";
		intermediate.push_back(formLine("FUNC", node->children[0]->strValue, arguments, "_"));
	}
	if (node->type == VAR_t && node->subType == ID_t)
	{
		if (node->strValue == "printf" || node->strValue == "scanf")
			return;
		bool isFunc = false;
		bool isVar = false;
		for (int i = funcNodes.size() - 1; i >= 0; i--)
		{
			if (funcNodes[i]->id == node->strValue)
			{
				isFunc = true;
				break;
			}
		}
		for (int i = availNodesCnt - 1; i >= 0; i--)
		{
			if (availNodes[i]->id == node->strValue)
			{
				if (!availNodes[i]->isAssigned && !node->isAssigned)
				{
					notAssignedError(node);
					// exit(0);
				}
				lexRes = lexRes.replace(lexRes.find(node->strValue + " ~") + node->strValue.length() + 1, 1, to_string((long long)availNodes[i]));
				node->symType = availNodes[i]->type;
				isVar = true;
				break;
			}
		}
		if (!isFunc && !isVar)
		{
			throwLine = node->lineno;
			undefinedError(node, node->strValue);
		}
	}
}

void postProcess(Node *node)
{
	string op = "_";
	string var1 = "_";
	string var2 = "_";
	string ret = "_";
	int preOrPost = 0;

	if (node->type == EXPR_t)
	{
		SymType exprType = SYM_INT_t;
		for (int i = 0; i < node->children.size(); i++)
		{
			if (node->children[i]->type == EXPR_t)
			{
				if (var1 == "_")
					var1 = node->children[i]->strValue;
				else
					var2 = node->children[i]->strValue;
			}
			else if (node->children[i]->type == VAR_t)
			{
				if (node->children[i]->subType == NUMBER_t)
				{
					if (var1 == "_")
						var1 = to_string(node->children[i]->intValue);
					else
						var2 = to_string(node->children[i]->intValue);
				}
				else
				{
					int j;
					for (j = availNodes.size() - 1; j >= 0; j--)
						if (availNodes[j]->id == node->children[i]->strValue)
							break;
					if (var1 == "_")
						var1 = "#" + to_string((long long)availNodes[j]);
					else
						var2 = "#" + to_string((long long)availNodes[j]);
				}
			}
			else if (node->children[i]->type == OP_t)
			{
				switch (node->children[i]->subType)
				{
				case OP_ADD_t:
				{
					op = "+";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_PP_t:
				{
					op = "+";
					if (i == 0)
					{
						preOrPost = PRE_PLUS;
						var2 = "1";
					}
					else
					{
						preOrPost = POST_PLUS;
						var2 = "0";
					}
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_MM_t:
				{
					op = "-";
					if (i == 0)
					{
						preOrPost = PRE_SUB;
						var2 = "1";
					}
					else
					{
						preOrPost = POST_SUB;
						var2 = "0";
					}
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_SUB_t:
				{
					op = "-";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_MUL_t:
				{
					op = "*";
					if (i == 0)
						typeCheck(node, SYM_INT_STAR_t, op);
					else
						typeCheck(node, SYM_INT_t, op);
					break;
				}

				case OP_ADDRESS_t:
				{
					op = "&";
					exprType = SYM_INT_STAR_t;
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_DIV_t:
				{
					op = "/";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_MOD_t:
				{
					op = "%";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case AND_t:
				{
					op = "&&";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OR_t:
				{
					op = "||";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case NOT_t:
				{
					op = "!";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case LT_t:
				{
					op = "<";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case LE_t:
				{
					op = "<=";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case EQ_t:
				{
					op = "==";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case NE_t:
				{
					op = "!=";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case GT_t:
				{
					op = ">";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case GE_t:
				{
					op = ">=";
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				default:
					break;
				}
			}
		}
		node->symType = exprType;
		node->strValue = "temp" + to_string(tempValCnt++);
		ret = node->strValue;
	}
	else if (node->type == STMT_t && node->subType == ASSIGN_t)
	{
		string target;
		SymType targetType = SYM_INT_t;
		if (node->children[0]->subType == ID_t)
		{
			target = node->children[0]->strValue;
			op = "=";
			targetType = node->children[0]->symType;
		}
		else if (node->children[0]->subType == LEFT_VALUE_t)
		{
			target = node->children[0]->children[1]->strValue;
			op = "&=";
		}

		int i;
		for (i = availNodesCnt - 1; i >= 0; i--)
			if (availNodes[i]->id == target)
				break;

		var1 = "#" + to_string((long long)availNodes[i]);
		availNodes[i]->isAssigned = true;

		if (node->children[1]->symType != targetType)
		{
			typeError(node, "=");
		}

		if (node->children[1]->type == EXPR_t)
		{
			var2 = node->children[1]->strValue;
		}
		else if (node->children[1]->subType == CALL_t)
		{
			var2 = node->children[1]->strValue;
		}
		else if (node->children[1]->type == VAR_t)
		{
			if (node->children[1]->subType == NUMBER_t)
				var2 = to_string(node->children[1]->intValue);
			else
			{
				string anotherTarget = node->children[1]->strValue;
				int k;
				for (k = availNodesCnt - 1; k >= 0; k--)
					if (availNodes[k]->id == anotherTarget)
						break;
				var2 = "#" + to_string((long long)availNodes[k]);
			}
		}
		availNodes[i]->value = var2;
	}
	else if (node->type == STMT_t && node->subType == RET_t)
	{
		op = "RET";
		if (node->children[0]->type == EXPR_t)
		{
			var1 = node->children[0]->strValue;
		}
		else if (node->children[0]->type == VAR_t)
		{
			if (node->children[0]->subType == NUMBER_t)
				var1 = to_string(node->children[0]->intValue);
			else
			{
				string target = node->children[0]->strValue;
				int i;
				for (i = availNodesCnt - 1; i >= 0; i--)
				{
					if (availNodes[i]->id == target)
						break;
				}
				var1 = "#" + to_string((long long)availNodes[i]);
			}
		}
		node->symType = node->children[0]->symType;
	}
	else if (node->type == STMT_t && node->subType == CALL_t)
	{
		op = "CALL";
		ret = "temp" + to_string(tempValCnt++);
		node->strValue = ret;
		var1 = node->children[0]->strValue;
		var2 = "{";
		int idx;
		for (idx = 0; idx < funcNodes.size(); idx++)
		{
			if (var1 == funcNodes[idx]->id)
			{
				break;
			}
		}
		if (var1 != "printf" && var1 != "scanf")
		{
			if (funcNodes[idx]->sub.size() != node->children[1]->children.size())
			{
				cout << "[error]Wrong function call in line " << node->lineno << endl;
				cout << "Different number of arguments in defination and call of " + var1 << endl;
				exit(-1);
			}
			for (int i = 0; i < node->children[1]->children.size(); i++)
			{
				if (funcNodes[idx]->sub[i] != node->children[1]->children[i]->symType)
				{
					cout << "[error]Wrong function call in line " << node->lineno << endl;
					cout << "Different type of No." + to_string(i) + " argument in defination and call of " + var1 << endl;
					exit(-1);
				}
			}
		}

		for (int i = 0; i < node->children[1]->children.size(); i++)
		{
			if (node->children[1]->children[i]->type == EXPR_t || node->children[1]->children[i]->subType == CALL_t)
			{
				var2 += node->children[1]->children[i]->strValue + ",";
			}
			else if (node->children[1]->children[i]->type == VAR_t)
			{
				if (node->children[1]->children[i]->subType == NUMBER_t)
					var2 += to_string(node->children[1]->children[i]->intValue) + ",";
				else if (node->children[1]->children[i]->subType == ID_t)
				{
					string target = node->children[1]->children[i]->strValue;
					int k;
					for (k = availNodesCnt - 1; k >= 0; k--)
					{
						if (availNodes[k]->id == target)
							break;
					}
					var2 += "#" + to_string((long long)availNodes[k]) + ",";
				}
				else
				{
					var2 += node->children[1]->children[i]->strValue + ",";
				}
			}
		}
		var2 += "}";
	}
	else if (node->type == FUNC_t)
	{
		for (int i = 0; i < node->children[2]->children.size(); i++)
		{
			if (node->children[2]->children[i]->subType == RET_t)
			{
				SymType funcType;
				if (node->subType == INT_t)
					funcType = SYM_INT_t;
				else if (node->subType == INT_STAR_t)
					funcType = SYM_INT_STAR_t;
				if (node->children[2]->children[i]->symType != funcType)
				{
					cout << "[error]Wrong Return Statement in line " << node->children[2]->children[i]->lineno << endl;
					cout << "Return value of " + node->children[0]->strValue + " do not match" << endl;
					exit(-1);
				}
			}
		}
		op = "ENDF";
		var1 = node->children[0]->strValue;
	}
	if (preOrPost == PRE_PLUS)
	{
		intermediate.push_back(formLine("+", var1, "1", "temp" + to_string(tempValCnt)));
		intermediate.push_back(formLine("=", var1, "temp" + to_string(tempValCnt), "_"));
		tempValCnt++;
	}
	if (preOrPost == PRE_SUB)
	{
		intermediate.push_back(formLine("-", var1, "1", "temp" + to_string(tempValCnt)));
		intermediate.push_back(formLine("=", var1, "temp" + to_string(tempValCnt), "_"));
		tempValCnt++;
	}
	if (op != "_")
	{
		intermediate.push_back(formLine(op, var1, var2, ret));
	}
	if (preOrPost == POST_PLUS)
	{
		intermediate.push_back(formLine("+", var1, "1", "temp" + to_string(tempValCnt)));
		intermediate.push_back(formLine("=", var1, "temp" + to_string(tempValCnt), "_"));
		tempValCnt++;
	}
	if (preOrPost == POST_SUB)
	{
		intermediate.push_back(formLine("-", var1, "1", "temp" + to_string(tempValCnt)));
		intermediate.push_back(formLine("=", var1, "temp" + to_string(tempValCnt), "_"));
		tempValCnt++;
	}
}

void formIntermediateCode(Node *node)
{
	if (node->lineno == throwLine)
		return;
	int blockVarCnt = availNodesCnt;
	preProcess(node);
	if (node->subType == IF_t)
	{
		formIntermediateCode(node->children[0]);
		if (node->children[0]->strValue == "")
			return;
		intermediate.push_back(formLine("IFNZ", node->children[0]->strValue, "_", to_string(intermediate.size() + 3)));
		intermediate.push_back(formLine("JMP", "_", "_", "_"));
		int jmpPos = intermediate.size() - 1;
		formIntermediateCode(node->children[1]);
		if (node->children.size() > 2)
		{
			intermediate.push_back(formLine("JMP", "_", "_", "_"));
		}

		int elsePos = intermediate.size() - 1;
		intermediate[jmpPos][3] = to_string(intermediate.size() + 1);
		if (node->children.size() > 2)
		{
			formIntermediateCode(node->children[2]);
			intermediate[elsePos][3] = to_string(intermediate.size() + 1);
		}
	}
	else if (node->subType == WHILE_t)
	{
		int startLine = intermediate.size() + 1;
		formIntermediateCode(node->children[0]);
		if (node->children[0]->strValue == "")
			return;
		intermediate.push_back(formLine("IFNZ", node->children[0]->strValue, "_", to_string(intermediate.size() + 3)));
		intermediate.push_back(formLine("JMP", "_", "_", "_"));
		int jmpPos = intermediate.size() - 1;
		formIntermediateCode(node->children[1]);
		intermediate.push_back(formLine("JMP", "_", "_", to_string(startLine)));
		intermediate[jmpPos][3] = to_string(intermediate.size() + 1);
	}
	else if (node->subType == FOR_t)
	{
		formIntermediateCode(node->children[0]);
		int startLine = intermediate.size() + 1;
		formIntermediateCode(node->children[1]);
		if (node->children[1]->strValue == "")
			return;
		intermediate.push_back(formLine("IFNZ", node->children[1]->strValue, "_", to_string(intermediate.size() + 3)));
		intermediate.push_back(formLine("JMP", "_", "_", "_"));
		int jmpPos = intermediate.size() - 1;
		formIntermediateCode(node->children[3]);
		formIntermediateCode(node->children[2]);
		intermediate.push_back(formLine("JMP", "_", "_", to_string(startLine)));
		intermediate[jmpPos][3] = to_string(intermediate.size() + 1);
	}
	else
		for (int i = 0; i < node->children.size(); i++)
			formIntermediateCode(node->children[i]);
	if (node->lineno == throwLine)
		return;
	postProcess(node);
	if ((node->type == STMT_t && (node->subType == FOR_t || node->subType == COMPOUND_t)) || node->type == FUNC_t)
		while (availNodesCnt > blockVarCnt)
		{
			symbolTable.push_back(availNodes[availNodesCnt - 1]);
			availNodes.pop_back();
			availNodesCnt--;
		}
}

void showGrammerTreeNode(Node *node, int depth)
{
	for (int i = 0; i < depth; i++)
		cout << "-";
	cout << ">";
	switch (node->type)
	{
	case ROOT_t:
	{
		cout << "ROOT";
		break;
	}
	case STMT_t:
	{
		cout << "Statement";
		switch (node->subType)
		{
		case IF_t:
		{
			cout << ">IF";
			break;
		}
		case FOR_t:
		{
			cout << ">FOR";
			break;
		}
		case WHILE_t:
		{
			cout << ">WHILE";
			break;
		}
		case ASSIGN_t:
		{
			cout << ">ASSIGN";
			break;
		}
		case DECLARE_t:
		{
			cout << ">DECLARE";
			break;
		}
		case CALL_t:
		{
			cout << ">CALL";
			break;
		}
		case RET_t:
		{
			cout << ">RET";
			break;
		}
		case COMPOUND_t:
		{
			cout << ">COMPOUND";
			break;
		}
		}
		break;
	}
	case VAR_t:
	{
		switch (node->subType)
		{
		case NUMBER_t:
		{
			cout << "Number:" << node->intValue;
			break;
		}
		case ID_t:
		{
			cout << "ID:" + node->strValue;
			break;
		}
		case STRING_t:
		{
			cout << "String:" + node->strValue;
			break;
		}
		case LEFT_VALUE_t:
		{
			cout << "Left Value:";
		}
		}
		break;
	}
	case OP_t:
	{
		cout << "Operator";
		switch (node->subType)
		{
		case OP_ADDRESS_t:
		{
			cout << "> & ";
			break;
		}
		case OP_MM_t:
		{
			cout << "> -- ";
			break;
		}
		case OP_PP_t:
		{
			cout << "> ++ ";
			break;
		}
		case OP_ADD_t:
		{
			cout << "> + ";
			break;
		}
		case OP_SUB_t:
		{
			cout << "> - ";
			break;
		}
		case OR_t:
		{
			cout << "> || ";
			break;
		}
		case AND_t:
		{
			cout << "> && ";
			break;
		}
		case NOT_t:
		{
			cout << "> ! ";
			break;
		}
		case OP_MUL_t:
		{
			cout << "> * ";
			break;
		}
		case OP_DIV_t:
		{
			cout << "> / ";
			break;
		}
		case OP_MOD_t:
		{
			cout << "> % ";
			break;
		}
		case LT_t:
		{
			cout << "> < ";
			break;
		}
		case LE_t:
		{
			cout << "> <= ";
			break;
		}
		case GT_t:
		{
			cout << "> > ";
			break;
		}
		case GE_t:
		{
			cout << "> >= ";
			break;
		}
		case EQ_t:
		{
			cout << "> == ";
			break;
		}
		case NE_t:
		{
			cout << "> != ";
			break;
		}
		}
		break;
	}
	case FUNC_t:
	{
		cout << "Function";
		switch (node->subType)
		{
		case INT_t:
		{
			cout << "|Int";
			break;
		}
		case INT_STAR_t:
		{
			cout << "|Int*";
			break;
		}
		}
		break;
	}
	case MAIN_t:
	{
		cout << "EntryPoint";
		break;
	}
	case TYPE_t:
	{
		switch (node->subType)
		{
		case INT_t:
		{
			cout << "Type: Int";
			break;
		}
		case INT_STAR_t:
		{
			cout << "Type: Int Pointer:" + node->strValue;
			break;
		}
		}
		break;
	}
	case ARGU_t:
	{
		switch (node->subType)
		{
		case SINGLE_t:
		{
			cout << node->strValue;
			break;
		}
		case TOTAL_t:
		{
			cout << "Arguments: ";
			break;
		}
		}
		break;
	}
	case PARAM_t:
	{
		switch (node->subType)
		{
		case SINGLE_t:
		{
			cout << node->strValue << "|";
			switch (node->intValue)
			{
			case INT_t:
			{
				cout << "INT";
				break;
			}
			case INT_STAR_t:
			{
				cout << "INT*";
				break;
			}
			}
			break;
		}
		case TOTAL_t:
		{
			cout << "Parameters: ";
			break;
		}
		}
		break;
	}
	case EXPR_t:
	{
		cout << "Expression";
		break;
	}
	default:
	{
	}
	}
	cout << "(" << node->lineno << ")" << endl;
	return;
}

void grammerTreeDfs(Node *node, int depth)
{
	showGrammerTreeNode(node, depth);
	for (int i = 0; i < node->children.size(); i++)
	{
		grammerTreeDfs(node->children[i], depth + 1);
	}
}

void split(const string &s, vector<string> &tokens, const string &delimiters = " ")
{
	string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos)
	{
		tokens.push_back(s.substr(lastPos, pos - lastPos));
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}

void IntermediateOptimize()
{
	//rule1
	vector<int> tempUsage(tempValCnt, 0);
	for (int i = 0; i < intermediate.size(); i++)
	{
		if (intermediate[i][0] == "CALL")
		{
			tempUsage[stoi(intermediate[i][3].substr(4, intermediate[i][3].length()))]++;
		}
		if (intermediate[i][1].find("temp") != string::npos)
		{
			tempUsage[stoi(intermediate[i][1].substr(4, intermediate[i][1].length()))]++;
		}
		if (intermediate[i][2][0] == '{')
		{
			string temp = intermediate[i][2].substr(1, intermediate[i][2].length() - 2);
			vector<string> argus;
			split(temp, argus, ",");
			for (int k = 0; k < argus.size(); k++)
			{
				if (argus[k].find("temp") != string::npos)
				{
					tempUsage[stoi(argus[k].substr(4, argus[k].length()))]++;
				}
			}
		}
		else
		{
			if (intermediate[i][2].find("temp") != string::npos)
			{
				tempUsage[stoi(intermediate[i][2].substr(4, intermediate[i][2].length()))]++;
			}
		}
	}
	for (int i = 0; i < intermediate.size(); i++)
	{
		if (intermediate[i][3].find("temp") != string::npos)
		{
			if (tempUsage[stoi(intermediate[i][3].substr(4, intermediate[i][3].length()))] == 0)
			{
				intermediate[i][0] = "_";
			}
		}
	}
	// for (int i = 0; i < tempUsage.size(); i++)
	// {
	// 	cout << i << ":" << tempUsage[i] << endl;
	// }
	//rule2
	for (int i = 0; i < intermediate.size(); i++)
	{
		if ((intermediate[i][0] == "=" || intermediate[i][0] == "&=") && intermediate[i][1][0] == '#')
		{
			Symbol *ptr = (Symbol *)stoll(intermediate[i][1].substr(1, intermediate[i][1].length()));
			ptr->assignCnt++;
			if (intermediate[i][2][0] == '#' || intermediate[i][2][0] == 't')
			{
				ptr->assignCnt = 2;
			}
		}
		if (intermediate[i][0] == "&")
		{
			Symbol *ptr = (Symbol *)stoll(intermediate[i][1].substr(1, intermediate[i][1].length()));
			ptr->assignCnt = 2;
		}
	}
	vector<pair<string, string> *> toBeRemove;
	for (int i = 0; i < symbolTable.size(); i++)
	{
		if (symbolTable[i]->assignCnt <= 1)
		{
			toBeRemove.push_back(new pair<string, string>("#" + to_string((long long)symbolTable[i]), symbolTable[i]->value));
		}
	}
	for (int i = 0; i < toBeRemove.size(); i++)
	{
		for (int j = 0; j < intermediate.size(); j++)
		{
			for (int k = 1; k < 4; k++)
			{
				if (intermediate[j][k] == toBeRemove[i]->first)
				{
					intermediate[j][k] = toBeRemove[i]->second;
					if (intermediate[j][0] == "=" && k == 1)
					{
						intermediate[j][0] = "_";
					}
				}
				if (k == 2 && intermediate[j][k][0] == '{')
				{
					if (intermediate[j][k].find(toBeRemove[i]->first) != -1)
						intermediate[j][k] = intermediate[j][k].replace(intermediate[j][k].find(toBeRemove[i]->first), toBeRemove[i]->first.size(), toBeRemove[i]->second);
				}
			}
		}
	}
}

string findVarOrArg(string aimVar, map<string, int> &argumentMap, map<string, int> &variableMap, int &stackDeep)
{
	string retStr;
	map<string, int>::iterator it;
	it = argumentMap.find(aimVar);
	if (it == argumentMap.end())
	{
		it = variableMap.find(aimVar);
		if (it == variableMap.end())
		{
			return "";
		}
		else
		{
			stringstream ss;
			ss << (stackDeep - it->second);
			ss >> retStr;
			retStr = "[esp+" + retStr + "]";
		}
	}
	else
	{
		stringstream ss;
		ss << it->second;
		ss >> retStr;
		retStr = "[ebp+" + retStr + "]";
	}
	return retStr;
}

void formAsmCode()
{
	map<string, int> anchorMap;	  //key跳转标签名
	map<string, int> variableMap; //key 变量(局部、temp)标记 value 与abp的距离
	map<string, int> argumentMap; //key 实参标记 value 与abp的距离
	int JMPAnchorCount = 0;		  //跳转标签计数
	int stackDeep = 0;			  //栈深，即asp与abp的差值
	stringstream ss;
	vector<int> funcStackDeepList;
	int currentFuncIndex = 0;
	for (int i = 0; i < intermediate.size(); ++i)
	{
		if (intermediate[i][0] == "JMP" || intermediate[i][0] == "IFNZ")
		{
			anchorMap.insert(map<string, int>::value_type("Anchor" + intermediate[i][3], 0));
		}
		else if (intermediate[i][0] == "RET")
		{
			funcStackDeepList.push_back(stackDeep);
			stackDeep = 0;
		}
		else
		{
			if (intermediate[i][1][0] == 't' || intermediate[i][1][0] == '#')
			{
				if (variableMap.find(intermediate[i][1]) == variableMap.end())
				{
					stackDeep += 4;
					variableMap.insert(map<string, int>::value_type(intermediate[i][1], stackDeep));
				}
			}
			if (intermediate[i][2][0] == 't' || intermediate[i][2][0] == '#')
			{
				if (variableMap.find(intermediate[i][2]) == variableMap.end())
				{
					stackDeep += 4;
					variableMap.insert(map<string, int>::value_type(intermediate[i][2], stackDeep));
				}
			}
			if (intermediate[i][3][0] == 't' || intermediate[i][3][0] == '#')
			{
				if (variableMap.find(intermediate[i][3]) == variableMap.end())
				{
					stackDeep += 4;
					variableMap.insert(map<string, int>::value_type(intermediate[i][3], stackDeep));
				}
			}
		}
	}
	for (int i = 0; i < intermediate.size(); ++i)
	{

		string op = intermediate[i][0];
		string arg1 = intermediate[i][1];
		string arg2 = intermediate[i][2];
		string re = intermediate[i][3];

		string anchorStr = "Anchor" + to_string(i + 1);
		vector<string> tempcode;
		if (anchorMap.find(anchorStr) != anchorMap.end())
		{
			tempcode.push_back("." + anchorStr + ":\n");
		}

		if (op == "FUNC")
		{
			if (arg1 == "printf")
			{
			}
			else if (arg1 == "scanf")
			{
			}
			else
			{
				tempcode.push_back(arg1.append(":\n"));
				tempcode.push_back("\tpush ebp\n\tmov ebp, esp\n\tsub esp, " + to_string(funcStackDeepList[currentFuncIndex]) + "\n");
				//有参数时记录参数相对ebp位置
				if (arg2 != "{}")
				{
					//分离出参数
					queue<string> tempArgQueue;
					string tempArgStr = arg2;
					int pos = 1;
					while (pos != tempArgStr.size() - 1)
					{
						int nextPos = tempArgStr.find(',', pos);
						string tempArg = tempArgStr.substr(pos + 1, nextPos - pos - 1);
						tempArgQueue.push(tempArg);
						pos = nextPos + 1;
					}
					int stackSizeCount = 8; //返回指针占4字节 进入函数后push ebp占用4字节
					while (!tempArgQueue.empty())
					{
						argumentMap.insert(map<string, int>::value_type("#" + tempArgQueue.front(), stackSizeCount));
						tempArgQueue.pop();
						stackSizeCount += 4;
					}
				}
			}
		}
		else if (op == "ENDF")
		{
			currentFuncIndex++;
			argumentMap.clear();
			tempcode.push_back("\n");
			if (currentFuncIndex == funcStackDeepList.size() - 1)
			{
				tempcode.push_back("CMAIN:\n\tmov ebp, esp\n\tsub esp, " + to_string(funcStackDeepList[currentFuncIndex]) + "\n");
			}
		}
		else if (op == "RET")
		{
			//返回值为常量
			if (arg1[0] != '#' && arg1[0] != 't')
			{
				tempcode.push_back("\tmov eax, dword " + arg1 + "\n");
			}
			//返回值为变量
			else
			{
				string arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov eax, " + arg1Str + "\n");
				}
			}
			//清理栈帧 补充寄存器备份
			tempcode.push_back("\tadd esp, " + to_string(funcStackDeepList[currentFuncIndex]) + "\n");

			map<string, int>::iterator it;
			if (currentFuncIndex < funcStackDeepList.size() - 1)
				tempcode.push_back("\tpop ebp\n");
			tempcode.push_back("\tret\n");
		}
		else if (op == "CALL")
		{
			if (arg1 == "printf")
			{
				if (arg2 != "{}")
				{
					//分离出参数
					int argCount = 0;
					queue<string> tempArgQueue;
					string tempArgStr = arg2;
					int pos = 1;
					while (pos != tempArgStr.size() - 1)
					{
						int nextPos = tempArgStr.find(",", pos);
						string tempArg = tempArgStr.substr(pos, nextPos - pos);
						tempArgQueue.push(tempArg);
						argCount++;
						pos = nextPos + 1;
					}

					string oriStr = tempArgQueue.front();
					tempArgQueue.pop();
					if (!tempArgQueue.empty())
					{
						int pos = 1;
						while (true)
						{
							int nextPos = oriStr.find("%d", pos);
							if (nextPos > oriStr.size())
							{

								tempcode.push_back("\tPRINT_STRING `" + oriStr.substr(pos, oriStr.size() - 1 - pos) + "`\n");
								break;
							}
							else
							{
								tempcode.push_back("\tPRINT_STRING `" + oriStr.substr(pos, nextPos - pos) + "`\n");
								string decStr = findVarOrArg(tempArgQueue.front(), argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
								tempcode.push_back("\tPRINT_DEC 4," + decStr + "\n");
								tempArgQueue.pop();
							}
							pos = nextPos + 2;
						}
					}
					else
					{
						tempcode.push_back("\tPRINT_STRING `" + oriStr.substr(1, oriStr.size() - 2) + "`\n");
					}
				}
				else
				{
					tempcode.push_back("\tNEWLINE\n");
				}
			}
			else if (arg1 == "scanf")
			{
				if (arg2 == "{}")
				{
					cout << "[error] arg2 is empty in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
				}
				else
				{
					//分离出参数
					int argCount = 0;
					queue<string> tempArgQueue;
					string tempArgStr = arg2;
					int pos = 1;
					while (pos != tempArgStr.size() - 1)
					{
						int nextPos = tempArgStr.find(",", pos);
						string tempArg = tempArgStr.substr(pos, nextPos - pos);
						tempArgQueue.push(tempArg);
						argCount++;
						pos = nextPos + 1;
					}
					while (!tempArgQueue.empty())
					{
						string varStr = findVarOrArg(tempArgQueue.front(), argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
						if (varStr == "")
						{
							cout << tempArgQueue.front() << " not found in (" + op + "<" + arg1 + "<" + arg2 + "," + re + ")" << endl;
						}
						else
						{
							tempcode.push_back("\tGET_DEC 4, eax\n\tmov ebx, " + varStr + "\n\tmov [ebx], eax\n");
						}
						tempArgQueue.pop();
					}
				}
			}
			else
			{
				//处理传参
				int argSizeCount = 0;
				if (arg2 != "{}")
				{
					//分离出参数
					stack<string> tempArgStack;
					string tempArgStr = arg2;
					int pos = 1;
					while (pos != tempArgStr.size() - 1)
					{
						int nextPos = tempArgStr.find(',', pos);
						string tempArg = tempArgStr.substr(pos, nextPos - pos);
						tempArgStack.push(tempArg);
						argSizeCount += 4;
						pos = nextPos + 1;
					}
					//参数压栈
					while (!tempArgStack.empty())
					{
						string str = tempArgStack.top();
						//传入参数为变量
						if (str[0] == '#' || str[0] == 't')
						{
							string varStr = findVarOrArg(str, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
							if (varStr == "")
							{
								cout << "[error] arg:" + str + " not found in  " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
							}
							else
							{
								tempcode.push_back("\tpush dword " + varStr + "\n");
							}
						}
						//传入参数为常量
						else
						{
							tempcode.push_back("\tpush dword " + str + "\n");
						}
						tempArgStack.pop();
					}
				}
				tempcode.push_back("\tcall " + arg1 + "\n");
				if (argSizeCount != 0)
				{
					string tempStr;
					ss.str("");
					ss.clear();
					ss << argSizeCount;
					ss >> tempStr;
					tempcode.push_back("\tadd esp, " + tempStr + "\n");
				}
				string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (reStr == "")
				{
					cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov " + reStr + ", eax\n");
				}
			}
		}
		else if (op == "+" || op == "-" || op == "*")
		{
			//解引用
			if (op == "*" && arg2 == "_")
			{
				string varStr = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (varStr == "")
				{
					cout << "[error] " + arg1 + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")";
				}
				else
				{
					tempcode.push_back("\tmov eax, " + varStr + "\n");
					tempcode.push_back("\tmov eax, [eax]\n");
					string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
					if (reStr == "")
					{
						cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
					}
					else
					{
						tempcode.push_back("\tmov " + reStr + ", eax\n");
					}
				}
			}
			else
			{
				string opStr;
				if (op == "+")
					opStr = "add";
				else if (op == "-")
					opStr = "sub";
				else
					opStr = "imul";
				//arg1导入eax
				if (arg1[0] == '#' || arg1[0] == 't')
				{
					string varStr = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
					if (varStr == "")
					{
						cout << "[error] " + arg1 + "not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
					}
					else
					{
						tempcode.push_back("\tmov eax, " + varStr + "\n");
					}
				}
				else
				{
					tempcode.push_back("\tmov eax, dword " + arg1 + "\n");
				}
				//arg2导入ebx
				if (arg2[0] == '#' || arg2[0] == 't')
				{
					string varStr = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
					if (varStr == "")
					{
						cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
					}
					else
					{
						tempcode.push_back("\tmov ebx, " + varStr + "\n");
					}
				}
				else
				{
					tempcode.push_back("\tmov ebx, dword " + arg2 + "\n");
				}
				//运算
				tempcode.push_back("\t" + opStr + " eax, ebx\n");
				//处理re
				string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (reStr == "")
				{
					cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov " + reStr + ", eax\n");
				}
			}
		}
		else if (op == "/" || op == "%")
		{
			string opStr;
			if (op == "/")
				opStr = "eax";
			else
				opStr = "edx";

			//arg1导入eax
			if (arg1[0] == '#' || arg1[0] == 't')
			{
				string varStr = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (varStr == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov eax, " + varStr + "\n");
				}
			}
			else
			{
				tempcode.push_back("\tmov eax, dword " + arg1 + "\n");
			}

			//arg2导入ebx
			if (arg2[0] == '#' || arg2[0] == 't')
			{
				string varStr = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (varStr == "")
				{
					cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov ebx, " + varStr + "\n");
				}
			}
			else
			{
				tempcode.push_back("\tmov ebx, dword " + arg2 + "\n");
			}

			//运算
			tempcode.push_back("\tcwd\n\tdiv ebx\n");

			//处理re
			string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			if (reStr == "")
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			else
			{
				tempcode.push_back("\tmov " + reStr + ", " + opStr + "\n");
			}
		}
		else if (op == "&")
		{
			string varStr = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			if (varStr == "")
			{
				cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
			}
			else
			{
				tempcode.push_back("\tlea eax, " + varStr + "\n");
				string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (reStr == "")
				{
					cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
				}
				else
				{

					tempcode.push_back("\tmov " + reStr + ", eax\n");
				}
			}
		}
		else if (op == "=")
		{
			string arg2Str;
			if (arg2[0] == '#' || arg2[0] == 't')
			{
				arg2Str = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg2Str == "")
				{
					cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
				else
				{
					tempcode.push_back("\tmov eax, " + arg2Str + "\n");
					arg2Str = "eax";
				}
			}
			else
			{
				arg2Str = "dword " + arg2;
			}
			string arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			if (arg1Str == "")
			{
				cout << "[error] " + arg1 + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			tempcode.push_back("\tmov " + arg1Str + ", " + arg2Str + "\n");
		}
		else if (op == "_")
		{
		}
		else if (op == "&=")
		{
			string arg2Str = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			if (arg2Str == "")
			{
				cout << "[error] " + arg2 + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")";
			}
			else
			{
				string arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")";
				}
				else
				{
					tempcode.push_back("\tmov eax, " + arg1Str + "\n");
					tempcode.push_back("\tmov ebx, " + arg2Str + "\n");
					tempcode.push_back("\tmov [eax], ebx\n");
				}
			}
		}
		else if (op == "||")
		{
			string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			string arg1Str, arg2Str;
			if (reStr == "")
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			if (arg1[0] == 't' || arg1[0] == '#')
			{
				arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg1Str = "dword " + arg1;
			}
			if (arg2[0] == 't' || arg2[0] == '#')
			{
				arg2Str = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg2Str = "dword " + arg2;
			}

			tempcode.push_back("\tcmp " + arg1Str + ", dword 0\n\tjnz .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			tempcode.push_back("\tcmp " + arg2Str + ", dword 0\n\tjnz .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			tempcode.push_back("\tmov " + reStr + ", dword 0\n\tjmp .tempAnchor" + to_string(JMPAnchorCount + 1) + "\n");
			tempcode.push_back(".tempAnchor" + to_string(JMPAnchorCount) + ":\n\tmov " + reStr + ", dword 1\n.tempAnchor" + to_string(JMPAnchorCount + 1) + ":\n");
			JMPAnchorCount += 2;
		}
		else if (op == "&&")
		{
			string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			string arg1Str, arg2Str;
			if (reStr == "")
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			if (arg1[0] == 't' || arg1[0] == '#')
			{
				arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg1Str = "dword " + arg1;
			}
			if (arg2[0] == 't' || arg2[0] == '#')
			{
				arg2Str = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg2Str = "dword " + arg2;
			}

			tempcode.push_back("\tcmp " + arg1Str + ", dword 0\n\tjnz .tempAnchor" + to_string(JMPAnchorCount) + "\n\tjmp .tempAnchor" + to_string(JMPAnchorCount + 1) + "\n.tempAnchor" + to_string(JMPAnchorCount) + ":\n");
			tempcode.push_back("\tcmp " + arg2Str + ", dword 0\n\tjz .tempAnchor" + to_string(JMPAnchorCount + 1) + "\n\tmov " + reStr + ", dword 1\n\tjmp .tempAnchor" + to_string(JMPAnchorCount + 2) + "\n");
			tempcode.push_back(".tempAnchor" + to_string(JMPAnchorCount + 1) + ":\n\tmov " + reStr + ", dword 0\n.tempAnchor" + to_string(JMPAnchorCount + 2) + ":\n");
			JMPAnchorCount += 3;
		}
		else if (op == "<" || op == ">" || op == ">=" || op == "<=" || op == "==" || op == "!=")
		{
			string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			string arg1Str, arg2Str;
			if (reStr == "")
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			if (arg1[0] == 't' || arg1[0] == '#')
			{
				arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg1Str = "dword " + arg1;
			}
			if (arg2[0] == 't' || arg2[0] == '#')
			{
				arg2Str = findVarOrArg(arg2, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg2 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg2Str = "dword " + arg2;
			}

			tempcode.push_back("\tmov eax, " + arg1Str + "\n\tcmp eax, " + arg2Str + "\n");
			if (op == ">")
			{
				tempcode.push_back("\tja .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			else if (op == ">=")
			{
				tempcode.push_back("\tjnb .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			else if (op == "<")
			{
				tempcode.push_back("\tjb .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			else if (op == "<=")
			{
				tempcode.push_back("\tjna .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			else if (op == "==")
			{
				tempcode.push_back("\tje .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			else if (op == "!=")
			{
				tempcode.push_back("\tjne .tempAnchor" + to_string(JMPAnchorCount) + "\n");
			}
			tempcode.push_back("\tmov " + reStr + ", dword 0\n\tjmp .tempAnchor" + to_string(JMPAnchorCount + 1) + "\n");
			tempcode.push_back(".tempAnchor" + to_string(JMPAnchorCount) + ":\n");
			tempcode.push_back("\tmov " + reStr + ", dword 1\n.tempAnchor" + to_string(JMPAnchorCount + 1) + ":\n");
			JMPAnchorCount += 2;
		}
		else if (op == "!")
		{
			string reStr = findVarOrArg(re, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
			string arg1Str;
			if (reStr == "")
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			if (arg1[0] == 't' || arg1[0] == '#')
			{
				arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in " << i << " -> (" << op << "," << arg1 << "," << arg2 << "," << re << ")" << endl;
				}
			}
			else
			{
				arg1Str = "dword " + arg1;
			}

			tempcode.push_back("\tcmp " + arg1Str + ", dword 0\n");
			tempcode.push_back("\tjz .tempAnchor" + to_string(JMPAnchorCount) + "\n\tmov " + reStr + ", dword 0\n\tjmp .tempAnchor" + to_string(JMPAnchorCount + 1) + "\n");
			tempcode.push_back(".tempAnchor" + to_string(JMPAnchorCount) + ":\n\tmov " + reStr + ", dword 1\n.tempAnchor" + to_string(JMPAnchorCount + 1) + ":\n");
			JMPAnchorCount += 2;
		}
		else if (op == "JMP")
		{
			int renum = stoi(re);
			if (anchorMap.find("Anchor" + re) == anchorMap.end())
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			tempcode.push_back("\tjmp .Anchor" + re + "\n");
		}
		else if (op == "IFNZ")
		{
			string arg1Str;
			if (arg1[0] == 't' || arg1[0] == '#')
			{
				arg1Str = findVarOrArg(arg1, argumentMap, variableMap, funcStackDeepList[currentFuncIndex]);
				if (arg1Str == "")
				{
					cout << "[error] " + arg1 + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
				}
			}
			else
			{
				arg1Str = "dword " + arg1;
			}
			if (anchorMap.find("Anchor" + re) == anchorMap.end())
			{
				cout << "[error] " + re + " not found in (" + op + "," + arg1 + "," + arg2 + "," + re + ")" << endl;
			}
			tempcode.push_back("\tcmp " + arg1Str + ", dword 0\n");
			tempcode.push_back("\tjnz .Anchor" + re + "\n");
		}
		else
		{
			tempcode.push_back("(" + op + "," + arg1 + "," + arg2 + "," + re + ")\n");
		}
		asmCode.push_back(tempcode);
	}
	if (!asmCode.empty())
	{
		string beginStr = "%include \"io.inc\"\n\nsection .text\n\tglobal CMAIN\n\n";
		if (funcStackDeepList.size() == 1)
		{
			beginStr = "%include \"io.inc\"\n\nsection .text\n\tglobal CMAIN\nCMAIN:\n\tmov ebp, esp\n\tsub esp, " + to_string(funcStackDeepList[0]) + "\n";
		}
		asmCode[0].insert(asmCode[0].begin(), beginStr);
	}
}

void showTree(Node *node)
{
	formIntermediateCode(root);
	cout << "====== GRAMMER TREE ======" << endl;
	grammerTreeDfs(root, 1);
	cout << "====== LEX RESULT ======" << endl;
	cout << lexRes;
	if (!symbolTable.empty())
		cout << "====== SYMBOL TABLE ======" << endl;
	for (int i = 0; i < symbolTable.size(); i++)
	{
		if (symbolTable[i]->type == SYM_INT_STAR_t)
			cout << "INT* ";
		else if (symbolTable[i]->type == SYM_INT_t)
			cout << "INT ";
		cout << symbolTable[i]->id << "     " << (long long)symbolTable[i] << endl;
	}
	if (!funcNodes.empty())
		cout << "====== FUNCTIONS ======" << endl;
	for (int i = 0; i < funcNodes.size(); i++)
		cout << funcNodes[i]->id << "     " << (long long)funcNodes[i] << endl;
	IntermediateOptimize();
	cout << "====== Optimized IntermediateCode ======" << endl;
	int line = 1;
	for (int i = 0; i < intermediate.size(); i++)
	{
		if (intermediate[i][0] != "_")
		{
			cout << line << " " << i + 1 << " (" + intermediate[i][0] + "," + intermediate[i][1] + "," + intermediate[i][2] + "," + intermediate[i][3] + ")" << endl;
			line++;
		}
	}
	formAsmCode();
	cout << "====== AsmCode readable ======" << endl;
	line = 1;
	for (int i = 0; i < asmCode.size(); ++i)
	{
		cout << "line:" << line << "====>\n";
		line++;
		for (int j = 0; j < asmCode[i].size(); ++j)
		{
			cout << asmCode[i][j];
		}
		cout << endl;
	}
	cout << "==== AsmCode ====" << endl;
	for (int i = 0; i < asmCode.size(); ++i)
	{
		for (int j = 0; j < asmCode[i].size(); ++j)
		{
			cout << asmCode[i][j];
		}
	}
}
