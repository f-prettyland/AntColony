
all: ants

ants: ants.c BorrowedFunc.h Structure.h
	gcc -std=c99 ants.c -lOpenCL -o ants

antsg: ants.c BorrowedFunc.h Structure.h
	gcc -std=c99 ants.c -lOpenCL -o ants -g

clean:
	rm -f ants *~
