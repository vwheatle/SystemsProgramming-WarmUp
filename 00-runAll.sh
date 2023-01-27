#!/bin/bash
set -euo pipefail

# lolll..
# - set red background
# - clear current line (to set red formatting for all whitespace)
# - jump back a line (because valgrind is so nice and gives me my own
#    line just to put error markers on... but i don't really need it)
# - print the error (comma)
# - clear all formatting
# - clear current line (to set default formatting for all whitespace)
# - jump back a line (for same reason)
VALGRIND_PRETTY_COLORS=$(printf "\e[31m\e[2K\e[1F,\e[0m\e[2K\e[1F")

GCC_FLAGS=(-g -Wall -Wextra -Wpedantic -Werror)
VALGRIND_FLAGS=(--quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=3 --error-markers="$VALGRIND_PRETTY_COLORS" --error-exitcode=1)

mkdir -p out

echo "Compiling..."

gcc "${GCC_FLAGS[@]}" -o ./out/my-segfault ./01b-fix.c
gcc "${GCC_FLAGS[@]}" -o ./out/find-word ./02-find-word.c
gcc "${GCC_FLAGS[@]}" -o ./out/welcome ./05-welcome.c
gcc "${GCC_FLAGS[@]}" -o ./out/my-echo ./06-echo.c
gcc "${GCC_FLAGS[@]}" -o ./out/bathroom ./07-bathroom.c
gcc "${GCC_FLAGS[@]}" -c -o ./out/libhelloworld.o ./08a-libhelloworld.c
gcc "${GCC_FLAGS[@]}" -o ./out/hw ./out/libhelloworld.o ./08b-helloworld.c
gcc "${GCC_FLAGS[@]}" -o ./out/lrange ./11a-lrange.c
gcc "${GCC_FLAGS[@]}" -o ./out/last10 ./11b-last10.c

if [[ "$*" =~ "run" ]]; then
	echo "Automatic program checking!!!"
	echo "All programs are compiled and then run through Valgrind."
	echo "Arguments are copy-pasted into an echo, and printed right before the run."

	printf "\n=== ./01b-fix.c ===\n"
	echo "$ ./out/my-segfault"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/my-segfault

	printf "\n=== ./02-find-word.c ===\n"
	echo "$ ./out/find-word ./sampleText.txt"
	echo "(try entering 'c'...)"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/find-word ./sampleText.txt

	printf "\n=== ./04-files.sh ===\n"
	echo "$ ./04-files.sh"
	./04-files.sh

	printf "\n=== ./05-welcome.c ===\n"
	echo "$ ./out/welcome"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/welcome

	printf "\n=== ./06-echo.c ===\n"
	echo "$ ./out/my-echo 1 2 3 '1 2 3'"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/my-echo 1 2 3 '1 2 3'

	printf "\n=== ./07-bathroom.c ===\n"
	echo "$ ./out/bathroom < ./sampleText.txt"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/bathroom < ./sampleText.txt

	printf "\n=== ./08b-helloworld.c ===\n"
	echo "$ nm ./out/libhelloworld.o"
	nm ./out/libhelloworld.o
	echo "$ ./out/hw"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/hw

	printf "\n=== ./11a-lrange.c ===\n"
	echo "$ ./out/lrange 6 7 ./sampleText.txt"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/lrange 6 7 ./sampleText.txt

	printf "\n=== ./11b-last10.c ===\n"
	echo "$ ./out/last10 ./sampleText.txt"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/last10 ./sampleText.txt
	echo "$ ./out/last10 < ./sampleText.txt"
	valgrind "${VALGRIND_FLAGS[@]}" ./out/last10 < ./sampleText.txt

	printf "\n=== done! ===\n"
fi
