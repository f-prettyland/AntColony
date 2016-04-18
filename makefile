
all: vecadd

ants: ants.c BorrowedFunc.h Structure.h
	gcc -std=c99 ants.c -lOpenCL -o ants

antsg: ants.c BorrowedFunc.h Structure.h
	gcc -std=c99 ants.c -lOpenCL -o ants -g

vecadd: vecadd.c
	gcc -std=c99 vecadd.c -lOpenCL -o vecadd

test: kernelTest.c
	gcc -std=c99 kernelTest.c -lOpenCL -o kernelTest

disTest: disTest.c
	gcc -std=c99 disTest.c -lOpenCL -o disTest

clean:
	rm -f vecadd *~
	rm -f ants *~
	rm -f disTest *~
	rm -f kernelTest *~
