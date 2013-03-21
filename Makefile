all: paren

paren: paren.cpp libparen.cpp libparen.h
	g++ -std=c++0x -Wall -O3 -o paren paren.cpp libparen.cpp

clean:
	rm -f paren
