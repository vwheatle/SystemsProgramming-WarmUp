#include <stdlib.h>
#include <stdio.h>

// -> funct should be defined before main.
void funct(int *p2) {
	// -> *p2 argument immediately overwritten.
	
	p2 = malloc(sizeof(int));
	// -> don't need to cast pointer type on right side, as
	//    compiler already knows which type to cast malloc to.
	// -> should use sizeof type instead of bare byte count.
	
	*p2 = 14;
	printf("%d\n", *p2);
}

int main() {
	int *p;
	funct(p);
	
	printf("%d\n", *p);
	
	return EXIT_SUCCESS;
	// -> main should return an exit value
}
