V Wheatley  
Systems Programming

<link rel="stylesheet" href="./my.css" />
<!-- pandoc -p --lua-filter=replace.lua --output=warmUp.html warmUp.md -->

# Warm-Up

All this code is available as a Git repository [over at my GitHub](https://github.com/vwheatle/SystemsProgramming-WarmUp).

## Question 1

> In the following code, the first `printf` reached produces the output "14", but the second `printf` can cause a bus error / segmentation fault. Why?

Before I answered, I went through and formatted the program to my liking. The functionality should be the same (it still segfaults) but now it's clearer where problems lie, because I've annotated a few oddities.

### `01a-segfault.c`

```{.c include=01a-segfault.c}
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

```{.c include=01b-fix.c}
```

Hooray for indirection! Sorry if this is too many words and not much meaning per word, I wrote out my *entire* thought process.

## Question 2

> Write a C program that reads a text file and prints out any words that begin with a user-given string. The filename should be given at the command line as an argument. The program should prompt the user for the search string. The program should then read the file one word at a time and print(?) out the word if its first N bytes match the search string, where N is the length of the search string.

I wrote a growable string library for this.

### `growString.h`

```{.c include=growString.h}
```

### `02-find-word.c`

```{.c include=02-find-word.c}
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

```{.bash include=04-files.sh}
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

```{.c include=05-welcome.c}
```

```
$ gcc -Wall -Werror -o ./welcome 05-welcome.c
$ ./welcome
TODO: Put an interesting text string here.
```

## Question 6

> Write, compile, and execute a C program that prints its arguments.

### `06-echo.c`

```{.c include=06-echo.c}
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

```{.c include=07-bathroom.c}
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

```{.c include=08a-libhelloworld.c}
```

```
$ gcc -c -Wall -Werror -o ./libhelloworld.o 08a-libhelloworld.c
$ nm ./libhelloworld.o
                 U puts
0000000000000000 T sayHelloWorld
```

And reading `nm`'s manual page, I know now that `U` in front of a name means it's undefined. Seeing `puts` as an undefined symbol makes sense &mdash; but it doesn't mean the program won't work, just that while linking this object, it'll need to be provided the definition for `puts`. Also, it's neat that `printf` compiles down to `puts` in this scenario!

Meanwhile, our symbol `sayHelloWorld` has `T`, which means it's defined at that memory address written to the left of `T`, and it's in the text section of the object. (Sections are a familiar concept from Computer Organization, but it's unrelated here so I'll skip over it.)

Anyway, that's the object containing the `sayHelloWorld` symbol... now for the program that'll use it!

### `08b-helloworld.c`

```{.c include=08b-helloworld.c}
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
> <pre>CAT(1)                           User Commands                          CAT(1)
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
> <pre>printf(3)                  Library Functions Manual                  printf(3)
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
> <span style="background:black;color:white"> Manual page printf(3) line 1 (press h for help or q to quit)</span>
> </pre>

### `write`

And finally, `write`, the system call. This means it'll be in section 2.

```
$ man write.2
```
> <pre>write(2)                      System Calls Manual                     write(2)
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
> <span style="background:black;color:white"> Manual page write(2) line 1 (press h for help or q to quit)</span>
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

```{.c include=11a-lrange.c}
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

Thankfully, question 2 comes to the rescue! All I really had to add to my `growString.h` library was a helper method for initializing a growable string.

(Note: I automatically write the source files into this document, so the helper method is already there in Question 2's source listing. It shouldn't effect anything.)

#### `11b-last10.c`

```{.c include=11b-last10.c}
```

```
$ gcc -Werror -Wall -o last10 11a-last10.c
$ ./last10 < sampleText.txt

In April 1988, the second edition of the book was published, updated to cover the changes to the language resulting from the then-new ANSI C standard, particularly with the inclusion of reference material on standard libraries. The second edition of the book (and as of 2022, the most recent) has since been translated into over 20 languages. In 2012, an eBook version of the second edition was published in ePub, Mobi, and PDF formats.

ANSI C, first standardized in 1989 (as ANSI X3.159-1989), has since undergone several revisions, the most recent of which is ISO/IEC 9899:2018 (also termed C17 or C18), adopted as an ANSI standard in June 2018. However, no new edition of The C Programming Language has been issued to cover the more recent standards. 

Brian Kernighan and Dennis Ritchie are quoted as saying "Supercalifragilistic expialidocious. More people have been inside the CS building than I have. The quick brown fox jumps over the lazy dog." When asked how this relates to C, they responded with "Well, this whole paragraph isn't real. [[V Wheatley]] (red link) is just trying to write some funny stuff."

ab ab ac ad

thin
```

Aww, and I forgot that I had a few test phrases at the end there for debugging question 2.
