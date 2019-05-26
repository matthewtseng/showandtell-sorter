// network.h
// Module to spawn a separate thread to host a network on a background thread
// so the host can communicate with target through text commands.

#ifndef NETWORK_H_
#define NETWORK_H_

// Begin/end the background thread for establishing network
void network_start(void);
void network_stop(void);

// Network thread function to be called by pthread
void* network_thread(void* arg);

#endif /* NETWORK_H_ */
