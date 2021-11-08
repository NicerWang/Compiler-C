lex lex.l
yacc -d -v yacc.y --debug
g++ lex.yy.c y.tab.c -o out
cat ./test/base.cpp | ./out > ./test/result-base.txt
cat ./test/func.cpp | ./out > ./test/result-func.txt
cat ./test/io.cpp | ./out > ./test/result-io.txt

