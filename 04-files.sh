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
