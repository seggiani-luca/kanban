#include "net/net.h"
#include "../shared/command/command.h"
#include "../shared/card/card.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define HELLO_MSG "HELLO richiedo registrazione al server\n"
#define QUIT_MSG "QUIT mi deregistro dal server\n"
#define ACK_MSG_0 "ACK"
#define ACK_MSG_1 " ho processato la card\n"

/*
 * Chiude la connessione e il socket
 */
void quit() {
	send_net_req(QUIT_MSG);
	
	// aspetta risposta
	recv_net_res();

	// ignora risposta
	
	close_net();
}

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
	quit();
	exit(0);
}

// realizza un ciclo di gestione di una card, che comprende:
// - ricevere card e user list
// - aspettare un tempo casuale
// - inviare l'ack card
void handle_card() {
	recv_net_res();
	if(get_cmd_type(r_argv[0]) != HANDLE_CARD) {
		printf("Risposta server inaspettata, aspettavo HANDLE_CARD\n");
		return;
	}
	if(r_argc < 3) {
		printf("Troppi pochi argomenti per HANDLE_CARD");
		return;
	}

	card_id card_id = atoi(r_argv[1]);
		
	recv_net_res();
	if(get_cmd_type(r_argv[0]) != SEND_USER_LIST) {
		printf("Risposta server inaspettata, aspettavo SEND_USER_LIST\n");
		return;
	}

	const char* ack_mess; // TODO: genera, conviene riportare func. da core (creare messaggi con prepare_mess o quel che era...)
	send_net_req(ack_mess);

	recv_net_res();
	if(get_cmd_type(r_argv[0]) != OK) {
		printf("Risposta server a ACK CARD inaspettata\n");
		return;
	}
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Troppi pochi argomenti\n");
		return 1;
	}

	signal(SIGINT, int_handler);

	// configura connessione
	int port = atoi(argv[1]);
	if(configure_net(port) < 0) return 1;

	// registrati
	send_net_req(HELLO_MSG);
	
	// aspetta risposta
	recv_net_res();
	if(get_cmd_type(r_argv[0]) != OK) {
		printf("Risposta server a HELLO inaspettata\n");
		quit();
	}

	while(1) {
		handle_card();
	}

	quit();
	return 0;
}
