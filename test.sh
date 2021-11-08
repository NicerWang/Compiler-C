lex lex.l
yacc -d -v yacc.y --debug
g++ lex.yy.c y.tab.c -o out
cat main.cpp | ./out > result.txt

