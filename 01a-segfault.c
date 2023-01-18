#include <stdlib.h>
#include <stdio.h>

// -> funct should be defined before main.
void funct() {
	// -> *p2 argument never actually used.
	
	int *p = malloc(sizeof(int));
	// -> don't need to cast pointer type on right side, left side is enough.
	// -> should use sizeof type instead of bare byte count.
	
	*p = 14;
	printf("%d\n", *p);
}

int main() {
	int *p;
	funct();
	
	printf("%d\n", *p);
	
	return EXIT_SUCCESS;
	// -> main should return an exit value
}
