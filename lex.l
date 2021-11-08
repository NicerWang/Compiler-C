%option noyywrap
%{
#include "main.h"

#include "y.tab.h"

class Var{
public: 
    string name;
    SubType type = INT_t;
    int id;
    Var(string name, int id):
        name(name),id(id){}
};

vector<Var*> symbolTable;
int checkExist(string name);
int installID(string name);

%}

delim [ \t\n]
ws {delim}+
letter [A-Za-z]
digit [0-9]
id {letter}({letter}|{digit})*
number {digit}
inlineNote \/\/[^\n]*
multiNote "/*"[^*]*[*]+([^*/][^*]*[*]+)*[*]*"/"

%%
"main"      return MAIN;
"int"       return INT;
"while"     return WHILE;
"for"       return FOR;
"if"        return IF;
"else"      return ELSE;
"in"        return IN;
"out"       return OUT;

"++"        return OP_PP;
"--"        return OP_MM;
"="         return OP_ASSIGN;
"+"         return OP_ADD;
"-"         return OP_SUB;
"*"         return OP_MUL;
"/"         return OP_DIV;
"%"         return OP_MOD;
"!"         return NOT;
"&&"        return AND;
"||"        return OR;

"<"         return LT;
"<="        return LE;
">"         return GT;
">="        return GE;
"=="        return EQ;
"!="        return NE;

";"         return SEMI;
","         return COMMA;
"("         return LBS;       
")"         return RBS; 
"{"         return LP;
"}"         return RP;
"["         return LBT;
"]"         return RBT;

{ws} {}
{inlineNote} {}
{multiNote} {}
{number}+   {
                yylval.node = new Node();
                yylval.node->intValue = atoi(yytext); return NUMBER;}
{id}        { 
                yylval.node = new Node();
                int t = checkExist(yytext);
                yylval.node->strValue = yytext;
                if(t == -1) {
                    yylval.node->intValue = installID(yytext);
                }
                else yylval.node->intValue = t;
                return ID;
            }

%%

int checkExist(string name){
    for(int i = 0; i < symbolTable.size(); i++){
        if(symbolTable[i]->name == name){
            return i;
        }
    }
    return -1;
}
int installID(string name){
    symbolTable.push_back(new Var(name,symbolTable.size()));
    return symbolTable.size() - 1;
}