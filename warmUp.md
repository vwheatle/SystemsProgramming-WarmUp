V Wheatley  
Systems Programming

# Warm-Up

## Question 1

> In the following code, the first `printf` reached produces the output "14", but the second `printf` can cause a bus error / segmentation fault. Why?

Before I answered, I went through and formatted the program to my liking. The functionality should be the same (it still segfaults) but now it's clearer where problems lie, because I've annotated a few oddities.

### `01a-segfault.c`

```c
#include <stdlib.h>
#include <stdio.h>

// -> funct should be defined before main.
void funct(int *p2) {
	// -> *p2 argument immediately overwritten.
	
	p2 = malloc(sizeof(int));
	// -> don't need to cast pointer type on right side, as
	//    compiler already knows which type to cast malloc to.
	// -> should use sizeof type instead of bare byte count.
	
	*p2 = 14;
	printf("%d\n", *p2);
}

int main() {
	int *p;
	funct(p);
	
	printf("%d\n", *p);
	
	return EXIT_SUCCESS;
	// -> main should return an exit value
}
```

Their intended program flow:
- A pointer `*p` to an integer `p` is created in `main`.
- `p` is passed by reference into `funct`, as `p2`.
- `funct` `malloc`ates some memory on the heap for an integer...
- and `p2` is updated with the address of this heap-stored integer.
	- (and this all assumes we don't immediately OoM for some reason...)
- the value `14` is written to the heap-stored integer, and then printed.
- we return from `funct` and print the value of `p`.

What the program really does:
- An uninitialized pointer to an integer `p` is created in `main`.
- `p` is passed **by value** into `funct`, as `p2`.
	- This means that `main` and `funct` don't actually have a way of agreeing on a place to put the value `p`.
- `funct` `malloc`ates some memory on the heap for an integer...
- and `p2` is **overwritten** with the address of this heap-stored integer.
	- This means that we lose the address `main` wanted `p2` to use...
- and `funct` returns, losing the reference to the heap-stored integer and leaking memory.
- finally, `main` tries to dereference its still-not-actually-initialized pointer and segfaults and dies.

Even if we passed a pointer to the local integer pointer by writing `funct(&p)`, this wouldn't be enough &mdash; we still need to update the types to indicate that we're passing a pointer to a pointer to a thing. Convoluted!

Anyway, onto a fixed version.

### `01b-fix.c`

```c
#include <stdlib.h>
#include <stdio.h>

// takes a pointer to a pointer to an integer.
void funct(int **p2) {
	// give the pointer to the integer an allocation of heap memory.
	*p2 = malloc(sizeof(int));
	
	// assign to and print the integer.
	**p2 = 14;
	printf("%d\n", **p2);
}

int main() {
	// uninitialized, but it's okay because funct will complete it.
	int *p;
	
	// pass reference to local variable holding pointer to integer.
	funct(&p); // &p is of type int**.
	
	// print the integer, confirming the local variable is now initialized
	// and that it has the value funct set.
	printf("%d\n", *p);
	
	return EXIT_SUCCESS;
}
```

Hooray for indirection! Sorry if this is too many words and not much meaning per word, I wrote out my *entire* thought process.

## Question 2

> Write a C program that reads a text file and prints out any words that begin with a user-given string. The filename should be given at the command line as an argument. The program should prompt the user for the search string. The program should then read the file one word at a time and print(?) out the word if its first N bytes match the search string, where N is the length of the search string.

I wrote a growable string library for this.

### `growArray.h`

```c
#include <stddef.h> // -> size_t, ptrdiff_t
#include <stdlib.h> // -> malloc, free
#include <stdio.h> // -> printf

#include <string.h> // -> memcpy (lol)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// It's fine to store this thing on the stack.
// Don't mutate the internal variables directly
// unless you know what you're doing.
typedef struct {
	char *data;   // pointer to string data (always a valid C string)
	size_t length;   // length of string (index of null termination byte)
	// note that this means length should always be less than capacity.
	size_t capacity; // size of malloc'd container
} GrowString;

// Create a new growable string with a preset capacity.
// Note that when a string grows, its capacity is doubled.
// This means that it's recommended to supply a power of 2 as the capacity.
GrowString growstr_new_with_capacity(size_t startCapacity) {
	GrowString g;
	g.data = calloc(startCapacity, sizeof(char));
	g.length = 0;
	g.capacity = startCapacity > 0 ? startCapacity : 1;
	return g;
}

// Create a new growable string from an existing C string.
// This allocates a new container and copies the contents, so
// after calling this you can really do whatever with the C string.
GrowString growstr_from_cstr(char *s, size_t capacity) {
	GrowString g;
	g.length = strlen(s);
	if (capacity < g.length) capacity = 0;
	g.capacity = capacity > 0 ? capacity : g.length + 1;
	g.data = calloc(g.capacity, sizeof(char));
	memcpy(g.data, s, g.length);
	return g;
}

// Return a new GrowString, one which points to a clone of
// the provided other GrowString.
GrowString growstr_clone(GrowString *other) {
	GrowString g;
	g.data = calloc(other->capacity, sizeof(char));
	memcpy(g.data, other->data, other->length);
	g.length = other->length;
	g.capacity = other->capacity;
	return g;
}

// Destroy the string, including freeing its container.
// If you try to push onto this, it'll automatically grow
// back into a real string! (Probably.) Neat.
void growstr_destroy(GrowString *g) {
	free(g->data);
	g->data = NULL; // if freed, nothing happens.
	g->length = 0;
	g->capacity = 1; // capacity needs its null byte space..
}

// Clear the string, leaving the container full of zeroes.
// This does not shrink or free the container.
void growstr_clear(GrowString *g) {
	memset(g->data, 0, sizeof(char) * g->length);
	g->length = 0;
}

// Grow the string's container to fit the new capacity.
// You can shrink the container with this method,
// but it will silently fail to grow if you mess up.
void growstr_grow(GrowString *g, size_t newCapacity) {
	if (newCapacity <= g->length) return;
	
	char *nextData = realloc(g->data, sizeof(char) * newCapacity);
	
	// I wish there was recalloc...
	// Instead, I have to manually zero the new memory.
	// Also, since we allow shrinking, I have to make sure this is signed.
	ptrdiff_t remain = newCapacity - g->capacity;
	if (remain > 0) memset(&nextData[g->capacity], 0, sizeof(char) * remain);
	
	// char *nextData = calloc(newCapacity, sizeof(char));
	// memcpy(nextData, g->data, g->length);
	// free(g->data);
	
	g->data = nextData;
	g->capacity = newCapacity;
}
// (the function name sounds like you're cheering it on...)

// Erase the front of the string up to but not including `start`.
// This will shift all unerased characters back.
void growstr_snipstart(GrowString *g, size_t start) {
	if (start >= g->length) return growstr_clear(g);
	
	size_t remain = g->length - start;
	memmove(g->data, &g->data[start], sizeof(char) * remain);
	memset(&g->data[remain], 0, sizeof(char) * start);
	
	g->length = remain;
	
	// [ a b >c< 0 ] snip to 2 (len: 3; remain: 3-2=1)
	// [ c<< b c 0 ] move remainder of string to front
	// [ c >0 0< 0 ] fill with 0s after moved region, length: (length - remain)
	// thats [sic] probably fine
}

// Return the index of the first char that satisfies the predicate.
// (It should return a boolean value, but unfortunately it's unsafe to
//  pass in isspace unless I make it awkwardly cast to and from an int...)
// (if i had my way, it'd be bool (*fn)(char) not int (*fn)(int) ...)
ptrdiff_t growstr_indexofpredicate(GrowString *g, int (*fn)(int), size_t start) {
	for (size_t i = start; i < g->length; i++)
		if (fn(g->data[i])) return i;
	return -1;
}

// Push a character onto the end of the string.
// If there's not enough space, this will grow the string so there is.
void growstr_push(GrowString *g, char c) {
	if (g->length + 1 >= g->capacity)
		growstr_grow(g, g->capacity * 2);
	g->data[(g->length)++] = c;
	g->data[g->length] = '\0';
}

// Append a string onto the end of the string.
// If there's not enough space, this will grow the string by a power of two.
void growstr_pushstr(GrowString *g, char *s) {
	if (s == NULL) return;
	
	size_t sl = strlen(s);
	if (g->length + sl >= g->capacity) {
		// would really love std::usize::next_power_of_two right about now
		size_t newCap = MAX(g->length + sl, g->capacity * 2);
		growstr_grow(g, newCap);
	}
	
	memcpy(&(g->data[g->length]), s, sl);
	g->length += sl;
	g->data[g->length] = '\0';
}

// Pop a character from the end of the string.
// Returns -1 if you're a fool and there's no characters left.
// Yes, that's in-band sentinel whatever, but you should know better
// and check ahead of time before popping.
// That, and they didn't give me Option<T>, so I have to make do.
char growstr_pop(GrowString *g) {
	if (g->length < 1) return -1;
	
	char the = g->data[--(g->length)];
	g->data[g->length] = '\0';
	return the;
}
```

### `02-find-word.c`

```c
#include <stdlib.h> // -> EXIT_*, malloc
#include <stdio.h> // -> printf, File I/O
#include <stdbool.h> // -> bool

#include <string.h> // -> strlen
#include <ctype.h> // -> isspace

#include <errno.h> // -> errno, perror

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
	
	// Open requested file, and check if it was opened successfully.
	FILE *subject;
	if ((subject = fopen(argv[1], "r")) == NULL) {
		perror("An error occurred while opening the file");
		exit(EXIT_FAILURE);
	}
	
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
```

```
$ gcc -Wall -Werror -o ./find-word 02-find-word.c
$ ./find-word sampleText.txt
Enter search string: C
C
C
C.
[ and so on ]
C17
C18),
C
CS
C,
```

## Question 3

> Explain the purpose of the `ls`, `cat`, `rm`, `cp`, `mv`, `mkdir`, and `cc` Unix commands.

- `ls` prints a list of all files in the current working directory.
- `cat` outputs the entirety of a specified file into stdout. (Its name is meant to be short for "concatenate" &mdash; as you can specify more than one file, and it will print all of them in order.)
- `rm` removes (deletes) a specified file. If you want, it can also recursively delete entire folder structures, files included!
- `cp` copies a specified file or folder structure to a destination.
- `mv` is like `cp` but it moves files to the destination instead of copying. It also renames them! That's written as if you're moving a file from its old name to its new name.
- `mkdir` creates an empty directory. If you want, it can even make several nested directories all in one go!
- `cc` is short for "C compiler". It takes one or more C source files and outputs an executable! Or an object that you can then link with other objects to make an executable. Nowadays, it's probably an alias for `gcc` or `clang` or something.

## Question 4

> Using your favorite editor, create a small text file. Use `cat` to create another file consisting of five repetitions of this small text file. Use `wc` to count the number of characters and words in the original file and in the one you made from it. Explain the result. Create a subdirectory and move the two files into it.

### `04-files.sh`

```bash
#!/bin/bash
set -euo pipefail

# Create the file.
printf "Hello, world!\nI am a small text file!\n" > small_text_file.txt

# Create another file consisting of five repetitions of the small text file.
cat small_text_file.txt small_text_file.txt small_text_file.txt small_text_file.txt small_text_file.txt > another_text_file.txt

# Count number of characters and words.
# ( -w : words; -m : characters )
wc -wm small_text_file.txt
wc -wm another_text_file.txt

# Create a directory.
mkdir -p my_text_files

# Move both the files into this new directory.
mv small_text_file.txt another_text_file.txt my_text_files
```

```
$ ./04-files.sh
 8 38 small_text_file.txt
 40 190 another_text_file.txt
```

And yeah, the result of the `wc` commands were kinda surprising to me for a moment &mdash; I had `wc -mw <file>` written for a bit, but it turns out that the order of the single-letter arguments is not respected, and it always prints them in a specific order! It'll always print the number of words first, then the number of characters. Thankfully, this is documented in its manual page.

## Question 5

> Write, compile, and execute a C program that prints a welcoming message of your choice.

### `05-welcome.c`

```c
#include <stdlib.h>
#include <stdio.h>

int main() {
	// 5: Blink; 92: Bright Green text color.
	// `man console_codes` for info
	printf("\033[5;92mTODO: Put an interesting text string here.\033[0m\n");
	
	return EXIT_SUCCESS;
}
```

```
$ gcc -Wall -Werror -o ./welcome 05-welcome.c
$ ./welcome
TODO: Put an interesting text string here.
```

## Question 6

> Write, compile, and execute a C program that prints its arguments.

### `06-echo.c`

```c
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	for (size_t i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	
	return EXIT_SUCCESS;
}
```

```
$ gcc -Wall -Werror -o ./my-echo 06-echo.c
$ ./my-echo a b c
./my-echo
a
b
c
```

## Question 7

> Using `getchar()`, write a program that counts the number of words, lines, and characters in its input.

Affectionately calling this "bathroom", because it's an alternative to `wc`.

### `07-bathroom.c`

```c
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
```

```
$ gcc -Wall -Werror -o ./bathroom 07-bathroom.c
$ cat sampleText.txt | ./bathroom
2540 characters
418 words
17 lines
```

And yeah, the way I count lines is like `wc`. If the input doesn't have a trailing newline (like most inputs should) then my program reports "0 lines". This is probably fine.

```
$ echo "Hello, world!" | ./bathroom  # echo adds a trailing newline
14 characters
2 words
1 lines
$ printf "Hello, world!" | ./bathroom  # printf doesn't add a trailing newline
13 characters
2 words
0 lines
```

## Question 8

> Create a file containing a C function that prints the message "Hello, world!". Create a separate file containing the main program which calls this function. Compile and link the resulting program, calling it `hw`.

I'm a bit of a newbie to this kinda stuff. At the very least I now know about `gcc -c` (to make an object without immediately linking and turning it into an executable) and ~~`objdump` (to inspect an object file's contents, here using `-t` to filter to just the symbol list inside the object file)~~ `nm` to print (the list of symbols inside an object file).

### `08a-libhelloworld.c`

```c
#include <stdio.h>

void sayHelloWorld() {
	printf("Hello, world!\n");
}
```

```
$ gcc -c -Wall -Werror -o ./out/libhelloworld.o 08a-libhelloworld.c
$ nm ./out/libhelloworld.o
                 U puts
0000000000000000 T sayHelloWorld
```

And reading `nm`'s manual page, I know now that `U` in front of a name means it's undefined. Seeing `puts` as an undefined symbol makes sense &mdash; but it doesn't mean the program won't work, just that while linking this object, it'll need to be provided the definition for `puts`. Also, it's neat that `printf` compiles down to `puts` in this scenario!

Meanwhile, our symbol `sayHelloWorld` has `T`, which means it's defined at that memory address written to the left of `T`, and it's in the text section of the object. (Sections are a familiar concept from Computer Organization, but it's unrelated here so I'll skip over it.)

Anyway, that's the object containing the `sayHelloWorld` symbol... now for the program that'll use it!

### `08b-helloworld.c`

```c
#include <stdlib.h>

extern void sayHelloWorld();

int main() {
	sayHelloWorld();
	return EXIT_SUCCESS;
}
```

And I'm using an `extern` declaration here to say "this function should not be expected to be present in this program, and you gotta link it in from somewhere else!"

```
$ gcc -Wall -Werror -o ./hw libhelloworld.o 08b-helloworld.c
$ ./hw
Hello, world!
```

...And it works!

## Question 9

> Look up the entries for the following topics in your system's manual:
>
> - the `cat` command
> - the `printf` function
> - the `write` system call

Because I constantly forget what the section numbers mean, I'm gonna first run `man man` and copy down just that.

```
$ man man
```
```stdout
Man: find all matching manual pages (set MAN_POSIXLY_CORRECT to avoid this)
 * man (1)
   man (7mp)
   man (1p)
Man: What manual page do you want?
Man: _
```

Augh. Annoying.

```stdout
Man: 1
```

And here's the relevant passage:

> The table below shows the <u>section</u> numbers of the manual followed by the types of pages they contain.
>
> <table>
> <tbody>
> <tr><td>0<td>Header files (usually found in <u>/usr/include</u>)
> <tr><td>1<td>Executable programs or shell commands
> <tr><td>2<td>System calls (functions provided by the kernel)
> <tr><td>3<td>Library calls (functions within program libraries)
> <tr><td>4<td>Special files (usually found in <u>/dev</u>)
> <tr><td>5<td>File formats and conventions, e.g. <u>/etc/passwd</u>
> <tr><td>6<td>Games
> <tr><td>7<td>Miscellaneous (including macro packages and conventions), e.g. <b>man</b>(7), <b>groff</b>(7), <b>man-pages</b>(7)
> <tr><td>8<td>System administration commands (usually only for root)
> <tr><td>9<td>Kernel routines [Non standard]
> </table>

Hmm, and it seems stuff like `1p` means "the POSIX standardized version of this", which can sometimes greatly differ from the thing you have on your computer. For example, `echo` accepts a much smaller set of arguments in the POSIX standard. (GNU developers really like their extensions to C and the shell...)

### `cat`

`cat` is a shell command, so it'll be under section 1.

```
$ MAN_POSIXLY_CORRECT=0 man cat.1
```
> <pre style="font-family:'Iosevka',monospace;">CAT(1)                           User Commands                          CAT(1)
>
> <b>NAME</b>
>        cat - concatenate files and print on the standard output
>
> <b>SYNOPSIS</b>
>        <b>cat </b>[<u>OPTION</u>]... [<u>FILE</u>]...
>
> <b>DESCRIPTION</b>
>        Concatenate FILE(s) to standard output.
>
>        With no FILE, or when FILE is -, read standard input.
>
>        <b>-A</b>, <b>--show-all</b>
>               equivalent to <b>-vET</b>
>
>        <b>-b</b>, <b>--number-nonblank</b>
>               number nonempty output lines, overrides <b>-n</b>
>
>        <b>-e     </b>equivalent to <b>-vE</b>
>
>        <b>-E</b>, <b>--show-ends</b>
>               display $ at end of each line
>
>        <b>-n</b>, <b>--number</b>
>               number all output lines
>
>        <b>-s</b>, <b>--squeeze-blank</b>
>               suppress repeated empty output lines
> <span style="background:white;color:black;"> Manual page cat(1) line 1 (press h for help or q to quit)</span>
> </pre>

### `printf`

While `printf` can also be a shell command, we're talking about the C function. This means it'll be under section 3, "Library Calls".

```
$ MAN_POSIXLY_CORRECT=0 man printf.3
```
> <pre style="font-family:'Iosevka',monospace;">printf(3)                  Library Functions Manual                  printf(3)
>
> <b>NAME</b>
>        printf,  fprintf,  dprintf,  sprintf,  snprintf, vprintf, vfprintf, vd-
>        printf, vsprintf, vsnprintf - formatted output conversion
>
> <b>LIBRARY</b>
>        Standard C library (<u>libc</u>, <u>-lc</u>)
>
> <b>SYNOPSIS</b>
>        <b>#include &lt;stdio.h&gt;</b>
>
>        <b>int printf(const char *restrict </b><u>format</u><b>, ...);</b>
>        <b>int fprintf(FILE *restrict </b><u>stream</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, ...);</b>
>        <b>int dprintf(int </b><u>fd</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, ...);</b>
>        <b>int sprintf(char *restrict </b><u>str</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, ...);</b>
>        <b>int snprintf(char </b><u>str</u><b>[restrict .</b><u>size</u><b>], size_t </b><u>size</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, ...);</b>
>
>        <b>int vprintf(const char *restrict </b><u>format</u><b>, va_list </b><u>ap</u><b>);</b>
>        <b>int vfprintf(FILE *restrict </b><u>stream</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, va_list </b><u>ap</u><b>);</b>
>        <b>int vdprintf(int </b><u>fd</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, va_list </b><u>ap</u><b>);</b>
>        <b>int vsprintf(char *restrict </b><u>str</u><b>,</b>
>                    <b>const char *restrict </b><u>format</u><b>, va_list </b><u>ap</u><b>);</b>
> <span style="background:white;color:black"> Manual page printf(3) line 1 (press h for help or q to quit)</span>
> </pre>

### `write`

And finally, `write`, the system call. This means it'll be in section 2.

```
$ man write.2
```
> <pre style="font-family:'Iosevka',monospace;">write(2)                      System Calls Manual                     write(2)
>
> <b>NAME</b>
>        write - write to a file descriptor
>
> <b>LIBRARY</b>
>        Standard C library (<u>libc</u>, <u>-lc</u>)
>
> <b>SYNOPSIS</b>
>        <b>#include &lt;unistd.h&gt;</b>
>
>        <b>ssize_t write(int </b><u>fd</u><b>, const void </b><u>buf</u><b>[.</b><u>count</u><b>], size_t </b><u>count</u><b>);</b>
>
> <b>DESCRIPTION</b>
>        <b>write</b>() writes up to <u>count</u> bytes from the buffer starting at <u>buf</u> to the
>        file referred to by the file descriptor <u>fd</u>.
>
>        The number of bytes written may be less than  <u>count</u>  if,  for  example,
>        there  is  insufficient space on the underlying physical medium, or the
>        <b>RLIMIT_FSIZE </b>resource limit is encountered (see <b>setrlimit</b>(2)),  or  the
>        call was interrupted by a signal handler after having written less than
>        <u>count</u> bytes.  (See also <b>pipe</b>(7).)
>
>        For a seekable file (i.e., one to which <b>lseek</b>(2) may  be  applied,  for
>        example,  a  regular  file) writing takes place at the file offset, and
>        the file offset is incremented by the number of bytes actually written.
>        If  the  file was <b>open</b>(2)ed with <b>O_APPEND</b>, the file offset is first set
>        to the end of the file before writing.  The adjustment of the file off-
>        set and the write operation are performed as an atomic step.
> <span style="background:white;color:black"> Manual page write(2) line 1 (press h for help or q to quit)</span>
> </pre>

## Question 10

> Write a function that computes some basic statistics for a list of numbers and stores those results in fields of a struct. In particular, given this definition...
>
> ```c
> struct numlist {
> 	float *list; // list of floats.
> 	int len; // number of items in list.
> 	float min, max, avg; // statistics.
> };
> ```
>
> ...write a function `compute_stats(struct numlist *listptr)`, which takes a list that already has the `->list` and `->len` fields populated.

```c
void compute_stats(struct numlist *listptr) {
	float total = 0.0;
	
	for (int i = 0; i < listptr->len; i++) {
		float thisOne = listptr->list[i];
		
		// Could be more efficient but eh. Stack space is precious.
		if (thisOne < listptr->min) listptr->min = thisOne;
		if (thisOne > listptr->max) listptr->max = thisOne;
		
		total += thisOne;
	}
	
	// (int gets promoted to float)
	listptr->avg = total / listptr->len;
}
```

## Question 11

### Part 1

> Write a program that prints a range of lines from a text file. The program should take command line arguments of the form:
>
> `lrange 10 20 filename`
>
> ...which will print lines 10 through 20 of the named file. If there aren't enough lines in the file, the program should print what it can.

#### `11a-lrange.c`

```c
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
```

My version is not inclusive of the end part of the line range. This can easily be changed, thankfully. As simple as changing line 32 to `if (currentLine >= startLine && currentLine <= endLine)`.

```
$ gcc -Werror -Wall -o lrange 11a-lrange.c
$ ./lrange 6 7 sampleText.txt
The first edition, published February 22, 1978, was the first widely available book on the C programming language. Its version of C is sometimes termed K&R C (after the book's authors), often to distinguish this early version from the later version of C standardized as ANSI C.
```

### Part 2

> Write a program called `last10` that prints the last ten lines of a text file. The program can be used from the command line with:
>
> ```
> last10 <filename>
> last10
> ```
>
> If no file is provided (the latter), the program processes standard input.

But yeah, since it's possible to use `stdin`, `fseek`ing backwards is out of the question. This means we'll need to save our own line buffer.
