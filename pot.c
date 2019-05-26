#include "pot.h"
#include "sorter.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Piecewise linear values
#define A2D_READING_PWL1 0
#define A2D_READING_PWL2 500
#define A2D_READING_PWL3 1000
#define A2D_READING_PWL4 1500
#define A2D_READING_PWL5 2000
#define A2D_READING_PWL6 2500
#define A2D_READING_PWL7 3000
#define A2D_READING_PWL8 3500
#define A2D_READING_PWL9 4000
#define A2D_READING_PWL10 4100
#define ARRAY_SIZE_PWL1 1
#define ARRAY_SIZE_PWL2 20
#define ARRAY_SIZE_PWL3 60
#define ARRAY_SIZE_PWL4 120
#define ARRAY_SIZE_PWL5 250
#define ARRAY_SIZE_PWL6 300
#define ARRAY_SIZE_PWL7 500
#define ARRAY_SIZE_PWL8 800
#define ARRAY_SIZE_PWL9 1200
#define ARRAY_SIZE_PWL10 2100

#define DEFAULT_ARRAY_SIZE 100
#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define A2D_MIN_READING 0
// #define A2D_MAX_READING 4095

static pthread_t pot_tid;
static int loop = 1;

static int next_size_check = 0;
static int next_size = 0;

long long number_to_display = 0;

void pot_start(void) {
	// May need to enable A2D Linux functionality by turning on ADC virtual cape:
	// echo BB-ADC > /sys/devices/platform/bone_capemgr/slots
	pot_export_pins();
	pthread_create(&pot_tid, NULL, pot_thread, NULL);
}

void pot_stop(void) {
	loop = 0;
	pthread_join(pot_tid, NULL);
}

void* pot_thread(void* arg) {
	int reading = A2D_MIN_READING;
	// Variables for PWL
	float a, b, m, n = 0;
	long long array_sorted_previous_sec = sorter_get_num_arrays_sorted();
	long long array_sorted_current = 0;

	while (loop) {
		// Calculations to find out arrays sorted in the last second
		array_sorted_current = sorter_get_num_arrays_sorted();
		number_to_display = array_sorted_current - array_sorted_previous_sec;
		array_sorted_previous_sec = array_sorted_current;

		reading = pot_get_voltage_reading();
		// printf("Voltage reading is: %d\n", reading);

		// Perform PWL calculation
		// f'(s) = [(s-a)/(b-a)] * (n-m) + m
		// a and b are pot readings (a: lower bound; b: upper bound)
		// m and n are array sizes (m: lower bound; n: upper bound)

		// Initializing variables
		a = A2D_READING_PWL1; b = A2D_READING_PWL1;
		m = ARRAY_SIZE_PWL1; n = ARRAY_SIZE_PWL1;

		// Calculate piecewise linear and assign to temporary
		// variable to check previous size
		next_size_check = pot_find_pwl(a, b, m, n, reading);

		// Only update size if it is different from the previous
		if (next_size_check != next_size) {
			next_size = next_size_check;
			printf("The next array size is: %d\n", next_size);
		}

		// Perform once every second
		sleep(1);
	}
	pthread_exit(NULL);
}

// Used by sorter function to determine size of array
int pot_get_next_array_size(void) {
	return next_size;
}

// Helper function for pot_thread()
// Returns raw voltage value between 0 to 4095
int pot_get_voltage_reading(void) {
	// Open file
	FILE *f = fopen(A2D_FILE_VOLTAGE0, "r");
	if (!f) {
		printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
		printf("try: echo BB-ADC > /sys/devices/platform/bone_capemgr/slots\n");
		exit(-1);
	}

	// Get reading
	int a2d_reading = 0;
	int items_read = fscanf(f, "%d", &a2d_reading);
	if (items_read <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Close file
	fclose(f);
	return a2d_reading;
}

// Export pins to allow reading of potentiometer
void pot_export_pins(void) {
	// Export potentiometer pins
	edit_file("/sys/class/gpio/export", "39");
}

int pot_digit(void) {
	return number_to_display;
}

int pot_find_pwl(float a, float b, float m, float n, float reading) {
	// Perform PWL calculation
	// f'(s) = [(s-a)/(b-a)] * (n-m) + m
	// a and b are pot readings (a: lower bound; b: upper bound)
	// m and n are array sizes (m: lower bound; n: upper bound)
	if (reading > A2D_READING_PWL9) {
		a = A2D_READING_PWL9;
		b = A2D_READING_PWL10;
		m = ARRAY_SIZE_PWL9;
		n = ARRAY_SIZE_PWL10;
	} else if (reading > A2D_READING_PWL8) {
		a = A2D_READING_PWL8;
		b = A2D_READING_PWL9;
		m = ARRAY_SIZE_PWL8;
		n = ARRAY_SIZE_PWL9;
	} else if (reading > A2D_READING_PWL7) {
		a = A2D_READING_PWL7;
		b = A2D_READING_PWL8;
		m = ARRAY_SIZE_PWL7;
		n = ARRAY_SIZE_PWL8;
	} else if (reading > A2D_READING_PWL6) {
		a = A2D_READING_PWL6;
		b = A2D_READING_PWL7;
		m = ARRAY_SIZE_PWL6;
		n = ARRAY_SIZE_PWL7;
	} else if (reading > A2D_READING_PWL5) {
		a = A2D_READING_PWL5;
		b = A2D_READING_PWL6;
		m = ARRAY_SIZE_PWL5;
		n = ARRAY_SIZE_PWL6;
	} else if (reading > A2D_READING_PWL4) {
		a = A2D_READING_PWL4;
		b = A2D_READING_PWL5;
		m = ARRAY_SIZE_PWL4;
		n = ARRAY_SIZE_PWL5;
	} else if (reading > A2D_READING_PWL3) {
		a = A2D_READING_PWL3;
		b = A2D_READING_PWL4;
		m = ARRAY_SIZE_PWL3;
		n = ARRAY_SIZE_PWL4;
	} else if (reading > A2D_READING_PWL2) {
		a = A2D_READING_PWL2;
		b = A2D_READING_PWL3;
		m = ARRAY_SIZE_PWL2;
		n = ARRAY_SIZE_PWL3;
	} else {
		a = A2D_READING_PWL1;
		b = A2D_READING_PWL2;
		m = ARRAY_SIZE_PWL1;
		n = ARRAY_SIZE_PWL2;
	}

	// Convert into int and then set to next_size
	// f'(s) = [(s-a)/(b-a)] * (n-m) + m
	return (int)( ( ((reading-a)/(b-a)) * (n-m)) + m);
}

// Helper function to export pins
// Changes the file name into specified value
void edit_file(char *fileName, char *value) {
	FILE *file = fopen(fileName, "w");
	if (file == NULL) {
		printf("ERROR: Unable to open file (%s) for write\n", fileName);
		exit(1);
	}
	// Write to data to the file using fprintf():
	fprintf(file, "%s", value);
	//printf("Value on file (%s) has been changed (to: %s)\n", fileName, value);

	// Close the file using fclose():
	fclose(file);
}
