#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen

#include <errno.h> // -> errno, perror

#include "growString.h"

#define SAVED_LINES 10

int main(int argc, char *argv[]) {
	if (argc > 2) {
		char *name = (argc > 0) ? argv[0] : "last10";
		fprintf(stderr,
			"Please supply arguments in either form:\n"
			"%s\n" "%s <file name>\n",
			name, name
		);
		exit(EXIT_FAILURE);
	}
	
	bool fromFile = argc == 2;
	FILE *subject = stdin;
	if (fromFile && (subject = fopen(argv[1], "r")) == NULL) {
		perror("An error occurred while opening the file");
		exit(EXIT_FAILURE);
	}
	
	// Initialize a list of GrowStrings to store the last 10 lines.
	GrowString lineBuffer[SAVED_LINES];
	for (size_t i = 0; i < SAVED_LINES; i++)
		growstr_default(&lineBuffer[i]);
	
	int nextChar; bool newLine = false;
	while ((nextChar = fgetc(subject)) != EOF) {
		// If last character was a newline, rotate the buffers back.
		if (newLine) {
			// Destroy oldest buffered line.
			growstr_destroy(&lineBuffer[0]);
			
			// Shift back all the lines.
			memmove(
				&lineBuffer[0], &lineBuffer[1],
				sizeof(GrowString) * (SAVED_LINES - 1)
			);
			
			// Create a "new" line.
			growstr_default(&lineBuffer[SAVED_LINES - 1]);
			
			// A nice gesture.
			newLine = false;
		}
		
		// Push a character into the string.
		growstr_push(&lineBuffer[SAVED_LINES - 1], nextChar);
		
		// Update "new line" status.
		newLine = nextChar == '\n';
	}
	
	// Remember to close your file handles...
	if (fromFile) fclose(subject);
	
	// Print out the last 10 or so lines!
	for (size_t i = 0; i < SAVED_LINES; i++) {
		if (lineBuffer[i].data != NULL)
			printf("%s", lineBuffer[i].data);
		growstr_destroy(&lineBuffer[i]);
	}
	
	return EXIT_SUCCESS;
}
