default: a.out

a.out: main.cpp *.hpp **/*.hpp
	clang++ -std=c++2a -O3 -o a.out -lSDL2 main.cpp

run: a.out
	./a.out
