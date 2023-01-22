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
	g->capacity = 1; // eeeeeeeeeeeeeh.. it works?
}

// Clear the string, leaving the container full of zeroes.
void growstr_clear(GrowString *g) {
	memset(g->data, 0, sizeof(char) * g->length);
	g->length = 0;
}

// Grow the string's container to fit the new capacity.
// You can shrink the container with this method,
// but it will silently fail to grow if you mess up.
void growstr_grow(GrowString *g, size_t newCapacity) {
	if (newCapacity <= g->length) return;
	
	// TODO: better thing to use
	// char *nextData = realloc(g->data, sizeof(char) * newCapacity);
	
	char *nextData = calloc(newCapacity, sizeof(char));
	memcpy(nextData, g->data, g->length);
	free(g->data);
	g->data = nextData;
	g->capacity = newCapacity;
}
// (the function name sounds like you're cheering it on...)

void growstr_snipstart(GrowString *g, size_t start) {
	if (start >= g->length) return growstr_clear(g);
	
	size_t remain = g->length - start;
	memmove(g->data, &g->data[start], sizeof(char) * remain);
	memset(&g->data[remain], 0, sizeof(char) * start);
	
	g->length = remain;
	
	// [ a b >c< 0 ] snip to 2 (len: 3; remain: 3-2=1)
	// [ c<< b c 0 ] move 
	// [ c >0 0< 0 ] set
	// thats probably fine
}

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
