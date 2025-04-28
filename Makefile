all: L1simulate

L1simulate: parser.o input.o 
	g++ -o L1simulate parser.o input.o

input.o: input.cpp
	g++ -c input.cpp -o input.o 

parser.o: parser.cpp parser.h
	g++ -c parser.cpp -o parser.o

clean: 
	rm -f *.o L1simulate