// main.h
// Main module that connects to other modules.
// Calls initialization functions and clean-up functions from other modules.

#ifndef MAIN_H_
#define MAIN_H_

#include "sorter.h"
#include "network.h"

// Function to be called by network so that program can stop
void main_unlock_mutex();

#endif /* MAIN_H_ */
