testlexer.o: testlexer.c gbasic.h
	gcc -c ./testlexer.c -o ./testlexer.o -g

lexer.o: lexer.c gbasic.h
	gcc -c ./lexer.c -o ./lexer.o -g

testlexer: testlexer.o lexer.o
	gcc ./testlexer.o ./lexer.o -o testlexer -g