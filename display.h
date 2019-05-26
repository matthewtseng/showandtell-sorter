// display.h
// Module to spawn a separate thread to interact with display
// on a background thread.

#ifndef DISPLAY_H_
#define DISPLAY_H_

// Start/stop display thread
void display_start(void);
void display_stop(void);

// Thread function to display different numbers depending on the
// number of sorts in the previous second
void* display_thread(void* arg);

// Configure pins necessary to display numbers
// Used in the display_start function
void display_configure_pins(void);

// Functions taken from I2C guide
int initI2cBus(char* bus, int address);
void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

#endif /* DISPLAY_H_ */
