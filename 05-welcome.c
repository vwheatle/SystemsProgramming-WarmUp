#include <stdlib.h>
#include <stdio.h>

int main() {
	// 5: Blink; 92: Bright Green text color.
	// `man console_codes` for info
	printf("\033[5;92mTODO: Put an interesting text string here.\033[0m\n");
	
	return EXIT_SUCCESS;
}
