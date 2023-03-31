CC = cc

all: main

cuno: src/cuno.c src/cuno.h 
	${CC} src/cuno.c src/cuno.h -o cuno.o

cli: cli.c cli.h 
	${CC} src/cli.c src/cli.h -o cli.o

main: main.c src/cuno.o src/cli.o
	${CC} main.c src/cuno.o src/cli.o -o main

clean:
	rm -rvf *.o 
	rm -rvf src/*.o
