#include "struct.h"

#define PRE_PLUS 1
#define POST_PLUS 2
#define PRE_SUB 3
#define POST_SUB 4

extern string lexRes;
extern int yylineno;
int offset = 0;
int throwLine = -1;
Node *root;

vector<Symbol *> symbolTable;
vector<Symbol *> availNodes;
vector<Symbol *> funcNodes;
vector<vector<string>> intermediate;
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
		string op = "_";
		string var1 = "_";
		string var2 = "_";
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
			}
			else
			{
				sym = formSymbol(node->children[i]->strValue, type);
			}
			availNodes.push_back(sym);
			op = "DEC";
			var1 = sym->id;
			var2 = to_string((long long)availNodes[availNodes.size() - 1]);
		}
		if (op != "")
		{
			intermediate.push_back(formLine(op, var1, var2, "_"));
		}
	}
	if (node->type == VAR_t && node->subType == ID_t)
	{
		if (node->strValue == "printf" || node->strValue == "scanf")
			return;
		for (int i = availNodesCnt - 1; i >= 0; i--)
		{
			if (availNodes[i]->id == node->strValue)
			{
				lexRes = lexRes.replace(lexRes.find(node->strValue + " ~") + node->strValue.length() + 1, 1, to_string((long long)availNodes[i]));
				node->symType = availNodes[i]->type;
				break;
			}
			if (i == 0)
			{
				undefinedError(node, node->strValue);
				throwLine = node->lineno;
				cout << throwLine << endl;
			}
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
			availNodes.push_back(formSymbol(node->children[1]->children[i]->strValue, type));
			int k;
			for (k = availNodes.size() - 1; k >= 0; k--)
				if (availNodes[k]->id == node->children[1]->children[k]->strValue)
					break;
			arguments += to_string((long long)availNodes[k]) + "(" + node->children[1]->children[i]->strValue + "),";
			lexRes = lexRes.replace(lexRes.find(node->children[1]->children[i]->strValue + " ~") + node->children[1]->children[i]->strValue.length() + 1, 1, to_string((long long)availNodes[k]));
		}
		funcNodes.push_back(func);
		arguments += "}";
		intermediate.push_back(formLine("FUNC", node->children[0]->strValue, arguments, "_"));
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
						var1 = to_string((long long)availNodes[j]) + "(" + availNodes[j]->id + ")";
					else
						var2 = to_string((long long)availNodes[j]) + "(" + availNodes[j]->id + ")";
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
					op = "++";
					if (i == 0)
						preOrPost = PRE_PLUS;
					else
						preOrPost = POST_PLUS;
					typeCheck(node, SYM_INT_t, op);
					break;
				}
				case OP_MM_t:
				{
					op = "--";
					if (i == 0)
						preOrPost = PRE_SUB;
					else
						preOrPost = POST_SUB;
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

		var1 = to_string((long long)availNodes[i]) + "(" + target + ")";

		if (node->children[1]->symType != targetType)
		{
			typeError(node, "=");
		}

		if (node->children[1]->type == EXPR_t)
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
				var2 = to_string((long long)availNodes[k]) + "(" + anotherTarget + ")";
			}
		}
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
				var1 = to_string((long long)availNodes[i]) + "(" + target + ")";
			}
		}
	}
	else if (node->type == STMT_t && node->subType == CALL_t)
	{
		op = "CALL";
		ret = "temp" + to_string(tempValCnt++);
		node->strValue = ret;
		var1 = node->children[0]->strValue;
		var2 = "{";
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
					var2 += to_string((long long)availNodes[k]) + "(" + target + "),";
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
		op = "ENDF";
		var1 = node->children[0]->strValue;
	}
	if (preOrPost == PRE_PLUS)
	{
		intermediate.push_back(formLine("+", var1, "1", var1));
	}
	if (preOrPost == PRE_SUB)
	{
		intermediate.push_back(formLine("-", var1, "1", var1));
	}
	if (op != "_" && preOrPost == 0)
	{
		intermediate.push_back(formLine(op, var1, var2, ret));
	}
	if (preOrPost != 0)
	{
		intermediate.push_back(formLine("=", ret, var1, "_"));
	}
	if (preOrPost == POST_PLUS)
	{
		intermediate.push_back(formLine("+", var1, "1", var1));
	}
	if (preOrPost == POST_SUB)
	{
		intermediate.push_back(formLine("-", var1, "1", var1));
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
		intermediate.push_back(formLine("IFNZ", node->children[0]->strValue, "_", to_string(intermediate.size() + 3)));
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
	cout << "(" << node->lineno << ")";
	for (int i = 0; i < depth; i++)
		cout << "-";
	cout << ">";
	switch (node->type)
	{
	case ROOT_t:
	{
		cout << "ROOT" << endl;
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
		cout << endl;
		break;
	}
	case VAR_t:
	{
		switch (node->subType)
		{
		case NUMBER_t:
		{
			cout << "Number:" << node->intValue << endl;
			break;
		}
		case ID_t:
		{
			cout << "ID:" + node->strValue << endl;
			break;
		}
		case STRING_t:
		{
			cout << "String:" + node->strValue << endl;
			break;
		}
		case LEFT_VALUE_t:
		{
			cout << "Left Value:" << endl;
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
			cout << "> & " << endl;
			break;
		}
		case OP_MM_t:
		{
			cout << "> -- " << endl;
			break;
		}
		case OP_PP_t:
		{
			cout << "> ++ " << endl;
			break;
		}
		case OP_ADD_t:
		{
			cout << "> + " << endl;
			break;
		}
		case OP_SUB_t:
		{
			cout << "> - " << endl;
			break;
		}
		case OR_t:
		{
			cout << "> || " << endl;
			break;
		}
		case AND_t:
		{
			cout << "> && " << endl;
			break;
		}
		case NOT_t:
		{
			cout << "> ! " << endl;
			break;
		}
		case OP_MUL_t:
		{
			cout << "> * " << endl;
			break;
		}
		case OP_DIV_t:
		{
			cout << "> / " << endl;
			break;
		}
		case OP_MOD_t:
		{
			cout << "> % " << endl;
			break;
		}
		case LT_t:
		{
			cout << "> < " << endl;
			break;
		}
		case LE_t:
		{
			cout << "> <= " << endl;
			break;
		}
		case GT_t:
		{
			cout << "> > " << endl;
			break;
		}
		case GE_t:
		{
			cout << "> >= " << endl;
			break;
		}
		case EQ_t:
		{
			cout << "> == " << endl;
			break;
		}
		case NE_t:
		{
			cout << "> != " << endl;
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
			cout << "|Int" << endl;
			break;
		}
		case INT_STAR_t:
		{
			cout << "|Int*" << endl;
			break;
		}
		}
		break;
	}
	case MAIN_t:
	{
		cout << "EntryPoint" << endl;
		break;
	}
	case TYPE_t:
	{
		switch (node->subType)
		{
		case INT_t:
		{
			cout << "Type: Int" << endl;
			break;
		}
		case INT_STAR_t:
		{
			cout << "Type: Int Pointer:" + node->strValue << endl;
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
			cout << node->strValue << endl;
			break;
		}
		case TOTAL_t:
		{
			cout << "Arguments: " << endl;
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
			}
			case INT_STAR_t:
			{
				cout << "INT*";
			}
			}
			cout << endl;
			break;
		}
		case TOTAL_t:
		{
			cout << "Parameters: " << endl;
			break;
		}
		}
		break;
	}
	case EXPR_t:
	{
		cout << "Expression" << endl;
		break;
	}
	default:
	{
		cout << endl;
	}
	}
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
	cout << "====== IntermediateCode ======" << endl;
	for (int i = 0; i < intermediate.size(); i++)
		cout << i + 1 << " (" + intermediate[i][0] + "," + intermediate[i][1] + "," + intermediate[i][2] + "," + intermediate[i][3] + ")" << endl;
}
