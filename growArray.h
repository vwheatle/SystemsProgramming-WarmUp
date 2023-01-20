#include <stdlib.h> // -> malloc, free
#include <stdio.h> // -> printf

#include <string.h> // -> memcpy (lol)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// It's fine to store this thing on the stack.
// Don't mutate the internal variables directly
// unless you know what you're doing.
struct GrowString {
	char *data;   // pointer to string data (always a valid C string)
	int length;   // length of string (index of null termination byte)
	// note that this means length should always be less than capacity.
	int capacity; // size of malloc'd container
};

// Create a new growable string with a preset capacity.
// Note that when a string grows, its capacity is doubled.
// This means that it's recommended to supply a power of 2 as the capacity.
struct GrowString growstr_new_with_capacity(int startCapacity) {
	struct GrowString g;
	g.data = calloc(startCapacity, sizeof(char));
	g.length = 0;
	g.capacity = startCapacity > 0 ? startCapacity : 1;
	return g;
}

// Create a new growable string from an existing C string.
// This allocates a new container and copies the contents, so
// after calling this you can really do whatever with the C string.
struct GrowString growstr_from_cstr(char *s, int capacity) {
	struct GrowString g;
	g.length = strlen(s);
	if (capacity < g.length) capacity = 0;
	g.capacity = capacity > 0 ? capacity : g.length + 1;
	g.data = calloc(g.capacity, sizeof(char));
	memcpy(g.data, s, g.length);
	return g;
}

// Destroy the string, including freeing its container.
// If you try to push onto this, it'll automatically grow
// back into a real string! (Probably.) Neat.
void growstr_destroy(struct GrowString *g) {
	free(g->data);
	g->data = NULL; // if freed, nothing happens.
	g->length = 0;
	g->capacity = 1; // eeeeeeeeeeeeeh.. it works?
}

// Grow the string's container to fit the new capacity.
// You can't shrink the container with this method.
void growstr_grow(struct GrowString *g, int newCapacity) {
	if (newCapacity < g->capacity) return;
	
	char *nextData = calloc(newCapacity, sizeof(char));
	memcpy(nextData, g->data, g->length);
	free(g->data);
	g->data = nextData;
	g->capacity = newCapacity;
}

// Push a character onto the end of the string.
// If there's not enough space, this will grow the string so there is.
void growstr_push(struct GrowString *g, char c) {
	if (g->length + 1 >= g->capacity)
		growstr_grow(g, g->capacity * 2);
	g->data[(g->length)++] = c;
}

// Append a string onto the end of the string.
// If there's not enough space, this will grow the string by a power of two.
void growstr_pushstr(struct GrowString *g, char *s) {
	if (s == NULL) return;
	
	int sl = strlen(s);
	if (g->length + sl >= g->capacity) {
		// would really love std::usize::next_power_of_two right about now
		int newCap = MAX(g->length + sl, g->capacity * 2);
		growstr_grow(g, newCap);
	}
	
	memcpy(&(g->data[g->length]), s, sl);
	g->length += sl;
}

// Pop a character from the end of the string.
// Returns -1 if you're a fool and there's no characters left.
// Yes, that's in-band sentinel whatever, but you should know better
// and check ahead of time before popping.
// That, and they didn't give me Option<T>, so I have to make do.
char growstr_pop(struct GrowString *g) {
	if (g->length < 1) return -1;
	
	char the = g->data[--(g->length)];
	g->data[g->length] = '\0';
	return the;
}
