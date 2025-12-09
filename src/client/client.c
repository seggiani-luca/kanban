#include "net/net.h"		// gestione di rete client
#include "core/core.h"	// logica client
#include <signal.h>			// segnali
#include <stdlib.h>			// exit
#include <stdio.h>			// printf
#include <unistd.h>			// sleep

/*
 * Il tempo massimo di attesa del client
 */
#define MAX_WAIT 5

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
	printf(" -> [%d]\t: Termino chiudendo la connessione\n", port);
	quit();
	close_net();
	exit(0);
}

/*
 * Helper che aspetta un tempo casuale
 */
void rand_wait() {
	srand(getpid());
	int r = rand() % MAX_WAIT;

	sleep(r);
}

/*
 * Implementa il loop di esecuzione del client
 */
int client_loop() {
	while(1) {
		// aspetta un tempo casuale
		rand_wait();
		
		// ottieni una card
		card c = {0};
		int clients[MAX_CLIENTS];
		int num_clients;

		int ret = get_card(&c, clients, &num_clients);
		if(ret < 0) return ret;
		
		// aspetta un tempo casuale
		rand_wait();

		// elabora la card
		ret = card_done();
		if(ret < 0) return ret;	
	}

	return 0;
}

int main(int argc, char* argv[]) {
	// aspetta un po' di tempo per lasciare che gli altri client vengano avviati
	sleep(1);

	// controlla argomenti
	if(argc < 2) {
		printf("Troppi pochi argomenti\n");
		return 1;
	}

	// imposta segnale SIGINT
	signal(SIGINT, int_handler);

	// configura connessione
	port = atoi(argv[1]);
	if(configure_net() < 0) return 1;

	// registrati al server
	int ret = hello();
	
	if(ret >= 0) {
		// inizia a fare richieste
		ret = client_loop();
	}

	if(ret > -2) {
		// chiudi la connessione
		quit();
	} else {
		printf("[%d]\t: Attenzione: termino senza chiudere la connessione\n", 
				port);
	}
	close_net();
}
