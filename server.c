/********************************************************************
* FILENAME : libserver.h
* AUTHOR : Tudor Paraschivescu
********************************************************************/

#include "libserver.h"

/* Run:
 * ./server <port_server> <users_file>
 * Example:
 * ./server 10000 users_database.txt
 */
int main(int argc, char *argv[]) {
	
	// Check the number of arguments
	if (argc != 3) {
		printf("The server does not have the correct number of arguments.\n");
		return -10;
	}

	// Read and store the clients accounts
	if (read_users_file(argv) == false) {
		printf("Error at reading and storing the clients accounts\n");
		return -10;
	}

	start_server(argv);

	free_memory();

	return 0;
}