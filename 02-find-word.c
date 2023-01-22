#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen
#include <ctype.h> // -> isspace

#include "growString.h"

#define MAX_SEARCH_LEN 128

bool startsWith(const char *s, const char *start) {
	while (*s != '\0' && *start != '\0')
		if (*(s++) != *(start++)) return false;
	
	// only successful if `start` was the one that ended first.
	return *start == '\0';
}

int main(int argc, char *argv[]) {
	// Make sure user supplied the correct number of arguments.
	if (argc != 2) { // (First argument is always the program executable.)
		fprintf(stderr, "Please supply a file name as the first argument.\n");
		exit(EXIT_FAILURE);
	}
	
	// Read search string from stdin.
	char searchCString[MAX_SEARCH_LEN];
	printf("Enter search string: ");
	if (fgets(searchCString, MAX_SEARCH_LEN, stdin) == NULL) {
		fprintf(stderr, "Something bad happened while reading stdin.\n");
		exit(EXIT_FAILURE);
	} // otherwise, searchString now contains the user-supplied string.
	
	size_t searchLength = strlen(searchCString);
	
	// Error if user submitted a zero-character string.
	if (searchLength <= 0) {
		fprintf(stderr, "Please enter a search string!\n");
		exit(EXIT_FAILURE);
	}
	
	GrowString searchString = growstr_from_cstr(searchCString, MAX_SEARCH_LEN);
	
	// Note that I do accept spaces in the search string. When I match the
	// beginning of the search string (no matter if it's whitespace or not)
	// the program will shift into the "trying to see if it matches all the
	// string" mode rather than "wait for the next word" mode.
	
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
					// For each space in the match, check if the substring
					// formed by all the characters after that space could
					// be the beginning of a new match using startsWith.
					
					// (Search 'ab ac' in a file containing 'ab ab ac'
					//  to see this scenario in action.)
					
					ptrdiff_t nextSpace = 0; // don't start at 0, we just
					// checked if the string matched starting there. it didn't.
					// (if you're wondering why it's set to 0, see this line: )
					while ((nextSpace = growstr_indexofpredicate(&matchString, isspace, nextSpace + 1)) >= 0)
						if (startsWith(searchString.data, &matchString.data[nextSpace + 1]))
							break;
					
					if (nextSpace >= 0) { // match found
						nextSpace++;
						
						// Remove all characters up to and including
						// the space, leaving only the matching portion.
						growstr_snipstart(&matchString, nextSpace);
						
						// Put the next character back into the file so
						// we get it back when we next loop around. In my
						// opinion, this is more solid than using a goto.
						ungetc(nextChar, subject);
					} else { // no match found
						// Clear match buffer, and start over next loop.
						growstr_clear(&matchString);
						
						// Also, update "new word" indicator since the
						// character that didn't match still could possibly
						// be a space / not a space.
						newWord = isspace(nextChar);
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
		fprintf(stderr, "Oops! Something bad happened while reading the file.\n");
		didOkay = false;
	}
	
	growstr_destroy(&matchString);
	growstr_destroy(&searchString);
	fclose(subject);
	
	return didOkay ? EXIT_SUCCESS : EXIT_FAILURE;
}
