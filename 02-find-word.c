#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen
#include <ctype.h> // -> isspace

#define MAX_SEARCH_LEN 128
#define MAX_MATCH_LEN (MAX_SEARCH_LEN*2)

int main(int argc, char *argv[]) {
	// Make sure user supplied the correct number of arguments.
	if (argc != 2) { // (First argument is always the program executable.)
		printf("Please supply a file name as the first argument.\n");
		exit(EXIT_FAILURE);
	}
	
	// Read search string from stdin.
	char searchString[MAX_SEARCH_LEN];
	printf("Enter search string: ");
	if (fgets(searchString, MAX_SEARCH_LEN, stdin) == NULL) {
		printf("Something bad happened while reading stdin.\n");
		exit(EXIT_FAILURE);
	} // otherwise, searchString now contains the user-supplied string.
	
	// Note that I do accept spaces in the search string. When I match the
	// beginning of the search string (no matter if it's whitespace or not)
	// the program will shift into the "trying to see if it matches all the
	// string" mode rather than "wait for the next word" mode.
	
	size_t searchStringLen = strlen(searchString);
	
	// Error if user submitted a zero-character string.
	if (searchStringLen <= 0) {
		printf("Please enter a search string!\n");
		exit(EXIT_FAILURE);
	}
	
	// Trim the last newline character, if present.
	// (Had to do this in assembly once! Annoying!)
	// (fgets stops reading after recieving a newline from stdin, but it still
	//  does add this newline to the buffer (unless the buffer'd be filled))
	if (searchString[searchStringLen - 1] == '\n') {
		searchString[--searchStringLen] = '\0';
	}
	
	// Open requested file.
	FILE *subject = fopen(argv[1], "r");
	
	// Create a string to store the contents of the file
	// that *match* the user-specified search string.
	char matchString[MAX_MATCH_LEN];
	int matchStringLen = 0;
	
	// Safe to use fgetc because it reads byte-by-byte but returns a value
	// wider than a byte to store EOF in. It scared me at first, but ehhh..
	// Also, here's hoping it doesn't freaking call read() every time I
	// call this thing. (Isn't one of C's retro claims to fame that "don't
	//  worry, it has buffered file I/O!" or am i misremembering?...)
	int nextChar; bool newWord = true;
	while ((nextChar = fgetc(subject)) != EOF) {
		if (matchStringLen && matchStringLen < searchStringLen) {
			// If we've already found a promising start...
			
			// Check if...
			// 1. the match buffer has any more of the search string to
			//    compare against. (curr. match shorter than search str.)
			// 2. the character we just read matches the next character of
			//    the search string, so that we can continue building up
			//    a match to display.
			// 3. the match string is fixed size, so we don't want to
			//    overflow its buffer. this will always be true, but
			//    i think it's cool   to keep here.
			if (nextChar == searchString[matchStringLen]) {
				// Add the character we just read to the match buffer.
				matchString[matchStringLen++] = nextChar;
				continue;
			}
			
			// (read until we run out of space in the buffer
			//  or we hit a whitespace char.)
			if (!isspace(nextChar)) {
				newWord = true;
			}
		} else if (newWord && nextChar == searchString[0]) {
			matchString[matchStringLen++] = nextChar;
		} else {
			newWord = isspace(nextChar);
		}
	}
	
	if (ferror(subject)) {
		printf("Oops! Something bad happened while reading the file.\n");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
