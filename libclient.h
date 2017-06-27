/********************************************************************
* FILENAME : libclient.h
* AUTHOR : Tudor Paraschivescu
********************************************************************/

#include "libcommon.h"

// File descriptors
int atmfd;          // The TCP connection of the ATM
int unlockfd;       // The UDP connection of the UNLOCK

FILE *log_stream;   // Stream of the log file

// Boolean that knows if a session is already opened or not
bool is_opened_session;

char *active_user;      // The user logged in the client

struct sockaddr_in serv_addr;       // The address of the server
struct sockaddr_in my_sockaddr;     // My address

fd_set read_fds;        // The set of reading file descriptors
int fdmax;              // The biggest file descriptor in read_fds


// Create a new command package
cmd_pkg create_cmd_pkg(command_type type, char *parameters) {
    cmd_pkg new_cmd_pkg;
    
    new_cmd_pkg.size_of_package =
            sizeof(int) + sizeof(command_type) + strlen(parameters) + 1;
    new_cmd_pkg.type = type;
    strcpy(new_cmd_pkg.parameters, parameters);

    return new_cmd_pkg;
}

// Try to connect to the given port of the given server (TCP)
bool connect_to_atm(char *argv[]) {

	char *IP_server = argv[1];
	char *port_server = argv[2];

	// Open the socket for the connection with the server
	atmfd = socket(AF_INET, SOCK_STREAM, 0);
    if (atmfd < 0) {
        return false;
    }
    
    // Create the structure of the server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port_server));
    inet_aton(IP_server, &serv_addr.sin_addr);
    
    // Connect to the server
    if (connect(atmfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        return false;
    }

    return true;
}

// Try to connect to the given port of the given server (UDP)
bool connect_to_unlock(char *argv[]) {

    char *IP_server = argv[1];
    char *port_server = argv[2];

    // Open the socket for the connection with the server
    unlockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (unlockfd < 0) {
        return false;
    }

    // Create the structure of my address
    my_sockaddr.sin_family = AF_INET;
    my_sockaddr.sin_port = 0;
    my_sockaddr.sin_addr.s_addr = INADDR_ANY;

    // Associate a port for the UDP connection
    if (bind(unlockfd, (struct sockaddr*) &my_sockaddr,
            sizeof(my_sockaddr)) == -1) {
        return false;
    }

    return true;
}

// Try to create a stream for the log file
bool create_log_file() {

    // Create the name of the log file
    pid_t pid = getpid();
    char *log_file_name = calloc(20, 1);
    sprintf(log_file_name, "client-%u.log", pid);

    // Open the stream
    log_stream = fopen(log_file_name, "w");
    free(log_file_name);
    if (log_stream == NULL) {
        return false;
    }

    return true;
}

// Print a package got from the server
void print_error(err_pkg error, int file_descriptor) {

    char error_message[100];

    // Add the source of the message
    if (file_descriptor == atmfd) {
        strcpy(error_message, "ATM> ");
    } else if (file_descriptor == unlockfd) {
        strcpy(error_message, "UNLOCK> ");
    } else {
        strcpy(error_message, "");
    }

    // Add the message associated to the error code
    if (error.type == NOERROR) {
        strcat(error_message, error.message);
    } else if (error.type == ERROR1) {
        strcat(error_message, "-1 : Clientul nu este autentificat\n");
    } else if (error.type == ERROR2) {
        strcat(error_message, "-2 : Sesiune deja deschisa\n");
    } else if (error.type == ERROR3) {
        strcat(error_message, "-3 : Pin gresit\n");
    } else if (error.type == ERROR4) {
        strcat(error_message, "-4 : Numar card inexistent\n");
    } else if (error.type == ERROR5) {
        strcat(error_message, "-5 : Card blocat\n");
    } else if (error.type == ERROR6) {
        strcat(error_message, "-6 : Operatie esuata\n");
    } else if (error.type == ERROR7) {
        strcat(error_message, "-7 : Deblocare esuata\n");
    } else if (error.type == ERROR8) {
        strcat(error_message, "-8 : Fonduri insuficiente\n");
    } else if (error.type == ERROR9) {
        strcat(error_message, "-9 : Suma nu este multiplu de 10\n");
    } else if (error.type == ERROR10) {
        strcat(error_message, "-10 : Error\n");
    }

    // Print the message in the console and in the log file
    printf("%s", error_message);
    fprintf(log_stream, "%s", error_message);
}

// Get the string of parameters
char *get_parameters(char *command, command_type type) {

    if (type == login) {
        return &(command[6]);
    } else if (type == logout) {
        return &(command[6]);
    } else if (type == listsold) {
        return &(command[8]);
    } else if (type == getmoney) {
        return &(command[9]);
    } else if (type == putmoney) {
        return &(command[9]);
    } else if (type == unlock) {
        return &(command[6]);
    } else if (type == quit) {
        return &(command[4]);
    }
    return NULL;
}

// Check the parameters of a new command
bool check_parameters(char *command, command_type type) {

    char *parameters = get_parameters(command, type);

    if (type == login) {
        if (parameters[6] != ' ' || parameters[11] != '\n') {
            return false;
        }
    } else if (type == logout) {
        if (parameters[0] != '\n') {
            return false;
        }
    } else if (type == listsold) {
        if (parameters[0] != '\n') {
            return false;
        }
    } else if (type == getmoney) {
        // It is known that the number is an integer
    } else if (type == putmoney) {
        // It is known that the number is of type double
    } else if (type == unlock) {

    } else if (type == quit) {
        if (parameters[0] != '\n') {
            return false;
        }
    }

    return true;
}

// Create and send a command package to server (a certain file descriptor)
bool send_command(char *command) {

    cmd_pkg new_command;

    if (strncmp(command, "login ", 6) == 0) {
        // The command is login
        if (check_parameters(command, login) == true) {
            if (is_opened_session == false) {
                char *parameters = get_parameters(command, login);
                new_command = create_cmd_pkg(login, parameters);
                is_opened_session = true;
                strncpy(active_user, parameters, 6);
            } else {
                err_pkg error;
                error.type = ERROR2;
                print_error(error, 0);
                return true;
            }
        } else {
            return true;
        }
    } else if (strncmp(command, "logout", 6) == 0) {
        // The command is logout
        if (check_parameters(command, logout) == true) {
            if (is_opened_session == true) {
                new_command = create_cmd_pkg(logout, "");
                is_opened_session = false;
                reset_of(active_user, 7);
            } else {
                err_pkg error;
                error.type = ERROR1;
                print_error(error, 0);
                return true;
            }
        } else {
            return true;
        }

    } else if (strncmp(command, "listsold", 8) == 0) {
        // The command is listsold
        if (check_parameters(command, listsold) == true) {
            if (is_opened_session == true) {
                new_command = create_cmd_pkg(listsold, "");
            } else {
                err_pkg error;
                error.type = ERROR1;
                print_error(error, 0);
                return true;
            }
        } else {
            return true;
        }

    } else if (strncmp(command, "getmoney ", 9) == 0) {
        // The command is getmoney
        if (check_parameters(command, getmoney) == true) {
            if (is_opened_session == true) {
                char *parameters = get_parameters(command, getmoney);
                new_command = create_cmd_pkg(getmoney, parameters);
            } else {
                err_pkg error;
                error.type = ERROR1;
                print_error(error, 0);
                return true;
            }
        } else {
            return true;
        }

    } else if (strncmp(command, "putmoney ", 9) == 0) {
        // The command is putmoney
        if (check_parameters(command, putmoney) == true) {
            if (is_opened_session == true) {
                char *parameters = get_parameters(command, putmoney);
                new_command = create_cmd_pkg(putmoney, parameters);
            } else {
                err_pkg error;
                error.type = ERROR1;
                print_error(error, 0);
                return true;
            }
        } else {
            return true;
        }

    } else if (strncmp(command, "unlock", 6) == 0) {
        // The command is unlock
    
    } else if (strncmp(command, "quit", 4) == 0) {
        // The command is quit
        if (check_parameters(command, quit) == true) {
            new_command = create_cmd_pkg(quit, "");
        }
    } else {
        return true;
    }

    if (new_command.type != unlock) {
        // Send package on the TCP connection
        if (send(atmfd, &new_command, PKGLEN, 0) < 0) {
            printf("Error at sending command\n");
            return false;
        }
    } else {
        // Send package on the UDP connection
        if (sendto(unlockfd, &new_command, PKGLEN,
                0, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) {
            printf("Error at sending command\n");
            return false;
        }
    }

    if (new_command.type == quit) {
        // EXIT code
        return -1;
    }

    return true;
}

// Receive an error from the server
bool receive_error(int fd) {

    err_pkg error;

    if (fd == atmfd) {
        if (recv(fd, &error, PKGLEN, 0) < 0) {
            printf("Error at receiving package\n");
            return false;
        }
    } else if (fd == unlockfd) {
        if (recvfrom(unlockfd, &error, PKGLEN, 0, NULL, NULL)) {
            printf("Error at receiving package\n");
            return false;
        }
    }

    if (error.size_of_package == 0) {
        // Server sent a quit request
        FD_CLR(atmfd, &read_fds);
        FD_CLR(unlockfd, &read_fds);
        close(atmfd);
        close(unlockfd);
        return -1;

    } else if (error.answer_to == login) {
        // The last login is not valid
        if (error.type != NOERROR) {
            reset_of(active_user, 7);
            is_opened_session = false;
        }

    } else if (error.answer_to == logout) {
        // No changes must be done in the client

    } else if (error.answer_to == listsold) {
        // No changes must be done in the client

    } else if (error.answer_to == getmoney) {
        // No changes must be done in the client

    } else if (error.answer_to == putmoney) {
        // No changes must be done in the client

    } else if (error.answer_to == unlock) {

    } else if (error.answer_to == quit) {
        // No changes must be done in the client
    }

    print_error(error, fd);
    return true;
}

bool talk_to_server() {

    fd_set tmp_fds;         // A temporary set of file descriptors
    char buffer[BUFLEN];    // Buffer in which we store a new command

    // Clear the sets
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Add stdin,atmfd and unlockfd to the reading file descriptors
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(atmfd, &read_fds);
    FD_SET(unlockfd, &read_fds);
    fdmax = (atmfd > unlockfd) ? atmfd : unlockfd;

    // Before any user logs in, there is no session opened
    is_opened_session = false;

    // There is no active user
    active_user = calloc(7, sizeof(char));

    while (true) {

        // Multiplex the input and select reading file descriptors
        tmp_fds = read_fds;
        if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
            printf("Error at selecting a file descriptor\n");
            return false;
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmp_fds)) {

                // Write a command from the client
                if (i == STDIN_FILENO) {

                    // Read the command
                    reset(buffer);
                    fgets(buffer, BUFLEN, stdin);

                    // Send the command to server
                    int code = send_command(buffer);
                    if (code == false) {
                        return false;
                    } else if (code == -1) {
                        // EXIT procedure
                        err_pkg error;
                        error.type = NOERROR;
                        strcpy(error.message, "quit\n");
                        print_error(error, 0);
                        return true;
                    }
                }

                // The server sent a command
                else {

                    // Receive the error
                    reset(buffer);
                    int code = receive_error(i);
                    if (code == false) {
                        return false;
                    } else if (code == -1) {
                        // Server sent a quit request
                        err_pkg error;
                        error.type = NOERROR;
                        strcpy(error.message, "quit\n");
                        print_error(error, atmfd);
                        return true;
                    }
                }
            }
        }
    }
}