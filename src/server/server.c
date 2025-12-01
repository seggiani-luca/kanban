#include "core/core.h"
#include "cons/cons.h"
#include <stdio.h>

int main() {
	for(;;) {
		// inizializza contenitori dati comando
		client_id cl;
		int argc;
		char* argv[MAX_CONS_ARGS];

		// imposta callback di risposta
		set_reply_callback(reply_cons_cmd);

		// ottieni comando da console
		recv_cons_cmd(&cl, &argc, argv);

		// esegui comando
		int ret = parse_command(cl, argc, argv);
		switch(ret) {
			case -1: printf("Errore esecuzione comando\n"); break;
			case -2: printf("Comando vuoto\n"); break;
			case -3: printf("Comando non valido\n"); break;
			case -4: printf("Troppi pochi argomenti\n");
		}
	}

	return 0;
}
