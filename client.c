/********************************************************************
* FILENAME : client.c
* AUTHOR : Tudor Paraschivescu
********************************************************************/

#include "libclient.h"

/* Run:
 * ./client <IP_server> <port_server>
 * Example:
 * ./client 127.0.0.1 10000
 */
int main (int argc, char* argv[]) {

	// Check the number of arguments
	if (argc != 3) {
		printf("The client does not have the correct number of arguments.\n");
		return -10;
	}

	// Attempt connecting to the ATM (TCP)
	if (connect_to_atm(argv) == false) {
		printf("Error appeared when connecting to the ATM.\n");
		return -10;
	}

	// Attempt connecting to the UNLOCK (UDP)
	if (connect_to_unlock(argv) == false) {
		printf("Error appeared when connecting to the UNLOCK.\n");
		return -10;
	}

	// Create the log file
	if (create_log_file() == false) {
		printf("Error appeared when creating the log file\n");
		return -10;
	}

	// Start the session
	if (talk_to_server() == false) {
		printf("Error appeared when talking with the server\n");
		return -10;
	}

	return 0;
}