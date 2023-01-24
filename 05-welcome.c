#include <stdlib.h>
#include <stdio.h>

int main() {
	// 5: Blink; 92: Bright Green text color.
	// `man console_codes` for info
	printf("\033[5;92mTODO: Put an interesting text string here.\033[0m\n");
	
	// // makes my eyes hurt
	// for (int jb = 0; jb < 2; jb++)
	// for (int j = 0; j < 8; j++) {
	// 	for (int ib = 0; ib < 2; ib++)
	// 	for (int i = 0; i < 8; i++) {
	// 		int fg = (!ib) ? (i + 30) : (i +  90);
	// 		int bg = (!jb) ? (j + 40) : (j + 100);
	// 		printf("\033[%d;%dm.▄", fg, bg);
	// 	}
	// 	printf("\033[0m\n");
	// }
	
	return EXIT_SUCCESS;
}
