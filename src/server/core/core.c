#include "core.h"
#include "../net/net.h"										// gestione di rete server
#include "../../shared/command/command.h"	// tipo cmd
#include "../../shared/core_const.h"			// costanti core
#include <string.h>												// utilità stringa
#include <stdlib.h>												// utilità
#include <stdio.h>												// printf

// ==== STATO SISTEMA ====

/*
 * Messaggio di errore corrente, impostato durante la gestione dei comandi 
 */
const char* err_mess;

// ==== GESTIONE CLIENT ====

/*
 * Rappresenta lo stato del client nella ricezione delle carte
 */
typedef enum {
	IDLE,
	SENT_CARD,
	BUSY,
} client_sts;

/*
 * Rappresenta un client, identificato da:
 * - id (se è 0 il client è nullo)
 * - stato
 * - puntatore alla card che sta gestendo (se è NULL sta aspettando una card)
 */
typedef struct {
	client_id id;
	client_sts sts;
	card* handling;
} client;

/*
 * Vettore client registrati 
 */
client clients[MAX_CLIENTS] = {0};

/*
 * Client dell'admin
 */
client admin_client = {0};

/*
 * Registra un nuovo client nel vettore client registrati e restituisce il suo
 * indice
 */
int register_client(client_id cl) {
	// controlla che l'id sia valido
	if(cl == 0) {
		err_mess = "id utente non valido";
		return -1;
	}

	// cerca uno slot libero per il client
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i].id == 0) {
			// registra il client
			clients[i].id = cl;
			clients[i].sts = IDLE;
			return i;
		}
	}

	// se sei qui non ci sono slot liberi
	err_mess = "spazio esaurito";
	return -1; 
}

// dichiarate in anticipo per unregister_client()
void set_timestamp(card* c);
int move_card(card_id id, col_id to);

/*
 * Deregistra un client dal vettore client registrati
 */
int unregister_client(client* cl) {
	// deregistra client impostando id a 0
	cl->id = 0;

	// se possedeva una card, liberala
	if(cl->sts != IDLE) {
		card* c = cl->handling;
		c->user = 0;
		
		// aggiorna timestamp
		set_timestamp(c);
		
		// restituisci errore se la card posseduta non viene trovata
		return move_card(c->id, TO_DO);
	}

	return 0;
}

/*
 * Trova il client con dato id nel vettore client registrati. Restituisce NULL 
 * se non lo trova
 */
client* find_client(client_id cl) {
	// l'admin usa un client speciale
	if(cl == 0) return &admin_client;

	// scansiona per id
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i].id == cl) {
			return &clients[i];
		}
	}

	// se sei qui non hai trovato nulla
	err_mess = "client non trovato";
	return NULL;
}

// ==== GESTIONE CARD ====

/*
 * Tabella delle colonne. L'ordine è column-major, dove columns[i][j] è la 
 * j-esima card della i-esima colonna
 */
card* columns[NUM_COLS][MAX_CARDS_PER_COL] = {0};

/*
 * Inserisce una card in una determinata colonna
 */
int insert_card(card* c, col_id where) {
	// ottieni la colonna
	card** column = columns[where];

	// cerca uno slot libero nella colonna per la card
	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] == NULL) {
			// inserisci la card
			column[i] = c;
			return 0;
		}
	}

	// se sei qui non ci sono slot liberi
	err_mess = "spazio esaurito";
	return -1;
}

/*
 * Trova l'indice di una card in una determinata colonna
 */
int find_card(card_id id, col_id* where) {
	// scansiona per colonna
	for(*where = 0; *where < NUM_COLS; (*where)++) {
		card** column = columns[*where];

		// scansiona card nella colonna per id
		for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
			if(column[i] != NULL && column[i]->id == id) {
				return i;
			}
		}
	}

	// se sei qui non hai trovato nulla
	err_mess = "card non trovata";
	return -1;
}

/*
 * Trova l'indice della prossima card da gestire in TO_DO 
 */
int get_next_card() {
	// ottieni la colonna TO_DO
	card** column = columns[TO_DO];

	// scansiona card nella colonna per user nullo
	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] != NULL && column[i]->user == 0) {
			return i;
		}
	}

	// se sei qui non hai trovato nulla
	err_mess = "colonna vuota";
	return -1;
}

// ==== FUNZIONI HELPER ====

/*
 * Imposta il timestamp di una card alla data corrente
 */
void set_timestamp(card* c) {
	time_t now = time(NULL);
	c->timestamp = *localtime(&now);
}

/*
 * Helper che invia una card ad un dato client. In particolare, si inviano id
 * della carta e descrizione
 */
void send_card(client* cl, card* c) {
	// prepara risposta 
	char id_str[6];
	snprintf(id_str, 6, "%d", c->id);
	
	cmd cm = {
		.typ = HANDLE_CARD,
		.args = { id_str, c->desc, "fornita card da processare, invio "
			"SEND_USER_LIST e aspetto ACK_CARD" }
	};

	// invia risposta 
	send_cmd(cl->id, &cm);

	// registra card gestita in utente
	cl->sts = SENT_CARD;
	cl->handling = c;

	// aggiorna card con utente
	c->user = cl->id;
}

// dichiarata in anticipo per push_card()
int request_user_list(client* cl);

/*
 * Helper di push_cards che invia una card ad un singolo client. Viene usata da
 * hello() e card_done() per provare ad inviare una card ad un singolo client
 */
int push_card(client* cl) {
	// ottieni prossima card da gestire
	int idx = get_next_card();
	if(idx < 0) return -1;
	card* c = columns[TO_DO][idx];

	// aggiorna timestamp
	set_timestamp(c);
	
	// invia la card
	send_card(cl, c);
	
	// invia anche lista utenti
	request_user_list(cl);

	return 0;
}

/*
 * L’utente conferma di aver ricevuto l’assegnazione dell’attività, così la 
 * lavagna può spostare la card nella colonna DOING 
 */
int ack_card(client* cl, card_id id) {
	// controlla che si aspettasse effettivamente una risposta dal client
	if(cl->sts != SENT_CARD) {
		err_mess = "non in attesa di risposta da questo client";
		return -1;
	}

	// controlla che il client ricordi bene la sua card
	if(cl->handling->id != id) {
		err_mess = "card non assegnata";
		return -1;
	}

	// ottieni card
	col_id where;
	int idx = find_card(id, &where);
	if(idx < 0) return -1;

	// controlla che la card sia in TO_DO
	if(where != TO_DO) {
		err_mess = "card già confermata";
		return -1;
	}

	card* c = columns[TO_DO][idx];

	// versione ridotta di move_card, conosciamo già l'indice
	if(insert_card(c, DOING)< 0) return -1;
	columns[TO_DO][idx] = NULL;

	// imposta client come BUSY
	cl->sts = BUSY;

	return 0;
}

/*
 * La lavagna invia, in ordine di porta, ad ogni utente connesso una card della
 * colonna TO_DO. Questo comando, oltre alla card, include la lista delle porte
 * degli utenti presenti (escluso il destinatario del messaggio), e il numero 
 * degli utenti presenti
 */
void push_cards() {
	// itera sui client
	for(int i = 0; i < MAX_CLIENTS; i++) {
		client* cl = &clients[i];

		// se non è BUSY, prova a inviare una card
		if(cl->id != 0 && cl->sts != BUSY) {
			if(push_card(cl) < 0) return; // quando finiscono le card, esci
		}
	}
}

/*
 * Sposta una card da una colonna ad un altra 
 */
int move_card(card_id id, col_id to) {
	// trova la card da spostare
	col_id from;
	int idx = find_card(id, &from);
	if(idx < 0) return -1;
	
	// aggiorna timestamp
	set_timestamp(columns[from][idx]);

	// sposta la card
	if(insert_card(columns[from][idx], to) < 0) return -1;
	columns[from][idx] = NULL;
	
	return 0;
}

// ==== GESTORI COMANDI ====

/*
 * Un utente comunica alla lavagna la creazione di una nuova card, assegnandole
 * id, colonna e testo attività
 */
int create_card(client* cl, card_id id, col_id col, const char* desc) {
	// controlla che l'id sia valido
	if(id == 0) {
		err_mess = "id card non valido";
		return -1;
	}

	// alloca la card
	card* c = alloc_card();
	if(c == NULL) {
		err_mess = "pool card esaurita";
		return -1;
	}
	
	// inizializza card
	c->id = id;

	// copia quanto della stringa effettivamente entra nella descrizione
	strncpy(c->desc, desc, CARD_DESC_LEN - 1);
	c->desc[CARD_DESC_LEN - 1] = '\0';

	// imposta timestamp
	set_timestamp(c);

	// inserisci card nella colonna richiesta
	if(insert_card(c, col) < 0) {
		free_card(c); // libera la card allocata
		return -1;
	}

	// rispondi al client
	cmd cm = {
		.typ = OK,
		.args = { "creata card richiesta" }
	}	;
	send_cmd(cl->id, &cm);
	
	// qui chiamiamo la routine push_cards, nel caso ci fossero client in attesa 
	push_cards();

	return 0;
}

/*
 * Un utente notifica la sua presenza (registrazione) alla lavagna
 */
int hello(client_id cl) {
	// prova a registrare il client
	int ret = register_client(cl);

	if(ret >= 0) {
		// client registrato, rispondi
		cmd cm = {
			.typ = OK,
			.args = { "client registrato, seguira' una card da processare" }
		};
		send_cmd(cl, &cm);
		
		// invia la prima card
		push_card(&clients[ret]);
		
		return 0;
	} else return -1;
}

/*
 * Un utente notifica la sua uscita alla lavagna. Se l’utente aveva una card in
 * DOING, questa andrà riportata in TO_DO per essere riassegnata in seguito
 */
int quit(client* cl) {
	// mantieni id del client
	int id = cl->id;
	
	// prova a deregistrare
	int ret = unregister_client(cl);
	if(ret >= 0) {
		// client deregistrato, rispondi
		cmd cm = {
			.typ = OK,
			.args = { "client deregistrato, arrivederci" }
		};
		send_cmd(id, &cm);
	}

	return ret;
}

int show_lavagna() {
	// mostra le card
	for(int j = 0; j < MAX_CARDS_PER_COL; j++) {

		// 4 linee per card
		for(int l = 0; l < 3; l++) {
			
			// prima colonne, poi righe
			for(int i = 0; i < NUM_COLS; i++) {
				card* c = columns[i][j];

				if(c != NULL) {
					switch(l) {
						case 0: printf("id: %-11d user: %-10d", c->id, c->user); break;
						case 1: {
							// passa il timestamp a stringa con strftime
							char tm_buf[64];
							strftime(tm_buf, sizeof(tm_buf), "%Y-%m-%d %H:%M:%S", 
									&c->timestamp);
							
							printf("time: %-26s", tm_buf); 
							break;
						}
						case 2: printf("%-32s", c->desc); break;
					}	
				} else {
					// tot. 3 righe vuote
					printf("%-32s", "");
				}
			}

			printf("\n");
		}

		printf("\n");
	}

	return 0;
}

int show_clients() {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		client* cl = &clients[i];
		if(cl->id == 0) continue;

		printf("%d: ", cl->id);
		switch(cl->sts) {
			case IDLE: printf("IDLE"); break;
			case SENT_CARD: printf("SENT_CARD "); break;
			case BUSY: printf("BUSY "); break;
		}

		if(cl->sts != IDLE) {
			printf("%d", cl->handling->id);
		}

		printf("\t");
	}
	
	printf("\n");
	
	return 0;
}

/*
 * La lavagna restituisce la lista delle porte degli utenti registrati ad un
 * utente
 */
int request_user_list(client* cl) {
	// prepara buffer per id client
	char id_strs[MAX_CLIENTS][6]; // 5 caratteri + terminatore per 65535
	
	// prepara risposta
	cmd cm = {
		.typ = SEND_USER_LIST
		// verrà popolato in seguito
	};
	
	int argc = 0;	
	// itera sui client 
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(cl == &clients[i]) continue;

		// solo client non nulli
		client_id o_cl = clients[i].id;
		if(o_cl != 0) {
			// copia id client nel buffer
			snprintf(id_strs[argc], 6, "%d", o_cl);

			// usa come argomento
			cm.args[argc] = id_strs[argc];
			argc++;
		}
	}
	
	// metti commento nell'ultimo argomento
	cm.args[argc++] = "fornita lista utenti registrati";

	// invia risposta
	send_cmd(cl->id, &cm);

	return 0;
}

/*
 * L'utente al quale è assegnata la card, dopo aver ricevuto la review da tutti
 * gli altri utenti, comunica che l’attività nella card è terminata
 */
int card_done(client* cl) {
	// controlla che il client stia effettivamente processando una card
	if(cl->sts != BUSY) {
		err_mess = "questo client non sta processando una card";
		return -1;
	}

	// mantieni l'id della card gestita
	int id = cl->handling->id;
	
	// sposta la card in DONE
	if(move_card(id, DONE) < 0) return -1;
	
	// rispondi al client
	cmd cm = {
		.typ = OK,
		.args = { "card processata, ne seguira' un'altra" }
	};
	send_cmd(cl->id, &cm);

	// fornisci una nuova card
	cl->sts = IDLE;
	cl->handling = NULL;
	push_card(cl);

	return 0;
}

// ==== INTERPRETAZIONE COMANDI ==== 

void exec_command(client_id cl_id, const cmd* cm) {
	// ottieni puntatore client
	client* cl = NULL;

	// non ci saranno dati relativi al client prima della sua registrazione 
	if(cm->typ != HELLO)	{
		cl = find_client(cl_id);

		if(cl == NULL) {
			// se nullo, il client non si è registrato
			cmd cm = {
				.typ = ERR,
				.args = { "registrarsi prima di fare richieste" }
			};
			send_cmd(cl_id, &cm);
			return;
		}
	}

	// prepara messaggio di errore per troppi pochi argomenti
	cmd arg_err = {
		.typ = ERR,
		.args = { "troppi pochi argomenti" }
	};

	int ret;
	switch (cm->typ) {
		// client -> server
		case CREATE_CARD:
			if(get_argc(cm) < 3) {
				send_cmd(cl_id, &arg_err);
				return;
			}
			ret = create_card(cl, atoi(cm->args[0]), atoc(cm->args[1]), cm->args[2]);
			break;

		case HELLO: ret = hello(cl_id); break;
		case QUIT: ret = quit(cl); break;
		case ACK_CARD:
			if(get_argc(cm) < 1) {
				send_cmd(cl_id, &arg_err);
				return;
			}
			ret = ack_card(cl, atoi(cm->args[0]));
			break;

		case REQUEST_USER_LIST: ret = request_user_list(cl); break;
		case CARD_DONE: ret = card_done(cl); break;
		
		// console -> server
		case SHOW_LAVAGNA: ret = show_lavagna(); break;
		case SHOW_CLIENTS: ret = show_clients(); break;
		case MOVE_CARD:
			if(get_argc(cm) < 2) {
				send_cmd(cl_id, &arg_err);
				return;
			}
			ret = move_card(atoi(cm->args[0]), atoc(cm->args[1]));
			break;

		default: {
			// errore comando invalido
			cmd val_err = {
				.typ = ERR,
				.args = { "comando non valido" }
			};
			send_cmd(cl_id, &val_err);
			return;
		}
	}

	if(ret < 0) {
		// c'è stato errore, restituiscilo
		cmd com_err = {
			.typ = ERR,
			.args = { err_mess }
		};
		send_cmd(cl_id, &com_err);
	}
}
