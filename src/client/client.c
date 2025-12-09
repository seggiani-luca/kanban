#include "net/net.h"		// gestione di rete client
#include "core/core.h"	// logica client
#include <signal.h>			// segnali
#include <stdlib.h>			// exit
#include <stdio.h>			// printf
#include <time.h>				// seme di srand
#include <unistd.h>			// sleep

/*
 * Il tempo massimo di attesa del client
 */
#define MAX_WAIT 5

/*
 * Il numero di card da gestire 
 */
#define NUM_CARDS 2

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
	printf("Termino chiudendo il socket\n");
	quit();
	close_net();
	exit(0);
}

/*
 * Implementa il loop di esecuzione del client
 */
int client_loop() {
	for(int i = 0; i < NUM_CARDS; i++) {
		// ottieni una card
		card c = {0};
		int clients[MAX_CLIENTS];
		int num_clients;

		int ret = get_card(&c, clients, &num_clients);
		if(ret < 0) return ret;
		
		// aspetta un tempo casuale
		srand(time(NULL));
		int r = rand() % MAX_WAIT;

		sleep(r);

		// elabora la card
		ret = card_done();
		if(ret < 0) return ret;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Troppi pochi argomenti\n");
		return 1;
	}

	// imposta segnale SIGINT
	signal(SIGINT, int_handler);

	// configura connessione
	int port = atoi(argv[1]);
	if(configure_net(port) < 0) return 1;

	// registrati al server
	int ret = hello();

	if(ret >= 0) {
		// inizia a fare richieste
		ret = client_loop();
	}

	if(ret > -2) {
		// chiudi la connessione
		int_handler();
	} else {
		printf("Attenzione: termino senza chiudere il socket\n");
	}
}
