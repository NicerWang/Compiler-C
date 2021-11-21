rm -f ./out
lex lex.l
yacc -d -v yacc.y --debug
g++ lex.yy.c y.tab.c -o out -w
cat ./test/base.cpp | ./out > ./test/result-base.txt
cat ./test/func.cpp | ./out > ./test/result-func.txt
cat ./test/ptr.cpp | ./out > ./test/result-ptr.txt
echo "Done"
