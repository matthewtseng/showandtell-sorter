// sorter.h
// Module to spawn a separate thread to sort random arrays
// (permutations) on a background thread. It provides access to the
// contents of the current (potentially partially sorted) array,
// and to the count of the total number of arrays sorted.

#ifndef _SORTER_H_
#define _SORTER_H_

// Begin/end the background thread which sorts random permutations.
void sorter_start_sorting(void);
void sorter_stop_sorting(void);

// Bubble sort function to be ran by pthread
void* sorter_bubblesort_thread(void* arg);

// Helper functions for bubble sort:
// sorter_bubblesort_permutation applies permutation to argument array
// sorter_bubblesort_swapper swaps the values of the first and the second addresses
void sorter_bubblesort_permutation(int arr[]);
void sorter_bubblesort_swapper(int* first, int* second);

// Get the size of the array currently being sorted.
int sorter_get_array_size(void);

// Get a copy of the current (potentially partially sorted) array.
// Returns a newly allocated array and sets 'length' to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
int* sorter_get_array_data(int *length);
		
// Get the number of arrays which have finished being sorted.
long long sorter_get_num_arrays_sorted(void);

#endif
