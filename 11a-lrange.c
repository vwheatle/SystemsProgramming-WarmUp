#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen
#include <limits.h> // -> ULONG_MAX

#include <errno.h> // -> errno, perror

// Helper macro that makes a few lines a bit shorter.
#define perror_fail(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	if (argc < 3 || argc > 4) {
		fprintf(stderr,
			"Please supply arguments in the form:\n"
			"%s <start line> <end line> <file name>\n",
			(argc > 0) ? argv[0] : "lrange"
		);
		exit(EXIT_FAILURE);
	}
	
	// Parse the start line/end line arguments.
	// There's no (safe) (usable) strtoui, so I'm using strtoul.
	// and then narrowing the types later. Kinda overkill.
	// strtoul is fallible, so we do have to check for errors.
	unsigned long startLineL = strtoul(argv[1], NULL, 0);
	if (startLineL == ULONG_MAX) perror_fail("Invalid start line");
	unsigned long endLineL   = strtoul(argv[2], NULL, 0);
	if (endLineL == ULONG_MAX) perror_fail("Invalid end line");
	unsigned int startLine = startLineL, endLine = endLineL;
	
	// Decide if we should read from a file or stdin
	// based on if there's a file argument.
	bool fromFile = argc == 4;
	FILE *subject = stdin;
	if (fromFile && (subject = fopen(argv[3], "r")) == NULL)
		perror_fail("An error occurred while opening the file");
	
	// Again, like in bathroom:
	// A line is an occurrence of a line break character.
	//  This will include empty lines, and it also means that an input without
	//  any line breaks will have "0 lines". This is normal.
	unsigned int currentLine = 0;
	
	int nextChar;
	while ((nextChar = fgetc(subject)) != EOF) {
		// Repeat char to stdout if within the range.
		// This is not inclusive of the provided end line.
		// If you want it to be inclusive, just... <= endline)
		if (currentLine >= startLine && currentLine < endLine)
			fputc(nextChar, stdout);
		
		// Keep track of how many lines we've encountered.
		if (nextChar == '\n') currentLine++;
	}
	
	// Remember to close your file handles...
	if (fromFile) fclose(subject);
	
	return EXIT_SUCCESS;
}
