/********************************************************************
* FILENAME : libcommon.h
* AUTHOR : Tudor Paraschivescu
********************************************************************/

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Define a boolean type
typedef int bool;
#define true 1
#define false 0

// Define the maximum length of a buffer (command)
#define BUFLEN 255

// Define the maximum length of a package
#define PKGLEN 263

// Define the type of commands a client can send to the server
typedef enum commands {
    login = 0,
    logout = 1,
    listsold = 2,
    getmoney = 3,
    putmoney = 4,
    unlock = 5,
    quit = 6
} command_type;

// Define the set of errors that can occur
typedef enum errors {
	NOERROR = 0,
	ERROR1 = -1,
	ERROR2 = -2,
	ERROR3 = -3,
	ERROR4 = -4,
	ERROR5 = -5,
	ERROR6 = -6,
	ERROR7 = -7,
	ERROR8 = -8,
	ERROR9 = -9,
	ERROR10 = -10
} error_type;

#define SIZE_OF_EMPTY_CMDPKG 8

// Define the structure of a package containing a command
typedef struct __attribute__((__packed__)) {
	int size_of_package;			// the total size of the package
	command_type type;				// the type of the command
	char parameters[BUFLEN];		// the paramaters of the command
} cmd_pkg;							// Sent client -> server

#define SIZE_OF_EMPTY_ERRPKG 12

// Define the structure of a package containing an error or a message
// Convention: if size_of_package = 0 -> a quit command was sent to the client
typedef struct __attribute__((__packed__)) {
	int size_of_package;			// the total size of the package
	error_type type;				// the type of the error
	command_type answer_to;			// the type of command which in answered
	char message[BUFLEN];			// the message, if no error occurres
} err_pkg;							// Sent server -> client

void reset(char *buffer) {
    memset(buffer, 0, BUFLEN);
}

void reset_of(char *buffer, int len) {
    memset(buffer, 0, len);
}