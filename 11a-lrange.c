#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen

#include <errno.h> // -> errno, perror

int main(int argc, char *argv[]) {
	if (argc < 3 || argc > 4) {
		fprintf(stderr,
			"Please supply arguments in the form:\n"
			"%s <start line> <end line> <file name>\n",
			(argc > 0) ? argv[0] : "lrange"
		);
		exit(EXIT_FAILURE);
	}
	
	unsigned int startLine = strtoul(argv[1], NULL, 0);
	unsigned int endLine   = strtoul(argv[2], NULL, 0);
	
	bool fromFile = argc == 4;
	FILE *subject = stdin;
	if (fromFile && (subject = fopen(argv[3], "r")) == NULL) {
		perror("An error occurred while opening the file");
		exit(EXIT_FAILURE);
	}
	
	// Again, like in bathroom:
	// A line is an occurrence of a line break character.
	//  This will include empty lines, and it also means that an input without
	//  any line breaks will have "0 lines". This is normal.
	unsigned int currentLine = 0;
	
	int nextChar;
	while ((nextChar = fgetc(subject)) != EOF) {
		// Repeat char to stdout if within the range.
		if (currentLine >= startLine && currentLine < endLine)
			fputc(nextChar, stdout);
		
		if (nextChar == '\n') currentLine++;
	}
	
	// Remember to close your file handles...
	if (fromFile) fclose(subject);
	
	return EXIT_SUCCESS;
}
