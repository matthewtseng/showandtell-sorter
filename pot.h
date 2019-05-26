// pot.h
// Module to spawn a separate thread to read potentiometer
// on a background thread. It provides access to the current
// voltage reading, the number of arrays sorted in the previous
// second, and calculating the next array size for the sorter module.

#ifndef POT_H_
#define POT_H_

// Start/stop reading from potentiometer
void pot_start(void);
void pot_stop(void);

// Thread function to read potentiometer
void* pot_thread(void* arg);

// Read analog input voltage 0
int pot_get_voltage_reading(void);

// Used by sorter function to determine size of array
int pot_get_next_array_size(void);

// Export pins to allow reading of potentiometer
void pot_export_pins(void);

// Give digit to display module
int pot_digit(void);

int pot_find_pwl(float a, float b, float m, float n, float reading);

// Helper function to export pins
void edit_file(char *fileName, char *value);

#endif /* POT_H_ */
