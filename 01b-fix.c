#include <stdlib.h>
#include <stdio.h>

// takes a pointer to a pointer to an integer.
void funct(int **p2) {
	// give the pointer to the integer an allocation of heap memory.
	*p2 = malloc(sizeof(int));
	
	// assign to and print the integer.
	**p2 = 14;
	printf("%d\n", **p2);
}

int main() {
	// uninitialized, but it's okay because funct will complete it.
	int *p;
	
	// pass reference to local variable holding pointer to integer.
	funct(&p); // &p is of type int**.
	
	// print the integer, confirming the local variable is now initialized
	// and that it has the value funct set.
	printf("%d\n", *p);
	
	// don't cause a memory leak!!
	free(p);
	
	return EXIT_SUCCESS;
}
