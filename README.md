# atm-simulator
An ATM application based on sockets with TCP and UDP connections

# Project Structure
* server and client sources
* common library libcommon.h for both server and client
* libserver.h and libclient.h libraries
  
# Server
* check arguments
* read and store the input data
* start the server and wait for connections

# Logica client
* check arguments
* create TCP and UDP connections
* create log file
* send commands to the server

# Implementare
## SERVER
* create_err_pkg -> create packages that will be transmitted
* read_users_file -> read and store the input data
	- print_bank -> printarea tuturor informatiilor conturilor din banca
	- execute -> executa actiunea unei comenzi si intoarce eroarea/raspunsul
	- start_server -> creaza conexiunile de pe canalele TCP si UDP si asteapta
		conexiuni noi, in limita a MAX_CLIENTS
-------------------------------------------------------------------------------
CLIENT:
	- create_cmd_pkg -> creaza pachetele ce vor fi trimise
	- connect_to_atm -> crearea conexiunii TCP
	- connect_to_unlock -> crearea conexiunii UDP
	- create_log_file -> crearea fisierului de log
	- print_error -> printarea unei erori/raspuns primit de pe o conexiune
		TCP/UDP sau o eroare interna
	- get_parameters -> returneaza parametrii unei comenzi
	- check_parameters -> verifica daca parametrii unei comenzi sunt valizi
	- send_command -> creaza si trimite o comanda catre server pe o anumita
		conexiune
	- receive_error -> primirea si evaluarea unei erori
	- talk_to_server -> discutia continua cu serverul, realizata prin
		trimiterea si primirea de pachete
-------------------------------------------------------------------------------
