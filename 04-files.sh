#! /bin/bash
set -euo pipefail

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
