#include "network.h"
#include "sorter.h"
#include "pot.h"
#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
	// Initialization of all threads
	network_start();
	pot_start();
	display_start();
	sorter_start_sorting();

	pthread_mutex_lock(&main_mutex);
	// Mutex gets blocked so main can run until 'stop' is called from network
	pthread_mutex_lock(&main_mutex);

	// Clean-up of all threads
	network_stop();
	display_stop();
	pot_stop();
	sorter_stop_sorting();

	return 0;
}

void main_unlock_mutex() {
	pthread_mutex_unlock(&main_mutex);
}
