%option noyywrap
%{
#include "structure.h" 

#include "y.tab.h"

extern string lexRes;


%}
%option yylineno

delim [ \t\n]
ws {delim}+
letter [A-Za-z_]
digit [0-9]
id {letter}({letter}|{digit})*
number {digit}
inlineNote \/\/[^\n]*
multiNote "/*"[^*]*[*]+([^*/][^*]*[*]+)*[*]*"/"
string \".*\"
invalid .

%%
"main"      lexRes += "MAIN\n";return MAIN;
"int"       lexRes += "INT\n";return INT;
"while"     lexRes += "WHILE\n";return WHILE;
"for"       lexRes += "FOR\n";return FOR;
"if"        lexRes += "IF\n";return IF;
"else"      lexRes += "ELSE\n";return ELSE;
"return"    lexRes += "RETURN\n";return RETURN;

"++"        lexRes += "OP_PP\n";return OP_PP;
"--"        lexRes += "OP_MM\n";return OP_MM;
"="         lexRes += "OP_ASSIGN\n";return OP_ASSIGN;
"+"         lexRes += "OP_ADD\n";return OP_ADD;
"-"         lexRes += "OP_SUB\n";return OP_SUB;
"*"         lexRes += "OP_MUL\n";return OP_MUL;
"/"         lexRes += "OP_DIV\n";return OP_DIV;
"%"         lexRes += "OP_MOD\n";return OP_MOD;
"!"         lexRes += "NOT\n";return NOT;
"&&"        lexRes += "AND\n";return AND;
"||"        lexRes += "OR\n";return OR;
"&"         lexRes += "BIT_AND\n";return BIT_AND;

"<"         lexRes += "LT\n";return LT;
"<="        lexRes += "LE\n";return LE;
">"         lexRes += "GT\n";return GT;
">="        lexRes += "GE\n";return GE;
"=="        lexRes += "EQ\n";return EQ;
"!="        lexRes += "NE\n";return NE;

";"         lexRes += "SEMI\n";return SEMI;
","         lexRes += "COMMA\n";return COMMA;
"("         lexRes += "LBS\n";return LBS;       
")"         lexRes += "RBS\n";return RBS; 
"{"         lexRes += "LP\n";return LP;
"}"         lexRes += "RP\n";return RP;
"["         lexRes += "LBT\n";return LBT;
"]"         lexRes += "RBT\n";return RBT;

{ws} {}
{inlineNote} {}
{multiNote} {}
{number}+   {
                yylval.node = new Node();
                lexRes += "NUMBER ";
                lexRes += yytext;
                lexRes += "\n";
                yylval.node->intValue = atoi(yytext); return NUMBER;
            }
{id}        { 
                yylval.node = new Node();
                lexRes += "ID ";
                lexRes += yytext;
                lexRes += " ~";
                lexRes += "\n";
                yylval.node->strValue = yytext;
                return ID;
            }
{string}    {
                yylval.node = new Node();
                yylval.node->strValue = yytext;
                return STR;
            }
{invalid}   {
                return INVALID;
            }
%%
