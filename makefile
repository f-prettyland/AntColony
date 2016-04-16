
all: vecadd

ants: ants.c
	gcc -std=c99 ants.c -lOpenCL -o ants

vecadd: vecadd.c
	gcc -std=c99 vecadd.c -lOpenCL -o vecadd

test: kernelTest.c
	gcc -std=c99 kernelTest.c -lOpenCL -o kernelTest

clean:
	rm -f vecadd *~
	rm -f ants *~
