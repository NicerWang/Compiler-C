%{
#define YYDEBUG 1
#include "main.h"



extern int yylex(void); 

extern int yyparse(void); 

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
%}


%token INT WHILE FOR IF ELSE MAIN
%token IN OUT
%token LT LE GT GE EQ NE
%token OP_ASSIGN
%token SEMI COMMA LBS RBS LP RP LBT RBT
%token OP_ADD OP_SUB OP_MUL OP_DIV OP_MOD 
%token USUB NOT AND OR
%token <node> ID NUMBER


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
%type <node> entry_point func_declare while for if factor expr nullable_expr assign num id type declare output input call stmt compound_stmt
%union{
	Node* node;
	vector<Node*>* nodes;
};
%error-verbose
%%
root: translation_unit 
{
	root = new Node();
	root->type = ROOT_t;
	vector<Node*> arr = *$1;
	for(int i = arr.size() - 1; i >= 0; i--){
		root->children.push_back(arr[i]);
	}
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
	$$ = new Node();
	$$->type = MAIN_t;
	$$->children.push_back($5);
}
;

func_declare: type id LBS declare_list RBS compound_stmt
{
	$$ = new Node();
	$$->type = FUNC_t;
	$$->children.push_back($2);

	Node* list_node = new Node();
	list_node->type = PARAM_t;
	list_node->subType = TOTAL_t;
	vector<Node*> arr = *$4;
	for(int i = arr.size() - 1; i >= 0; i--){
		list_node->children.push_back(arr[i]);
	}
	$$->children.push_back(list_node);

	$$->children.push_back($6);
}
;
declare_list: type id COMMA declare_list
{
	$$ = $4;
	Node* newNode = new Node();
	newNode->subType = SINGLE_t;
	newNode->intValue = $1->subType;
	newNode->strValue = $2->strValue;
	$$->push_back(newNode);
}
| type id
{
	Node* newNode = new Node();
	newNode->subType = SINGLE_t;
	newNode->intValue = $1->subType;
	newNode->strValue = $2->strValue;
	$$ = new vector<Node*>();
	$$->push_back(newNode);
}
;

compound_stmt: LP stmts RP  
{
	$$ = new Node();
	$$->type = STMT_t;
	vector<Node*> arr = *$2;
	for(int i = arr.size() - 1; i >= 0; i--){
		$$->children.push_back(arr[i]);
	}
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
| if {$$ = $1;}
| for {$$ = $1;}
| while {$$ = $1;}
| call SEMI {$$ = $1;}
| compound_stmt {$$ = $1;}
| expr SEMI {$$ = $1;}
| input {$$ = $1;}
| output {$$ = $1;}
;

call: id LBS call_list RBS
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = CALL_t;
	$$->children.push_back($1);
	Node* newNode = new Node();
	vector<Node*> arr = *$3;
	for(int i = arr.size() - 1; i >=0; i--){
		newNode->children.push_back(arr[i]);
	}
	$$->children.push_back(newNode);
}
;

call_list: id COMMA call_list
{
	$$ = $3;
	$3->push_back($1);
}
| num COMMA call_list 
{
	$$ = $3;
	$3->push_back($1);
}
| id
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| num
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
;

input: IN id
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = IN_t;
	$$->children.push_back($2);
}
;
output: OUT id
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = OUT_t;
	$$->children.push_back($2);
}
;

declare: type idlist
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = DECLARE_t;
	vector<Node*> arr = *$2;
	for(int i = arr.size() - 1; i >= 0; i--){
		$$->children.push_back(arr[i]);
	}
}
;

type: INT
{
	$$ = new Node();
	$$->type = TYPE_t;
	$$->subType = INT_t;
}
;

id: ID
{
	$$ = new Node();
	$$->type = VAR_t;
	$$->subType = ID_t;
	$$->strValue = $1->strValue;
}
;

num: NUMBER
{
	$$ = new Node();
	$$->type = VAR_t;
	$$->subType = NUMBER_t;
	$$->intValue = $1->intValue;

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
;

assign: id OP_ASSIGN expr
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = ASSIGN_t;
	$$->children.push_back($1);
	$$->children.push_back($3);
}
;

nullable_expr:expr {$$ = $1;}
| {$$ = new Node();}
;

expr: expr OP_ADD expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_ADD_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_SUB expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_SUB_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_MUL expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_MUL_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_DIV expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_DIV_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OP_MOD expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_MOD_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| OP_SUB expr %prec USUB
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_SUB_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| expr EQ expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = EQ_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr GT expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = GT_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr LT expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = LT_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr GE expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = GE_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr LE expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = LE_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr NE expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = NE_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr AND expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = AND_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| expr OR expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OR_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
	$$->children.push_back($3);
}
| NOT expr
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = NOT_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
| factor 
{
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
}
| assign
{
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
}
| declare
{
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
}
| id OP_PP
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_PP_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
}
| OP_PP id
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_PP_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back(sym);
	$$->children.push_back($2);

}
| id OP_MM
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_MM_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back($1);
	$$->children.push_back(sym);
}
| OP_MM id
{
	Node* sym = new Node();
	sym->type = OP_t;
	sym->subType = OP_MM_t;
	$$ = new Node();
	$$->type = EXPR_t;
	$$->children.push_back(sym);
	$$->children.push_back($2);

}
;

factor: id {$$ = $1;}
| num {$$ = $1;}
| LBS expr RBS {$$ = $2;}
;

if: IF LBS expr RBS stmt
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = IF_t;
	$$->children.push_back($3);
	$$->children.push_back($5);
}
| IF LBS expr RBS stmt ELSE stmt
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = IF_t;
	$$->children.push_back($3);
	$$->children.push_back($5);
	$$->children.push_back($7);
}
;

for: FOR LBS nullable_expr SEMI nullable_expr SEMI nullable_expr RBS stmt
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = FOR_t;
	$$->children.push_back($3);
	$$->children.push_back($5);
	$$->children.push_back($7);
	$$->children.push_back($9);
}
;

while: WHILE LBS expr RBS stmt
{
	$$ = new Node();
	$$->type = STMT_t;
	$$->subType = WHILE_t;
	$$->children.push_back($3);
	$$->children.push_back($5);
}
;


%%


void showTree(Node* node) {
	dfs(root, 1);
}
void showNode(Node* node, int depth) {
	for (int i = 0; i < depth; i++) cout<<"-";
	cout << ">";
	switch (node->type) {
    case ROOT_t: {
        cout << "ROOT" << endl;
        break;
    }
	case STMT_t: {
		cout << "Statement";
		switch (node->subType) {
		case IF_t: {
			cout << ">IF";
			break;
		}
		case FOR_t: {
			cout << ">FOR";
			break;
		}
		case WHILE_t: {
			cout << ">WHILE";
			break;
		}
		case ASSIGN_t: {
			cout << ">ASSIGN";
			break;
		}
		case DECLARE_t: {
			cout << ">DECLARE";
			break;
		}
		case CALL_t: {
			cout << ">CALL" + node->strValue;
			break;
		}

		}
		cout<<endl;
		break;

	}
	case VAR_t: {
		switch (node->subType) {
		case NUMBER_t: {
			cout << "Number:" << node->intValue << endl;
			break;
		}
		case ID_t: {
			cout << "ID:" + node->strValue << endl;
			break;
		}
		}
		break;
	}
	case OP_t: {
		cout << "Operator";
		switch (node->subType) {
		case OP_MM_t: {
			cout << "> -- " << endl;
			break;
		}
		case OP_PP_t: {
			cout << "> ++ " << endl;
			break;
		}
		case OP_ADD_t: {
			cout << "> + " << endl;
			break;
		}
		case OP_SUB_t: {
			cout << "> - " << endl;
			break;
		}
		case OR_t: {
			cout << "> || " << endl;
			break;
		}
		case AND_t: {
			cout << "> && " << endl;
			break;
		}
		case NOT_t: {
			cout << "> ! " << endl;
			break;
		}
		case OP_MUL_t: {
			cout << "> * " << endl;
			break;
		}
		case OP_DIV_t: {
			cout << "> / " << endl;
			break;
		}
		case OP_MOD_t: {
			cout << "> % " << endl;
			break;
		}
		case LT_t: {
			cout << "> < " << endl;
			break;
		}
		case LE_t: {
			cout << "> <= " << endl;
			break;
		}
		case GT_t: {
			cout << "> > " << endl;
			break;
		}
		case GE_t: {
			cout << "> >= " << endl;
			break;
		}
		case EQ_t: {
			cout << "> == " << endl;
			break;
		}
		case NE_t: {
			cout << "> != " << endl;
			break;
		}
		}
		break;
	}
	case FUNC_t: {
		cout << "Function";
		switch (node->subType)
		{
		case INT_t: {
			cout << "RetType: Int" << endl;
			break;
		}
		}
		break;
	}
	case MAIN_t: {
		cout << "EntryPoint" << endl;
		break;
	}
	case TYPE_t: {
		switch (node->subType)
		{
		case INT_t: {
			cout << "Type: Int" << endl;
			break;
		}
		}
		break;
	}
	case ARGU_t: {
        switch (node->subType)
        {
            case SINGLE_t:{
                cout<<node->strValue<<"|"<<node->intValue<<endl;
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
                cout<<node->strValue<<endl;
                break;
            }
            case TOTAL_t:{
                cout << "Parameters: "<< endl;
                break;      
            }
        }
        break;
	}
	case EXPR_t: {
		cout << "Expression" << endl;
		break;
	}
	default :{
		cout<<endl;
	}
	}
	return;
}

void dfs(Node* node, int depth) {
	showNode(node, depth);
	for (int i = 0; i < node->children.size(); i++)
		dfs(node->children[i], depth + 1);
}

