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
		parse_command(cl, argc, argv);
	}

	return 0;
}
