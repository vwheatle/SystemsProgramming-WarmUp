#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen
#include <ctype.h> // -> isspace

#include "growArray.h"

#define MAX_SEARCH_LEN 128

int main(int argc, char *argv[]) {
	// Make sure user supplied the correct number of arguments.
	if (argc != 2) { // (First argument is always the program executable.)
		printf("Please supply a file name as the first argument.\n");
		exit(EXIT_FAILURE);
	}
	
	// Read search string from stdin.
	char searchCString[MAX_SEARCH_LEN];
	printf("Enter search string: ");
	if (fgets(searchCString, MAX_SEARCH_LEN, stdin) == NULL) {
		printf("Something bad happened while reading stdin.\n");
		exit(EXIT_FAILURE);
	} // otherwise, searchString now contains the user-supplied string.
	
	GrowString searchString = growstr_from_cstr(searchCString, MAX_SEARCH_LEN);
	
	// Note that I do accept spaces in the search string. When I match the
	// beginning of the search string (no matter if it's whitespace or not)
	// the program will shift into the "trying to see if it matches all the
	// string" mode rather than "wait for the next word" mode.
	
	// Error if user submitted a zero-character string.
	if (searchString.length <= 0) {
		printf("Please enter a search string!\n");
		exit(EXIT_FAILURE);
	}
	
	// Trim the last newline character, if present.
	// (Had to do this in assembly once! Annoying!)
	// (fgets stops reading after recieving a newline from stdin, but it still
	//  does add this newline to the buffer (unless the buffer'd be filled))
	if (searchString.data[searchString.length - 1] == '\n') {
		growstr_pop(&searchString);
	}
	
	// Open requested file.
	FILE *subject = fopen(argv[1], "r");
	
	// Create a string to store the contents of the file
	// that *match* the user-specified search string.
	GrowString matchString = growstr_new_with_capacity(MAX_SEARCH_LEN);
	
	// Safe to use fgetc because it reads byte-by-byte but returns a value
	// wider than a byte to store EOF in. It scared me at first, but ehhh..
	// Also, here's hoping it doesn't freaking call read() every time I
	// call this thing. (Isn't one of C's retro claims to fame that "don't
	//  worry, it has buffered file I/O!" or am i misremembering?...)
	int nextChar; bool newWord = true;
	while ((nextChar = fgetc(subject)) != EOF) {
		if (matchString.length) {
			// If we've already found a promising start...
			
			// Check if...
			// 1. the match buffer has any more of the search string to
			//    compare against. (curr. match shorter than search str.)
			if (matchString.length < searchString.length) {
				// 2. the character we just read matches the next
				//    character of the search string, so that we can
				//    continue building up a match to display.
				
				// printf("%s %c?\t(%s)\n", matchString.data, nextChar, searchString.data);
				
				if (nextChar == searchString.data[matchString.length]) {
					// Add the character we just read to the match buffer.
					growstr_push(&matchString, nextChar);
				} else {
					// It's possible that a space is in the search string.
					// Check if any of the characters after each space are
					// a match with the search string.
					
					// this is scary. sorry
					
					ptrdiff_t nextSpace = 0;
					while ((nextSpace =
						growstr_indexofpredicate(&matchString, isspace, nextSpace + 1)
					) >= 0) {
						for (size_t i = nextSpace + 1; i < matchString.length; i++)
							if (searchString.data[i - (nextSpace + 1)] != matchString.data[i])
								goto not_a_match;
						break; not_a_match:
					}
					
					if (nextSpace >= 0) { // match found
						nextSpace++;
						growstr_snipstart(&matchString, nextSpace);
						ungetc(nextChar, subject); // :)
						// ...and can continue like nothing happened.
					} else { // no match found
						growstr_clear(&matchString);
						newWord = isspace(nextChar);
						// and eat characters until next space!
					}
				}
			} else if (!isspace(nextChar)) {
				// We're reading the rest of the word after we already found
				// a complete match with the search string.
				
				// Add the character we just read to the match buffer.
				growstr_push(&matchString, nextChar);
			} else {
				// We've hit a whitespace character. This means the word is
				// over and we can print it as a result!
				printf("%s\n", matchString.data);
				
				// Now we clean up and update the new word indicator.
				growstr_clear(&matchString);
				newWord = true;
			}
		} else if (newWord && nextChar == searchString.data[0]) {
			// Only triggers if this is a new word.
			// Ooh! This might be a new match!
			growstr_push(&matchString, nextChar);
		} else {
			// Eat character and do nothing.
			// Update new word indicator.
			newWord = isspace(nextChar);
		}
	}
	
	bool didOkay = true;
	
	if (ferror(subject)) {
		printf("Oops! Something bad happened while reading the file.\n");
		didOkay = false;
	}
	
	growstr_destroy(&matchString);
	growstr_destroy(&searchString);
	fclose(subject);
	
	return didOkay ? EXIT_SUCCESS : EXIT_FAILURE;
}
