/*
 * UDP Listening program on port 22110
 * By Brian Fraser, Modified from Linux Programming Unleashed (book)
 */

#include "network.h"
#include "sorter.h"
#include "main.h"

#include <pthread.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()

#define UDP_MSG_MAX_LEN 1500
#define MAX_NUM_PER_LINE 10
#define PORT 12345
// Assuming max bytes for digits is 4 with 2 for comma and space and 1 for new line at the end
// Each line is = 6 * 10 + 1 = 61 bytes per line
// 1500 - 61 = 1439
#define MAX_BYTES_PER_MSG 1439

static pthread_t network_tid;
static int loop = 1;

void network_start(void) {
	pthread_create(&network_tid, NULL, network_thread, NULL);
}

void network_stop(void) {
	loop = 0;
	pthread_join(network_tid, NULL);
	printf("The network connection has been closed.\n");
}

void* network_thread(void* arg) {
	printf("Connect using: \n");
	printf("    netcat -u 192.168.7.2 %d\n", PORT);

	// Buffer to hold packet data:
	char message[UDP_MSG_MAX_LEN];

	// Address
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(PORT);                 // Host to Network short

	// Create the socket for UDP
	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	// Bind the socket to the port (PORT) that we specify
	bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

	while (loop) {
		int get_num = 0;

		// Get the data (blocking)
		// Will change sin (the address) to be the address of the client.
		// Note: sin passes information in and out of call!
		unsigned int sin_len = sizeof(sin);
		int bytesRx = recvfrom(socketDescriptor, message, UDP_MSG_MAX_LEN, 0,
				(struct sockaddr *) &sin, &sin_len);

		// Make it null terminated (so string functions work):
		// NOTE: Unsafe in some cases; why?
		message[bytesRx] = 0;
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, message);

		// Extract the value from the message:
		// (Process the message any way your app requires).
		// int incMe = atoi(message);
		//char* incMe = message;

		if (!strcmp(message, "help\n")) {
			sprintf(message, "These are the list of commands sorted:\n"
					"count: Return the number of arrays sorted so far\n"
					"get #: Return the requested value from inside the current array being sorted\n"
					"get length: Returns the length of the array currently being sorted\n"
					"get array: Returns all the data in the current array being sorted\n"
					"stop: Stop sorting arrays and exit the program\n");
		} else if (!strcmp(message, "count\n")) {
			sprintf(message, "%lld arrays have been sorted so far\n", sorter_get_num_arrays_sorted());
		} else if (!strcmp(message, "get length\n")) {
			sprintf(message, "The length of the array currently being sorted is: %d\n", sorter_get_array_size());
		} else if (!strcmp(message, "get array\n")) {
			int curr_len = -1;
			int* curr_arr = sorter_get_array_data(&curr_len);

			// Pointer and counters to keep track of what has been printed
			char* next_pointer = message;
			int bytes_printed = 0;
			int curr_num_printed = 0;

			// Set buffer to 0
			memset(message, 0, UDP_MSG_MAX_LEN);

			// Traverse through array and attach values to message buffer
			for (int i = 0; i < curr_len-1; i++) {

				// Add to buffer
				bytes_printed = sprintf(next_pointer, "%d, ", curr_arr[i]);
				if (bytes_printed < 0) {
					printf("Error in sprintf");
					fflush(stdout);
					break;
				}
				next_pointer += bytes_printed;
				curr_num_printed++;

				if (curr_num_printed > MAX_NUM_PER_LINE) {
					// If 10 numbers reach, make a new line
					bytes_printed = sprintf(next_pointer, "\n");
					if (bytes_printed < 0) {
						printf("Error in sprintf");
						fflush(stdout);
						break;
					}
					next_pointer += bytes_printed;
					// Reset counter to 0 when 10 values are reached
					curr_num_printed = 0;

					// Send when amount of bytes is close to 1500
					if (next_pointer - message > MAX_BYTES_PER_MSG) {
						printf("Close to 1500 bytes, sending values..\n");
						// Transmit a reply
						sin_len = sizeof(sin);
						sendto( socketDescriptor,
								message, strlen(message),
								0,
								(struct sockaddr *) &sin, sin_len);

						// Reset next_pointer and buffer after sending
						memset(message, 0, UDP_MSG_MAX_LEN);
						next_pointer = message;
					}
				}
			}
			// Attach last number to message
			printf("Sending last number..\n");
			sprintf(next_pointer, "%d\n", curr_arr[curr_len-1]);

			// Free return pointer from sorter_get_array_data
			free(curr_arr);
			printf("Exiting get array\n");
		} else if (sscanf(message, "get %d", &get_num) == 1) {
			// use atoi() after the space
			int curr_len = -1;
			int* curr_arr = sorter_get_array_data(&curr_len);
			// Error check for array size
			if (get_num < 1) {
				sprintf(message, "Invalid argument. Must be between 1 and %d.\n", curr_len);
			} else if (get_num > curr_len) {
				sprintf(message, "Invalid argument. Must be between 1 and %d.\n", curr_len);
			} else {
				sprintf(message, "Value %d: %d\n", get_num, curr_arr[get_num - 1]);
			}
			free(curr_arr);
		} else if (!strcmp(message, "stop\n")) {
			loop = 0;
			main_unlock_mutex();
			sprintf(message, "Stopping program\n");
		} else {
			sprintf(message, "The command is unknown, please type 'help' to see available commands\n");
		}

		// Transmit a reply:
		sin_len = sizeof(sin);
		sendto( socketDescriptor,
				message, strlen(message),
				0,
				(struct sockaddr *) &sin, sin_len);
	}

	// Close
	close(socketDescriptor);
	pthread_exit(NULL);
}
