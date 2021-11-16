#include "struct.h"

extern string lexRes;
Node* root;

vector<Symbol*> symbolTable;
vector<Symbol*> availNodes;
vector<Symbol*> funcNodes;
int availNodesCnt = 0;


Node* formNode(NodeType type,SubType subType = NONE_t){
	Node* newNode = new Node();
	newNode->type = type;
	newNode->subType = subType;
	return newNode;
}

Symbol* formSymbol(string id,SymType type){
	Symbol* sym = new Symbol();
	sym->type = type;
	sym->id = id;
	return sym;
}

void addChildren(Node* node,vector<Node*>* nodes){
	vector<Node*> arr = *nodes;
	for(int i = arr.size() - 1; i >= 0; i--){
		node->children.push_back(arr[i]);
	}
}

void showTable(){
	cout<<"====== SYMBOL TABLE ======"<<endl;
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i]->type == SYM_INT_t) cout<<"INT ";
		if(symbolTable[i]->type == SYM_INT_STAR_t) cout<<"INT*";
		cout<<symbolTable[i]->id<<"     "<<(long long)symbolTable[i]<<endl;
	}
	cout<<"====== FUNCTIONS ======"<<endl;
	for(int i = 0; i < funcNodes.size(); i++){
		cout<<funcNodes[i]->id<<"     "<<(long long)funcNodes[i]<<endl;
	}

}



void showNode(Node* node, int depth) {
	for (int i = 0; i < depth; i++) cout<<"-";
	cout << ">";
	switch (node->type) {
    case ROOT_t: {
        cout << "ROOT"  << endl;
        break;
    }
	case STMT_t: {
		cout << "Statement";
		switch (node->subType) {
		case IF_t: {
			cout << ">IF" ;
			break;
		}
		case FOR_t: {
			cout << ">FOR" ;
			break;
		}
		case WHILE_t: {
			cout << ">WHILE" ;
			break;
		}
		case ASSIGN_t: {
			cout << ">ASSIGN" ;
			break;
		}
		case DECLARE_t: {
			for(int i = 1; i < node->children.size(); i++){
				availNodesCnt++;
				SymType type;
				if(node->children[1]->children[i]->subType == INT_t){
					type = SYM_INT_t;
				}
				else if(node->children[1]->children[i]->subType == INT_STAR_t){
					type = SYM_INT_STAR_t;
				}
				if(node->children[i]->subType == ASSIGN_t)
					availNodes.push_back(formSymbol(node->children[i]->children[0]->strValue,type));	
				else
					availNodes.push_back(formSymbol(node->children[i]->strValue,type));	
			}

			cout << ">DECLARE" ;
			break;
		}
		case CALL_t: {
			cout << ">CALL" + node->strValue ;
			break;
		}
		case RET_t: {
			cout << ">RET" ;
			break;
		}
		case COMPOUND_t: {
			cout << ">COMPOUND" ;
			break;
		}
		}
		cout<<endl;
		break;

	}
	case VAR_t: {
		switch (node->subType) {
		case NUMBER_t: {
			cout << "Number:" << node->intValue  << endl;
			break;
		}
		case ID_t: {
			cout << "ID:" + node->strValue  << endl;
			for(int i = availNodesCnt - 1; i >= 0; i--){
				if(availNodes[i]->id == node->strValue){
					lexRes = lexRes.replace(lexRes.find(node->strValue + " ~") + node->strValue.length() + 1, 1 , to_string((long long)availNodes[i]));
					break;
				}
			}	
			break;
		}
		case STRING_t: {
			cout << "String:" + node->strValue  << endl;
			break;
		}
		}
		break;
	}
	case OP_t: {
		cout << "Operator";
		switch (node->subType) {
		case OP_ADDRESS_t: {
			cout << "> & "  << endl;
			break;
		}	
		case OP_VALUE_t: {
			cout << "> * "  << endl;
			break;
		}	
		case OP_MM_t: {
			cout << "> -- "  << endl;
			break;
		}
		case OP_PP_t: {
			cout << "> ++ "  << endl;
			break;
		}
		case OP_ADD_t: {
			cout << "> + "  << endl;
			break;
		}
		case OP_SUB_t: {
			cout << "> - "  << endl;
			break;
		}
		case OR_t: {
			cout << "> || "  << endl;
			break;
		}
		case AND_t: {
			cout << "> && "  << endl;
			break;
		}
		case NOT_t: {
			cout << "> ! "  << endl;
			break;
		}
		case OP_MUL_t: {
			cout << "> * "  << endl;
			break;
		}
		case OP_DIV_t: {
			cout << "> / "  << endl;
			break;
		}
		case OP_MOD_t: {
			cout << "> % "  << endl;
			break;
		}
		case LT_t: {
			cout << "> < "  << endl;
			break;
		}
		case LE_t: {
			cout << "> <= "  << endl;
			break;
		}
		case GT_t: {
			cout << "> > "  << endl;
			break;
		}
		case GE_t: {
			cout << "> >= "  << endl;
			break;
		}
		case EQ_t: {
			cout << "> == "  << endl;
			break;
		}
		case NE_t: {
			cout << "> != "  << endl;
			break;
		}
		}
		break;
	}
	case FUNC_t: {
		cout << "Function" ;
		switch (node->subType)
		{
		case INT_t: {
			cout << "|Int"  << endl;
			break;
		}
		case INT_STAR_t: {
			cout << "|Int*"  << endl;
			break;
		}
		}
		funcNodes.push_back(formSymbol(node->children[0]->strValue,SYM_FUNC_t));
		while(lexRes.find(node->children[0]->strValue + " ~") != -1){
			lexRes = lexRes.replace(lexRes.find(node->children[0]->strValue + " ~") + node->children[0]->strValue.length() + 1, 1 , to_string((long long)node));
		}
		for(int i = 0; i < node->children[1]->children.size(); i++){
			availNodesCnt++;
			SymType type;
			if(node->children[1]->children[i]->subType == INT_t){
				type = SYM_INT_t;
			}
			else if(node->children[1]->children[i]->subType == INT_STAR_t){
				type = SYM_INT_STAR_t;
			}

			availNodes.push_back(formSymbol(node->children[1]->children[i]->strValue,type));	
			lexRes = lexRes.replace(lexRes.find(node->children[1]->children[i]->strValue + " ~") + node->children[1]->children[i]->strValue.length() + 1, 1 , to_string((long long)node->children[1]->children[i]));
		}
		break;
	}
	case MAIN_t: {
		cout << "EntryPoint"  << endl;
		break;
	}
	case TYPE_t: {
		switch (node->subType)
		{
		case INT_t: {
			cout << "Type: Int"  << endl;
			break;
		}
		case INT_STAR_t: {
			cout << "Type: Int Pointer:" + node->strValue  << endl;
			break;
		}
		}
		break;
	}
	case ARGU_t: {
        switch (node->subType)
        {
            case SINGLE_t:{
				cout<<node->strValue <<endl;
                break;
            }
            case TOTAL_t:{
                cout << "Arguments: "<< endl;
                break;      
            }
        }
        break;
		
	}
	
    case PARAM_t: {
        switch (node->subType)
        {
            case SINGLE_t:{
				cout<<node->strValue<<"|";
				switch (node->intValue){
					case INT_t:{
						cout << "INT" ;
					}
				}
				cout<<endl;
                break;
            }
            case TOTAL_t:{
                cout << "Parameters: "  << endl;
                break;      
            }
        }
        break;
	}
	case EXPR_t: {
		cout << "Expression"  << endl;
		break;
	}
	default :{
		cout<<endl;
	}
	}
	return;
}

void dfs(Node* node, int depth) {
	int blockVarCntNow = availNodesCnt;
	showNode(node, depth);
	for (int i = 0; i < node->children.size(); i++){
		dfs(node->children[i], depth + 1);
	}
	if( (node->type == STMT_t && (node->subType == FOR_t || node->subType == COMPOUND_t)) || node->type == FUNC_t)
		while(availNodesCnt > blockVarCntNow){
			symbolTable.push_back(availNodes[availNodesCnt - 1]);
			availNodes.pop_back();
			availNodesCnt--;
	}
}


void showTree(Node* node) {

	cout<<"====== GRAMMER TREE ======"<<endl;
	dfs(root, 1);
	cout<<"====== LEX RESULT ======"<<endl;
	cout<<lexRes;
	showTable();
}



