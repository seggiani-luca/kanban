#include "core.h"
#include "../net/net.h"										// gestione di rete client
#include "../../shared/command/command.h"	// tipo cmd
#include <stdio.h>												// printf
#include <stdlib.h>												// utilità
#include <string.h>												// utilità stringa

// ==== FUNZIONI HELPER ====

/*
 * Helper che:
 * - ottiene una risposta
 * - valuta se la risposta ottenuta combacia con quella attesa
 * - eventualmente restituisce la risposta
 *
 * Il valore di errore qui e nelle funzioni sotto è:
 * -1: se la terminazione è data dal protocollo, e il socket è sempre vivo
 * -2: se si hanno errori di lettura dal socket (si assume chiuso)
 */
int get_rep(cmd_type exp, cmd* got) {
	if(recv_cmd(got) < 0) {
		printf("[%d]\t: Errore di lettura dal socket\n", port);
		return -2;
	}

	if(got->typ != exp) {
		printf("[%d]\t: Server ha risposto %s, atteso %s\n", port, 
				typ_to_lit(got->typ), typ_to_lit(exp));
		return -1;
	}

	return 0;
}

/*
 * Helper che ottiene l'OK dal server
 */
int get_ack() {
	cmd rep = {0};
	return get_rep(OK, &rep); 
}

/*
 * Helper che ottiene la lista utenti dal server (non effettua la richiesta)
 */
int get_user_list(int clients[MAX_CLIENTS], int* num_clients) {
	// ottieni lista client 
	cmd cl_push = {0};
	if(get_rep(SEND_USER_LIST, &cl_push) < 0) return -1;
	
	printf("[%d]\t: Ho ottenuto la lista di client: ", port);

	// leggi la lista client
	*num_clients = 0;
	for(int i = 0; i < get_argc(&cl_push); i++) {
		int cl = atoi(cl_push.args[i]);
		if(cl != 0) {
			clients[(*num_clients)++] = cl;
			printf("%d ", cl);
		}
	}

	printf("\n");
	return 0;
}

// ==== RICHIESTE CLIENT ====

int create_card(const card* c) {
	printf("[%d]\t: Richiedo la creazione di una card al server\n", port);

	// invia la card
	char id_str[6];
	snprintf(id_str, 6, "%d", c->id);
	
	cmd cm = {
		.typ = CREATE_CARD,
		.args = { id_str, "TO_DO", c->desc, "richiedo la creazione di una card" }
	};
	send_cmd(&cm);
	
	// attendi ack
	return get_ack();
}

int get_card(card* c, int clients[MAX_CLIENTS], int* num_clients) {
	printf("[%d]\t: Cerco di ottenere una nuova card\n", port);
	
	// ottieni card 
	cmd c_push = {0};

	int ret = get_rep(HANDLE_CARD, &c_push);
	if(ret < 0) return ret;
	
	// leggi card
	c->id = atoi(c_push.args[0]);
	strncpy(c->desc, c_push.args[1], CARD_DESC_LEN - 1);
	c->desc[CARD_DESC_LEN - 1] = '\0';
	
	printf("[%d]\t: Ho ottenuto la card %d\n", port, c->id);

	// ottieni lista client
	ret = get_user_list(clients, num_clients);
	if(ret < 0) return ret;
	
	// fai l'ack
	printf("[%d]\t: Faccio ack per la card %d\n", port, c->id);
	
	char id_str[6];
	snprintf(id_str, 6, "%d", c->id);

	cmd ack = {
		.typ = ACK_CARD,
		.args = { id_str, "effettuo l'ack per la card ottenuta" }
	};
	send_cmd(&ack);

	return 0;
}

int hello() {
	printf("[%d]\t: Richiedo la mia registrazione al server\n", port);

	// richiedi registrazione
	cmd cm = {
		.typ = HELLO,
		.args = { "richiedo la mia registrazione" }
	};
	send_cmd(&cm);

	// attendi ack
	return get_ack();
}

int quit() {
	printf("[%d]\t: Richiedo la mia deregistrazione al server\n", port);
	
	// richiedi deregistrazione
	cmd cm = {
		.typ = QUIT,
		.args = { "richiedo la mia deregistrazione" }
	};
	send_cmd(&cm);

	// attendi ack
	return get_ack();
} 

int request_user_list(int clients[MAX_CLIENTS], int* num_clients) {
	printf("[%d]\t: Richiedo la lista dei client\n", port);
	
	// richiedi lista client 
	cmd cm = {
		.typ = REQUEST_USER_LIST,
		.args = { "richiedo la lista dei client" }
	};
	send_cmd(&cm);

	// ottieni lista client
	return get_user_list(clients, num_clients);
}

int card_done() {
	printf("[%d]\t: Segnalo di aver terminato di processare al server\n", port);
	
	// segnala di aver terminato 
	cmd cm = {
		.typ = CARD_DONE,
		.args = { "ho terminato di processare la mia card" }
	};
	send_cmd(&cm);

	// attendi ack
	return get_ack();
}
