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

## Question 2 - TODO

Jeez.. Why?

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
