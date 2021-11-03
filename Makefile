CC = clang++-12
CPPFLAGS = $(shell llvm-config-12 --cxxflags)
LDFLAGS = $(shell llvm-config-12 --libs --ldflags)

kaleidoscope: lex.yy.o parser.o ast.o
	${CC} ${LDFLAGS} -Wall $^ -o $@

lex.yy.o: lex.yy.c parser.tab.hpp ast.hpp
	${CC} ${CPPFLAGS} -Wno-deprecated -Wall -c $< -o $@

lex.yy.c: lexer.l
	flex $<

parser.o: parser.tab.cpp parser.tab.hpp ast.hpp
	${CC} ${CPPFLAGS} -Wall -c $< -o $@

parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -vd $<

ast.o: ast.cpp ast.hpp
	${CC} ${CPPFLAGS} -Wall -c $< -o $@
	
.PHONY:

clean:
	rm -rf *.tab.* *.o lex.yy.c *.output kaleidoscope *.ll *.s a.out
