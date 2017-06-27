# atm-simulator
An ATM application based on sockets with TCP and UDP connections

# Project Structure
* server and client sources
* common library libcommon.h for both server and client
* libserver.h and libclient.h libraries
  
# Server structure
* check arguments
* read and store the input data
* start the server and wait for connections

# Client structure
* check arguments
* create TCP and UDP connections
* create log file
* send commands to the server

# Implementation
## Server Library
* create_err_pkg -> create packages that will be transmitted
* read_users_file -> read and store the input data
* print_bank -> print the info of all the accounts in the bank
* execute -> execute a comman and return the error/answer
* start_server -> create TCP and UDP connections and wait for new connections

## Client Library
* create_cmd_pkg -> create packages that will be transmitted
* connect_to_atm -> create TCP connection
* connect_to_unlock -> create UDP connection
* create_log_file -> create log file
* print_error -> print the error/answer received on a connection or an internal error
* get_parameters -> return the arguments of a command
* check_parameters -> check if the arguments of a command are valid
* send_command -> send a valid command to the server
* receive_error -> receive and evaluate an error/answer
* talk_to_server -> send commands to the server
