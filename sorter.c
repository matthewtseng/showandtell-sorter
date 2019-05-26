#include "sorter.h"
#include "pot.h"
#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define DEFAULT_ARRAY_LENGTH 100

static long long num_arrays_sorted = 0;
static int current_array_size;

// Current array being sorted
static int* curr_arr;

// Global variables for mutex and thread
static pthread_mutex_t sort_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t sorter_tid;
static int loop = 1;

// Begin/end the background thread which sorts random permutations.
void sorter_start_sorting(void) {
	pthread_create(&sorter_tid, NULL, *sorter_bubblesort_thread, NULL);
}

void sorter_stop_sorting(void) {
	loop = 0;
	pthread_join(sorter_tid, NULL);
	printf("%lld arrays have been sorted.\n", num_arrays_sorted);
}

// Thread function that runs bubble sort on array size of n
void* sorter_bubblesort_thread(void* arg) {
	int len;
	while(loop) {
		// Get next array size from potentiometer
		len = pot_get_next_array_size();

		// Set current array size
		current_array_size = len;

		// Allocate memory for array
		curr_arr = (int *)malloc(len * sizeof(int));

		// Apply permutation to array
		sorter_bubblesort_permutation(curr_arr);

		// Bubble sort
		for (int i = 0; i <= len-1; i++) {
			for (int j = 0; j < len-i-1; j++) {
				// Start critical section
				pthread_mutex_lock(&sort_mutex);

				if (curr_arr[j] > curr_arr[j+1]) {
					sorter_bubblesort_swapper(&curr_arr[j], &curr_arr[j+1]);
				}

				// End critical section
				pthread_mutex_unlock(&sort_mutex);
			}
		}

		// Free memory for array
		free(curr_arr);
		num_arrays_sorted++;
		//printf("2\n");
	}
	pthread_exit(NULL);
}

// Helper function to swap values
void sorter_bubblesort_swapper(int* first, int* second) {
	int temp = *first;
	*first = *second;
	*second = temp;
}

// Apply permutation to array.
void sorter_bubblesort_permutation(int arr[]) {
	// Initialize array from 1 to n.
	int n = sorter_get_array_size();
	for (int i = 0; i < n; i++) {
		arr[i] = i + 1;
	}

	// Swap with a random with random element in array.
	for (int i = 0; i < n; i++) {
		sorter_bubblesort_swapper(&arr[i], &arr[rand() % n]);
	}
}

// Get the size of the array currently being sorted.
int sorter_get_array_size(void) {
	return current_array_size;
}

// Get a copy of the current (potentially partially sorted) array.
// Returns a newly allocated array and sets 'length' to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
int* sorter_get_array_data(int *length) {
	// Lock access to critical section while copying data
	pthread_mutex_lock(&sort_mutex);

	// Allocate new array to copy elements from current array
	int* new_arr = (int *)malloc((int)current_array_size * sizeof(int));

	// Set length to be number of elements in returned array
	*length = current_array_size;

	// Copy elements to new array
	for (int i = 0; i < current_array_size; i++) {
		new_arr[i] = curr_arr[i];
	}

	// Unlock access after completion
	pthread_mutex_unlock(&sort_mutex);

	return new_arr;
}

// Get the number of arrays which have finished being sorted.
long long sorter_get_num_arrays_sorted(void) {
	return num_arrays_sorted;
}
