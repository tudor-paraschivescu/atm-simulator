/********************************************************************
* FILENAME : libserver.h
* AUTHOR : Tudor Paraschivescu
********************************************************************/

#include "libcommon.h"

// The maximum number of clients accepted at once
#define MAX_CLIENTS 50

// The structure of a user (account)
typedef struct __attribute__((__packed__)) {
	// User info
	char family_name[13];
	char first_name[13];
	char card_no[7];
	char pin[5];
	char secret_password[17];
	double sold;

	// User attributes
	unsigned short login_attempts;
	bool is_blocked;
	bool is_logged;
	int sessionfd;
} user;

// The structure of a bank
typedef struct {
	int N;
	user *accounts;
} bank;

bank my_bank;

// Create a new error package
err_pkg create_err_pkg(error_type type, command_type last_cmd, char *message) {
	err_pkg new_err_pkg;
	
	new_err_pkg.size_of_package = sizeof(int) + sizeof(error_type)
		+ sizeof(command_type) + strlen(message);
	new_err_pkg.type = type;
	new_err_pkg.answer_to = last_cmd;
	strcpy(new_err_pkg.message, message);

	return new_err_pkg;
}

// Read abd store the data from the given input file
bool read_users_file(char *argv[]) {

	// Open the users file
	char* users_file_name = argv[2];
	FILE* users_file = fopen(users_file_name, "r");
	if (users_file == NULL) {
		return false;
	}

	// Read the number of users
	int aux;
	fscanf(users_file, "%d", &aux);
	my_bank.N = aux;

	// Create the user acounts
	my_bank.accounts = (user*) calloc(my_bank.N, sizeof(user));
	if (my_bank.accounts == NULL) {
		return false;
	}

	// Read the data of the user and initialise the accounts
	for (int i = 0; i < my_bank.N; i++) {
		double aux_sold;
		fscanf(users_file, "%s %s %s %s %s %lf",
			my_bank.accounts[i].family_name,
			my_bank.accounts[i].first_name,
			my_bank.accounts[i].card_no,
			my_bank.accounts[i].pin,
			my_bank.accounts[i].secret_password,
			&aux_sold);
		my_bank.accounts[i].sold = aux_sold;
		my_bank.accounts[i].login_attempts = 0;
		my_bank.accounts[i].is_blocked = false;
		my_bank.accounts[i].is_logged = false;
		my_bank.accounts[i].sessionfd = 0;
	}

	fclose(users_file);
	return true;
}

// Print the information of a bank
void print_bank() {
	printf("%d\n", my_bank.N);
	for (int i = 0; i < my_bank.N; i++) {
		printf("%s %s %s %s %s %.2lf %hu %d %d %d\n",
			my_bank.accounts[i].family_name,
			my_bank.accounts[i].first_name,
			my_bank.accounts[i].card_no,
			my_bank.accounts[i].pin,
			my_bank.accounts[i].secret_password,
			my_bank.accounts[i].sold,
			my_bank.accounts[i].login_attempts,
			my_bank.accounts[i].is_blocked,
			my_bank.accounts[i].is_logged,
			my_bank.accounts[i].sessionfd);
	}
}

err_pkg execute(cmd_pkg cmd, int fd) {

	err_pkg response;

	if (cmd.type == login) {
		char *card_no = cmd.parameters;
		char *pin = &(cmd.parameters[7]);
		bool card_exists = false;

		for (int i = 0; i < my_bank.N; i++) {
			if (strncmp(my_bank.accounts[i].card_no, card_no, 6) == 0) {
				card_exists = true;

				if (my_bank.accounts[i].is_logged == true) {
					// The user is logged in another process
					response = create_err_pkg(ERROR2, login, "");
					goto end;
				}

				if (my_bank.accounts[i].login_attempts == 3) {
					// A wrong PIN has been entered 3 times
					response = create_err_pkg(ERROR5, login, "");
					// Block the account
					my_bank.accounts[i].is_blocked = true;
					goto end;
				}

				if (strncmp(my_bank.accounts[i].pin, pin, 4) == 0) {
					
					char *welcome_text = calloc(50, 1);
					sprintf(welcome_text, "Welcome %s %s\n",
							my_bank.accounts[i].family_name,
							my_bank.accounts[i].first_name);

					my_bank.accounts[i].login_attempts = 0;
					my_bank.accounts[i].is_logged = true;
					my_bank.accounts[i].sessionfd = fd;
					
					// Login finished succesfuly
					response = create_err_pkg(NOERROR, login, welcome_text);
					goto end;

				} else {
					my_bank.accounts[i].login_attempts++;
					if (my_bank.accounts[i].login_attempts == 3) {
						// A wrong PIN has been entered 3 times
						response = create_err_pkg(ERROR5, login, "");
						// Block the account
						my_bank.accounts[i].is_blocked = true;
						goto end;
					} else {
						// Wrong PIN
						response = create_err_pkg(ERROR3, login, "");
						goto end;
					}
				}
			}
		}

		if (card_exists == false) {
			// The card number given does not exist
			response = create_err_pkg(ERROR4, login, "");
		}
	}

	else if (cmd.type == logout) {

		for (int i = 0; i < my_bank.N; i++) {
			if (my_bank.accounts[i].sessionfd == fd) {
				my_bank.accounts[i].is_logged = false;
				my_bank.accounts[i].sessionfd = 0;

				// Logout finished succesfuly
				response = create_err_pkg(NOERROR, logout,
						"Deconectare de la bancomat!\n");
				goto end;
			}
		}
	}

	else if (cmd.type == listsold) {

		for (int i = 0; i < my_bank.N; i++) {
			if (my_bank.accounts[i].sessionfd == fd) {

				char *sold = calloc(20, 1);
				sprintf(sold, "%.2f\n", my_bank.accounts[i].sold);

				// Show sold finished succesfuly
				response = create_err_pkg(NOERROR, listsold, sold);
				goto end;
			}
		}
	}

	else if (cmd.type == getmoney) {

		for (int i = 0; i < my_bank.N; i++) {
			if (my_bank.accounts[i].sessionfd == fd) {

				// Remove the newline from the end of the parameters
				cmd.parameters[strlen(cmd.parameters) - 1] = '\0';
				int money = atoi(cmd.parameters);

				// Check if money is multiple of 10
				if (money % 10 != 0) {
					response = create_err_pkg(ERROR9, getmoney, "");
					goto end;
				}

				// The sold is smaller than the money asked for
				if (money > my_bank.accounts[i].sold) {
					response = create_err_pkg(ERROR8, getmoney, "");
					goto end;
				}

				my_bank.accounts[i].sold -= money;
				char *answer = calloc(50, 1);
				sprintf(answer, "Suma %d retrasa cu succes\n", money);

				// Get money finished succesfuly
				response = create_err_pkg(NOERROR, getmoney, answer);
				goto end;
			}
		}
	}

	else if (cmd.type == putmoney) {

		for (int i = 0; i < my_bank.N; i++) {
			if (my_bank.accounts[i].sessionfd == fd) {

				// Remove the newline from the end of the parameters
				cmd.parameters[strlen(cmd.parameters) - 1] = '\0';
				
				double money;
				sscanf(cmd.parameters, "%lf", &money);

				my_bank.accounts[i].sold += money;
				char *answer = calloc(50, 1);
				sprintf(answer, "Suma depusa cu succes\n");

				// Get money finished succesfuly
				response = create_err_pkg(NOERROR, getmoney, answer);
				goto end;
			}
		}
	}

	else if (cmd.type == unlock) {

	}

	else if (cmd.type == quit) {

		for (int i = 0; i < my_bank.N; i++) {
			if (my_bank.accounts[i].sessionfd == fd) {
				// Disconnect the client
				my_bank.accounts[i].is_logged = false;
				my_bank.accounts[i].sessionfd = 0;
				break;
			}
		}

		response = create_err_pkg(ERROR10, quit, "");
	}

end:
	return response;
}

void start_server(char *argv[]) {

	int port_server = atoi(argv[1]);

	// Open the socket for the TCP connection
	int atmfd;
	atmfd = socket(AF_INET, SOCK_STREAM, 0);
	if (atmfd < 0) {
		printf("Error at opening ATM socket\n");
		return;
	}

	// Create the structure of the server address
	struct sockaddr_in serv_addr_in;
	memset((char*) &serv_addr_in, 0, sizeof(serv_addr_in));
	serv_addr_in.sin_family = AF_INET;
	serv_addr_in.sin_port = htons(port_server);
	serv_addr_in.sin_addr.s_addr = INADDR_ANY; // the local address
	struct sockaddr *serv_addr = (struct sockaddr*) &serv_addr_in;

	// Create the bind between the local host and socket
	if (bind(atmfd, serv_addr, sizeof(*serv_addr)) == -1) {
		printf("aaa\n");
		printf("Error at binding\n");
		return;
	}

	// Create the socket that listens for new connections
	if (listen(atmfd, MAX_CLIENTS) == -1) {
		printf("Error at listening\n");
		return;
	}

	// Open the socket for the UDP connection
	int unlockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (unlockfd < 0) {
		printf("Error at opening UNLOCK socket\n");
		return;
	}

	// Bind the socket
	if (bind(unlockfd, serv_addr, sizeof(*serv_addr)) != 0) {
		printf("Error at binding\n");
		return;
	}

	fd_set read_fds;		// The set of reading file descriptors
	fd_set tmp_fds;			// A temporary set of file descriptors
	int fdmax;				// The biggest file descriptor in read_fds
	int newsockfd;			// The file descriptor of new connections
	char buffer[BUFLEN];	// Buffer in which we store a new command

	// Clear the sets
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	// Add stdin, atmfd and unlockfd to the reading file descriptors
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(atmfd, &read_fds);
	FD_SET(unlockfd, &read_fds);
	fdmax = (atmfd > unlockfd) ? atmfd : unlockfd;

	err_pkg error;
	cmd_pkg command;

	while (true) {

		// Multiplex the input and select reading file descriptors
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			printf("Error at selecting a file descriptor\n");
			return;
		}

		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {

				// Write a command from the server
				if (i == STDIN_FILENO) {

					// Read the command
					reset(buffer);
					fscanf(stdin, "%s", buffer);

					// Check if the command is quit
					if (strncmp(buffer, "quit", 5) == 0) {

						// Send messages and close all clients
						printf("Sending quit command to clients\n");
						for (int j = 3; j <= fdmax; j++) {
							if (FD_ISSET(j, &read_fds) && j != atmfd &&
									j != unlockfd) {
								error.size_of_package = 0;
								if (send(j, &error, PKGLEN, 0) == -1) {
									printf("Error at sending quit\n");
								}
								FD_CLR(j, &read_fds);
								close(j);
							}
						}

						// Close the server
						printf("Closing server...\n");
						FD_CLR(atmfd, &read_fds);
						close(atmfd);
						FD_CLR(unlockfd, &read_fds);
						close(unlockfd);
						return;
					}
				}

				// A new client wants to connect to the server (TCP)
				else if (i == atmfd) {

					// Accept the new client
					if ((newsockfd = accept(atmfd, NULL, NULL)) < 0) {
						printf("Error at accepting a new connection\n");
						return;
					}

					// Add the new client in the list of file descriptors
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) {
						fdmax = newsockfd;
					}

					printf("A new client connected TCP [%d]\n", newsockfd);
				}

				// A client sent something to the server (UDP)
				else if (i == unlockfd) {



				}

				// A client sent something to the server
				else {

					// Receive the command
					if (recv(i, &command, PKGLEN, 0) < 0) {
						printf("Error at receiving command\n");
        				return;
					}

					printf("Command TYPE %d received from client [%d]\n",
							command.type, i);

					// Get the error response of the command
					error = execute(command, i);

					// Send the response to the client
					if (error.type != ERROR10) {
						if (send(i, &error, PKGLEN, 0) < 0) {
	           				printf("Error at sending response\n");
				            return;
				        }
					} else {
						// A client wants to disconnect
						printf("A client disconnected TCP [%d]\n", i);
						FD_CLR(i, &read_fds);
						close(i);
					}
				}
			}
		}
	}
}

// Free allocated memory
void free_memory() {
	free(my_bank.accounts);
}