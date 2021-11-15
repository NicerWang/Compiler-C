%{
#define YYDEBUG 1
#include "main.h"

extern int yylex(void); 

vector<Node*> symbolTable;
vector<Node*> availNodes;
vector<Node*> funcNodes;
int availNodesCnt = 0;

extern int yyparse(void); 

string lexRes = "";

int yywrap()
{
	return 1;
}

void yyerror(const char *s)
{
	printf("[error] %s\n", s);
}

int main()
{
	yyparse();
	return 0;
}

Node* root;
void showNode(Node* node, int depth);
void showTree(Node* node);
void dfs(Node* node, int depth);
void addChildren(Node* node,vector<Node*>* nodes);
Node* formNode(NodeType type,SubType subType = NONE_t);
%}


%token INT WHILE FOR IF ELSE MAIN
%token LT LE GT GE EQ NE
%token OP_ASSIGN RETURN
%token SEMI COMMA LBS RBS LP RP LBT RBT
%token OP_ADD OP_SUB OP_MUL OP_DIV OP_MOD 
%token USUB NOT AND OR BIT_AND 
%token <node> ID NUMBER STR


%right OP_ASSIGN

%left OR
%left AND  

%left EQ NE
%left GT LT GE LE
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV OP_MOD
%left OP_MM OP_PP

%right USUB
%right NOT

%nonassoc UMINUS

%type <nodes> translation_unit stmts declare_list call_list idlist 
%type <node> entry_point func_declare while for if factor expr nullable_expr assign num id string type declare call stmt compound_stmt left_val
%union{
	Node* node;
	vector<Node*>* nodes;
};
%error-verbose
%%
root: translation_unit 
{
	root = formNode(ROOT_t);
	addChildren(root,$1);
	showTree(root);
}
;

translation_unit: entry_point 
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| func_declare translation_unit 
{
	$$ = $2;
	$2->push_back($1);
}
;

entry_point: INT MAIN LBS RBS compound_stmt
{
	$$ = formNode(MAIN_t);
	$$->children.push_back($5);
}
;

func_declare: type id LBS declare_list RBS compound_stmt
{
	$$ = formNode(FUNC_t,$1->subType);
	$$->children.push_back($2);

	Node* list_node = formNode(PARAM_t,TOTAL_t);
	addChildren(list_node,$4);
	$$->children.push_back(list_node);
	$$->children.push_back($6);
}
;

declare_list: type id COMMA declare_list
{
	$$ = $4;
	Node* newNode = formNode(PARAM_t,SINGLE_t);
	newNode->intValue = $1->subType;
	newNode->strValue = $2->strValue;
	$$->push_back(newNode);
}
| type id
{
	Node* newNode = formNode(PARAM_t,SINGLE_t);
	newNode->intValue = $1->subType;
	newNode->strValue = $2->strValue;
	$$ = new vector<Node*>();
	$$->push_back(newNode);
}
| 
{
	$$ = new vector<Node*>();
}
;

compound_stmt: LP stmts RP  
{
	$$ = formNode(STMT_t,COMPOUND_t);
	addChildren($$,$2);
}
| LP RP 
{
	$$ = new Node();
	$$->type = STMT_t;
}
;

stmts: stmt 
{ 
	$$ = new vector<Node*>();
	$$->push_back($1);
} 
| stmt stmts
{
	$$ = $2;
	$$->push_back($1);
}
;

stmt: declare SEMI {$$ = $1;}
| assign SEMI {$$ = $1;}
| if {$$ = $1;}
| for {$$ = $1;}
| while {$$ = $1;}
| call SEMI {$$ = $1;}
| compound_stmt {$$ = $1;}
| expr SEMI {$$ = $1;}
| RETURN expr SEMI
{
	$$ = formNode(STMT_t,RET_t);
	$$->children.push_back($2);
}
;

call: id LBS call_list RBS
{
	$$ = formNode(STMT_t,CALL_t);
	$$->children.push_back($1);
	Node* newNode = formNode(ARGU_t,TOTAL_t);
	addChildren(newNode,$3);
	$$->children.push_back(newNode);
}
;

call_list: expr COMMA call_list
{
	$$ = $3;
	$3->push_back($1);
}
| string COMMA call_list 
{
	$$ = $3;
	$3->push_back($1);
}
| expr
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| string
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| 
{
	$$ = new vector<Node*>();
}
;


declare: type idlist
{
	$$ = formNode(STMT_t,DECLARE_t);
	$$->children.push_back($1);
	addChildren($$,$2);
}
;

type: INT OP_MUL
{
	$$ = formNode(TYPE_t,INT_STAR_t);
}
| INT
{
	$$ = formNode(TYPE_t,INT_t);
}
;

id: ID
{
	$$ = formNode(VAR_t,ID_t);
	$$->strValue = $1->strValue;
}
;

num: NUMBER
{
	$$ = formNode(VAR_t,NUMBER_t);	
	$$->intValue = $1->intValue;
}
;
string: STR
{
	$$ = formNode(VAR_t,STRING_t);
	$$->strValue = $1->strValue;
}
;

idlist:	id COMMA idlist
{
	$$ = $3;
	$3->push_back($1);
}
| assign COMMA idlist
{
	$$ = $3;
	$3->push_back($1);
}
| id 
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| assign 
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| 
;

assign: left_val OP_ASSIGN expr
{
	$$ = formNode(STMT_t,ASSIGN_t);
	$$->children.push_back($1);
	$$->children.push_back($3);
}
;

left_val: BIT_AND id
{
	Node* sym = formNode(OP_t,OP_ADDRESS_t);
	$$ = formNode(VAR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| OP_MUL id
{
	Node* sym = formNode(OP_t,OP_VALUE_t);
	$$ = formNode(VAR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| id
{
	$$ = $1;
}


nullable_expr:expr {$$ = $1;}
| {$$ = new Node();}
;

expr: expr OP_ADD expr
{
	Node* sym = formNode(OP_t,OP_ADD_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_SUB expr
{
	Node* sym = formNode(OP_t,OP_SUB_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_MUL expr
{
	Node* sym = formNode(OP_t,OP_MUL_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_DIV expr
{
	Node* sym = formNode(OP_t,OP_DIV_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_MOD expr
{
	Node* sym = formNode(OP_t,OP_MOD_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| OP_SUB expr %prec USUB
{
	Node* sym = formNode(OP_t,OP_SUB_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| expr EQ expr
{
	Node* sym = formNode(OP_t,EQ_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr GT expr
{
	Node* sym = formNode(OP_t,GT_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr LT expr
{
	Node* sym = formNode(OP_t,LT_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr GE expr
{
	Node* sym = formNode(OP_t,GE_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr LE expr
{
	Node* sym = formNode(OP_t,LE_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr NE expr
{
	Node* sym = formNode(OP_t,NE_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr AND expr
{
	Node* sym = formNode(OP_t,AND_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OR expr
{
	Node* sym = formNode(OP_t,OR_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| NOT expr
{
	Node* sym = formNode(OP_t,NOT_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| factor 
{
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
}
| assign
{
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
}
| declare
{
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
}
| id OP_PP
{
	Node* sym = formNode(OP_t,OP_PP_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
}
| OP_PP id
{
	Node* sym = formNode(OP_t,OP_PP_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| id OP_MM
{
	Node* sym = formNode(OP_t,OP_MM_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back($1);
	$$->children.push_back(sym);
}
| OP_MM id
{
	Node* sym = formNode(OP_t,OP_MM_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| BIT_AND id
{
	Node* sym = formNode(OP_t,OP_ADDRESS_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| OP_MUL id
{
	Node* sym = formNode(OP_t,OP_VALUE_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
;

factor: id {$$ = $1;}
| num {$$ = $1;}
| call {$$ = $1;}
| LBS expr RBS {$$ = $2;}
;

if: IF LBS expr RBS stmt
{
	$$ = formNode(STMT_t,IF_t);
	$$->children.push_back($3);
	$$->children.push_back($5);
}
| IF LBS expr RBS stmt ELSE stmt
{
	$$ = formNode(STMT_t,IF_t);
	$$->children.push_back($3);
	$$->children.push_back($5);
	$$->children.push_back($7);
}
;

for: FOR LBS nullable_expr SEMI nullable_expr SEMI nullable_expr RBS stmt
{
	$$ = formNode(STMT_t,FOR_t);
	$$->children.push_back($3);
	$$->children.push_back($5);
	$$->children.push_back($7);
	$$->children.push_back($9);
}
;

while: WHILE LBS expr RBS stmt
{
	$$ = formNode(STMT_t,WHILE_t);
	$$->children.push_back($3);
	$$->children.push_back($5);
}
;


%%
void showTable(){
	cout<<"====== SYMBOL TABLE ======"<<endl;
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i]->subType == INT_t) cout<<"INT ";
		if(symbolTable[i]->subType == INT_STAR_t) cout<<"INT*";
		cout<<symbolTable[i]->strValue<<"     "<<(long long)symbolTable[i]<<endl;
	}
	cout<<"====== FUNCTIONS ======"<<endl;
	for(int i = 0; i < funcNodes.size(); i++){
		cout<<funcNodes[i]->children[0]->strValue<<"     "<<(long long)funcNodes[i]<<endl;
	}

}

void showTree(Node* node) {

	cout<<"====== GRAMMER TREE ======"<<endl;
	dfs(root, 1);
	cout<<"====== LEX RESULT ======"<<endl;
	cout<<lexRes;
	showTable();
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
				if(node->children[i]->subType == ASSIGN_t)
					availNodes.push_back(node->children[i]->children[0]);	
				else
					availNodes.push_back(node->children[i]);	
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
				if(availNodes[i]->strValue == node->strValue){
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
		}
		funcNodes.push_back(node);
		while(lexRes.find(node->children[0]->strValue + " ~") != -1){
			lexRes = lexRes.replace(lexRes.find(node->children[0]->strValue + " ~") + node->children[0]->strValue.length() + 1, 1 , to_string((long long)node));
		}
		for(int i = 0; i < node->children[1]->children.size(); i++){
			availNodesCnt++;
			availNodes.push_back(node->children[1]->children[i]);	
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
Node* formNode(NodeType type,SubType subType){
	Node* newNode = new Node();
	newNode->type = type;
	newNode->subType = subType;
	return newNode;
}

void addChildren(Node* node,vector<Node*>* nodes){
	vector<Node*> arr = *nodes;
	for(int i = arr.size() - 1; i >= 0; i--){
		node->children.push_back(arr[i]);
	}
}