#include <stdlib.h> // -> EXIT_*
#include <stdio.h> // -> getchar
#include <stdbool.h> // -> bool
#include <ctype.h> // -> isspace

int main() {
	// A character is a byte received from the input.
	unsigned int chars = 0;
	
	// A word is the rising edge between a non-space character and a space.
	//  This means it doesn't include multiple spaces in a row.
	unsigned int words = 0;
	bool newWord = true;
	
	// A line is an occurrence of a line break character.
	//  This will include empty lines, and it also means that an input without
	//  any line breaks will have "0 lines". This is normal.
	unsigned int lines = 0;
	
	int nextChar;
	while ((nextChar = getchar()) != EOF) {
		chars++;
		
		if (isspace(nextChar)) {
			newWord = true;
		} else if (newWord) {
			words++;
			newWord = false;
		}
		
		if (nextChar == '\n') lines++;
	}
	
	printf("%d characters\n%d words\n%d lines\n", chars, words, lines);
	
	return EXIT_SUCCESS;
}
