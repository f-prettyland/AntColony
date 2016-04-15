
all: vecadd

ants: ants.c
	gcc -std=c99 ants.c -lOpenCL -o ants

vecadd: vecadd.c
	gcc -std=c99 vecadd.c -lOpenCL -o vecadd

clean:
	rm -f vecadd *~
	rm -f ants *~
