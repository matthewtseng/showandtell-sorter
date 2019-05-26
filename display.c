#include "display.h"
#include "pot.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

// Addresses
#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

// Bit pattern for register A and B
// A (0x14) for bottom, B (0x15) for top
#define B_TOP 0x04
#define B_RIGHT 0x02
#define B_LEFT 0x80
#define B_BOT 0x08
#define A_TOP 0x10
#define A_RIGHT 0x80
#define A_LEFT 0x01
#define A_BOT 0x20

#define DISPLAY_BASE 10
#define DISPLAY_MAX_DIGIT 99

// Patterns for specific digits
// Find digits to display by: (for two digit display)
// / 10 to find the first digit
// % 10 to find the last digit
#define B_0 B_TOP | B_RIGHT | B_LEFT
#define A_0 A_RIGHT | A_LEFT | A_BOT
#define B_1 B_RIGHT
#define A_1 A_RIGHT
#define B_2 B_TOP | B_RIGHT | B_BOT
#define A_2 A_TOP | A_LEFT | A_BOT
#define B_3 B_TOP | B_RIGHT | B_BOT
#define A_3 A_TOP | A_RIGHT | A_BOT
#define B_4 B_RIGHT | B_BOT | B_LEFT
#define A_4 A_TOP | A_RIGHT
#define B_5 B_TOP | B_LEFT | B_BOT
#define A_5 A_TOP | A_RIGHT | A_BOT
#define B_6 B_TOP | B_LEFT | B_BOT
#define A_6 A_TOP | A_RIGHT | A_BOT | A_LEFT
#define B_7 B_TOP | B_RIGHT
#define A_7 A_RIGHT
#define B_8 B_TOP | B_RIGHT | B_BOT | B_LEFT
#define A_8 A_TOP | A_RIGHT | A_BOT | A_LEFT
#define B_9 B_TOP | B_RIGHT | B_BOT | B_LEFT
#define A_9 A_TOP | A_RIGHT | A_BOT

static pthread_t display_tid;
static int loop = 1;

static struct timespec t = { .tv_nsec = 5e6 };

void display_start(void) {
	display_configure_pins();
	// May need to enable I2C1 cape using:
	// echo BB-I2C1 > /sys/devices/platform/bone_capemgr/slots
	pthread_create(&display_tid, NULL, display_thread, NULL);
}

void display_stop(void) {
	loop = 0;
	pthread_join(display_tid, NULL);
}

void* display_thread(void* arg) {
	int digit_to_display;
	int left_digit;
	int right_digit;
	while (loop) {

		int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

		// Set up for output
		writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
		writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

		digit_to_display = pot_digit();

		// Change number to display when larger than 100
		if (digit_to_display > DISPLAY_MAX_DIGIT) {
			digit_to_display = DISPLAY_MAX_DIGIT;
		}

		// Turn off both digits by driving a 0 to GPIO pins
		edit_file("/sys/class/gpio/gpio61/value","0");
		edit_file("/sys/class/gpio/gpio44/value","0");

		// Set and display left digit
		left_digit = digit_to_display / DISPLAY_BASE;

		if (left_digit == 0) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_0);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_0);
		} else if (left_digit == 1) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_1);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_1);
		} else if (left_digit == 2) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_2);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_2);
		} else if (left_digit == 3) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_3);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_3);
		} else if (left_digit == 4) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_4);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_4);
		} else if (left_digit == 5) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_5);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_5);
		} else if (left_digit == 6) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_6);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_6);
		} else if (left_digit == 7) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_7);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_7);
		} else if (left_digit == 8) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_8);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_8);
		} else if (left_digit == 9) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_9);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_9);
		}

		// Turn on left digit
		edit_file("/sys/class/gpio/gpio61/value","1");

		// Sleep/wait for 5 ms
		nanosleep(&t, NULL);

		// Turn off both digits by driving a 0 to GPIO pins
		edit_file("/sys/class/gpio/gpio61/value","0");
		edit_file("/sys/class/gpio/gpio44/value","0");

		// Set and display right digit
		right_digit = digit_to_display % DISPLAY_BASE;

		if (right_digit == 0) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_0);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_0);
		} else if (right_digit == 1) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_1);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_1);
		} else if (right_digit == 2) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_2);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_2);
		} else if (right_digit == 3) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_3);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_3);
		} else if (right_digit == 4) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_4);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_4);
		} else if (right_digit == 5) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_5);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_5);
		} else if (right_digit == 6) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_6);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_6);
		} else if (right_digit == 7) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_7);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_7);
		} else if (right_digit == 8) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_8);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_8);
		} else if (right_digit == 9) {
			writeI2cReg(i2cFileDesc, REG_OUTA, A_9);
			writeI2cReg(i2cFileDesc, REG_OUTB, B_9);
		}

		// Turn on right digit
		edit_file("/sys/class/gpio/gpio44/value","1");

		// Sleep/wait for 5 ms
		nanosleep(&t, NULL);

		// Cleanup I2C access;
		close(i2cFileDesc);
	}
	pthread_exit(NULL);
}

void display_configure_pins(void) {
	// Configure display pins
	edit_file("/sys/class/gpio/export", "61");
	edit_file("/sys/class/gpio/export", "44");
	edit_file("/sys/class/gpio/gpio61/direction","out");
	edit_file("/sys/class/gpio/gpio44/direction","out");

	// Turn on digit
	edit_file("/sys/class/gpio/gpio61/value","1");
	edit_file("/sys/class/gpio/gpio44/value","1");
}

int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(1);
	}
	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}
	return i2cFileDesc;
}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value) {
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("I2C: Unable to write i2c register.");
		exit(1);
	}
}

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr) {
	// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("I2C: Unable to write to i2c register.");
		exit(1);
	}
	// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		perror("I2C: Unable to read from i2c register");
		exit(1);
	}
	return value;
}


