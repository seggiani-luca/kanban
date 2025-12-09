#include "net/net.h"		// gestione di rete server
#include "core/core.h"	// logica server
#include <signal.h>			// segnali
#include <stdlib.h>			// exit
#include <stdio.h>			// printf

/*
 * Handler predisposto alla gestione di SIGINT per la chisura pulita della
 * connessione
 */
void int_handler() {
	printf(" -> "); // psicopatia
	close_net();
	exit(0);
}

int main() {
	// imposta segnale SIGINT
	signal(SIGINT, int_handler);
	
	// configura il socket di ascolto
	if(configure_net()) return 1;

	// mostra la lavagna
	show_lavagna();

	// mettiti in ascolto di richieste
	listen_net();

	return 0;
}
