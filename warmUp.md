V Wheatley  
Systems Programming

# Warm-Up

## Question 1

> In the following code, the first `printf` reached produces the output "14", but the second `printf` can cause a bus error / segmentation fault. Why?

Before I answer -- I've gone through and formatted the program to my liking. The functionality should be the same (it still segfaults) but now it's clearer where problems lie, thanks to the code having space to breathe.

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

Even if we passed a pointer to the local integer pointer by writing `funct(&p)`, this wouldn't be enough -- we still need to update the types to indicate that we're passing a pointer to a pointer to a thing. Convoluted!

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

Hooray for indirection! Sorry if this is too many words, I wrote out my *entire* thought process.

## Question 2

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

### `main.c`
```c
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
```

## Question 3

- `ls` prints a list of all files in the current working directory.
- `cat` outputs the entirety of a specified file into stdout. (Its name is meant to be short for "concatenate" -- as you can specify more than one file, and it will print all of them in order.)
- `rm` removes (deletes) a specified file. If you want, it can also recursively delete entire folder structures, files included!
- `cp` copies a specified file or folder structure to a destination.
- `mv` is like `cp` but it moves files to the destination instead of copying. It also renames them! That's written as if you're moving a file from its old name to its new name.
- `mkdir` creates an empty directory. If you want, it can even make several nested directories all in one go!
- `cc` is short for "C compiler". It takes one or more C source files and outputs an executable! Or an object that you can then link with other objects to make an executable. Nowadays, it's probably an alias for `gcc` or `clang` or something.

## Question 4

<!-- # (Nightmare syntax for specifying multiple lines.)
# cat <<-THE_TEXT_FILE > small_text_file.txt
# 	Hello, world!
# 	I am a small text file!
# THE_TEXT_FILE -->

```bash
# Create the file.
printf "Hello, world!\nI am a small text file!\n" > small_text_file.txt

# Create another file consisting of five repetitions of the small text file.
cat small_text_file.txt small_text_file.txt small_text_file.txt small_text_file.txt small_text_file.txt > another_text_file.txt

# Count number of characters and words.
# ( -m : characters; -w : words )
wc -mw small_text_file.txt
wc -mw another_text_file.txt

# Create a directory.
mkdir my_text_files

# Move both the files into this new directory.
mv small_text_file.txt another_text_file.txt my_text_files
```

## Question 9 - TODO

the real challenge here is to remember what the numbers in each man page entry's name mean

## Question 10

> Write a function that computes some basic statistics for a list of numbers and stores those results in fields of a struct. In particular, given this definition...

```c
struct numlist {
	float *list; // list of floats.
	int len; // number of items in list.
	float min, max, avg; // guess.
};
```

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

> Note: if you are not able to do the first part, you are not prepared to take this class.

ok

>

# note tho  slef

So. Pipes. and accepting data from `stdin`. Pipes actually do communicate the flags
uh.

noo um actually there's like. you can close stdout, and that's a signal to anyone listening (i.e. their stdin is your stdout) to stop. because the file has an EOF signal now, etc..

and the main point of confusion is when i use a tool that reads from stdin and i type some stuff and i have to press ctrl+d to stop.. like how do program say ctrl+d?? EOF?? but if it's an in-band magic byte then it'd fuck up !!

so the answer is just that. they need to call linux `close` with their stdout file descriptor to say "that's it, tthat's the  output". and this also happens automatically when a program closes. so it works real well real efficient!!! so cool!
