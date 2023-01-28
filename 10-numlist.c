#include <stdlib.h>
#include <stdio.h>

#include <math.h>

struct numlist {
	float *list;
	int len;
	float min, max, avg;
};

void compute_stats(struct numlist *listptr) {
	float total = 0.0;
	
	listptr->min = INFINITY;
	listptr->max = -INFINITY;
	
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

int main() {
	float items[] = { 1.0, 2.1, 3.0, 4.0, 5.3 };
	
	struct numlist thing;
	thing.list = items;
	thing.len = sizeof(items) / sizeof(items[0]);
	compute_stats(&thing);
	
	printf("min %f\tmax %f\tavg %f\n", thing.min, thing.max, thing.avg);
	
	return EXIT_SUCCESS;
}
