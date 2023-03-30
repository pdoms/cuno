CC = cc

all: main 

main: main.c cuno.o
	${CC} main.c cuno.o -o main

cuno: cuno.h cuno.c 
	${CC} cuno.c cuno.h -o cuno.o


