%{
#define YYDEBUG 1
#include "structure.h"
#include "core.h"
string lexRes = "";

extern int yylex(void); 

extern int yyparse(void); 

extern int yylineno;

int yywrap()
{
	return 1;
}

void yyerror(const char *s)
{
	printf("[error] Info:%s\n[error] Line: %d\n", s, yylineno);
}

int main()
{
	yyparse();
	return 0;
}
%}

%token INT WHILE FOR IF ELSE MAIN INVALID
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


%type <nodes> translation_unit stmts declare_list call_list idlist 
%type <node> entry_point func_declare while for if factor expr assign assign_id assign_lv num id string type declare call stmt no_semi_stmt compound_stmt left_val
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
	newNode->isAssigned = true;
	$$->push_back(newNode);
}
| type id
{
	Node* newNode = formNode(PARAM_t,SINGLE_t);
	newNode->intValue = $1->subType;
	newNode->strValue = $2->strValue;
	newNode->isAssigned = true;
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

stmt: no_semi_stmt SEMI {$$ = $1;}
| if {$$ = $1;}
| for {$$ = $1;}
| while {$$ = $1;}
| compound_stmt {$$ = $1;}
| RETURN expr SEMI
{
	$$ = formNode(STMT_t,RET_t);
	$$->children.push_back($2);
}
;

no_semi_stmt: assign {$$ = $1;}
| declare {$$ = $1;}
| expr {$$ = $1;}
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
| assign_id COMMA idlist
{
	$$ = $3;
	$3->push_back($1);
}
| id 
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
| assign_id 
{
	$$ = new vector<Node*>();
	$$->push_back($1);
}
;

assign:assign_lv
{
	$$ = $1;
}
| assign_id
{
	$$ = $1;
};

assign_lv: left_val OP_ASSIGN expr
{
	$$ = formNode(STMT_t,ASSIGN_t);
	$$->children.push_back($1);
	$1->isAssigned = true;
	$$->children.push_back($3);
}
;

assign_id: id OP_ASSIGN expr
{
	$$ = formNode(STMT_t,ASSIGN_t);
	$$->children.push_back($1);
	$1->isAssigned = true;
	$$->children.push_back($3);
}
;

left_val: OP_MUL id
{
	Node* sym = formNode(OP_t,OP_MUL_t);
	$$ = formNode(VAR_t,LEFT_VALUE_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
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
	// $$ = formNode(EXPR_t);
	// $$->children.push_back($1);
	$$ = $1;
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
	Node* sym = formNode(OP_t,OP_MUL_t);
	$$ = formNode(EXPR_t);
	$$->children.push_back(sym);
	$$->children.push_back($2);
}
;

factor: id {$$ = $1;}
| num 
{
	$$ = $1;
	$$->symType = SYM_INT_t;
}
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

for: FOR LBS no_semi_stmt SEMI expr SEMI no_semi_stmt RBS stmt
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