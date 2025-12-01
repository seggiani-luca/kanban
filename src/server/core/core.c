#include "core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SEP_30 "------------------------------"

// ==== IMPLEMENTAZIONE INTERFACCIA ====

/*
 * Funzione di callback usata per rispondere alle richieste
 */
static reply_cback reply = NULL;

void set_reply_callback(reply_cback new_reply) {
	reply = new_reply;
}

// ==== STATO SISTEMA ====

/*
 * Numero massimo di utenti supportati
 */
#define MAX_CLIENTS 4

/*
 * Vettore client registrati 
 */
static client_id clients[MAX_CLIENTS] = {0};

/*
 * Registra un nuovo client nel vettore client registrati
 *
 * Valori di errore:
 * -1: id non valido
 * -2: spazio esaurito
 */
int register_client(client_id cl) {
	if(cl == 0) return -1; // id non valido

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] == 0) {
			clients[i] = cl;
			return 0;
		}
	}

	return -2; // spazio esaurito 
}

/*
 * Deegistra un client dal vettore client registrati
 *
 * Valori di errore:
 * -1: id non valido
 * -2: client non registrato 
 */
int unregister_client(client_id cl) {
	if(cl == 0) return -1; // id non valido

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] == cl) {
			clients[i] = 0;
			return 0;
		}
	}

	return -2; // client non registrato 
} 

/*
 * Numero massimo di card supportate per colonna
 */
#define MAX_CARDS_PER_COL 10

/*
 * Macro per il numero massimo di card supportate
 */
#define MAX_CARDS (NUM_COLS * MAX_CARDS_PER_COL)

/*
 * Pool di card presenti nel sistema
 */
static struct card cards[MAX_CARDS] = {0};

/*
 * Alloca una card dalla pool. Restituisce NULL se non ce ne sono libere
 */
struct card* alloc_card() {
	for(int i = 0; i < MAX_CARDS; i++) {
		if(cards[i].id == 0) {
			return &cards[i];
		}
	}

	return NULL;
}

/*
 * Dealloca una card nella pool. Non fa nulla se si fornisce NULL
 */
void free_card(struct card* p) {
	if(p == NULL) return;

	p->id = 0;
}

/*
 * Tabella delle colonne. L'ordine è column-major, dove columns[i][j] è la 
 * j-esima card della i-esima colonna
 */
struct card* columns[NUM_COLS][MAX_CARDS_PER_COL] = {0};

/*
 * Inserisce una card in una determinata colonna
 *
 * Valori di errore:
 * -1: spazio esaurito
 */
int insert_card(struct card* c, col_id where) {
	struct card** column = columns[where];

	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] == NULL) {
			column[i] = c;
			return 0;
		}
	}

	return -1; // spazio esaurito
}

/*
 * Trova l'indice di una card in una determinata colonna
 *
 * Valori di errore:
 * -1: non trovata
 */
int find_card(card_id id, col_id* where) {
	for(*where = 0; *where < NUM_COLS; (*where)++) {
		struct card** column = columns[*where];

		for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
			if(column[i] != NULL && column[i]->id == id) {
				return i;
			}
		}
	}

	return -1; // non trovata
}

/*
 * Trova l'indice della prima card in una determinata colonna
 *
 * Valori di errore:
 * -1: colonna vuota
 */
int get_first_card(col_id where) {
	struct card** column = columns[where];

	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] != NULL) {
			return i;
		}
	}

	return -1;
}

// ==== GESTORI COMANDI ====

/*
 * Un utente comunica alla lavagna la creazione di una nuova card, assegnandole
 * id, colonna e testo attività
 */
int create_card(card_id id, col_id col, char* desc) {
	// alloca card
	struct card* c = alloc_card();
	if(c == NULL) return -1;

	// inizializza card
	c->id = id;

	// copia quanto della stringa effettivamente entra
	strncpy(c->desc, desc, CARD_DESC_LEN - 1);
	c->desc[CARD_DESC_LEN - 1] = '\0';

	// inserisci timestamp
	time_t now = time(NULL);
	c->timestamp = *localtime(&now);

	// inserisci card
	if(insert_card(c, col) < 0) {
		free_card(c); // libera la card allocata
		return -1;
	}

	printf("Creata card ");
	print_card(c);
	printf(" e inserita nella colonna %s\n", ctoa(col));

	return 0;
}

/*
 * Un'utente notifica la sua presenza (registrazione) alla lavagna
 */
int hello(client_id cl) {
	int ret = register_client(cl);
	if(ret == 0) printf("Registrato client %d\n", cl);

	return ret < 0 ? -1 : 0;
}

/*
 * Un'utente notifica la sua uscita alla lavagna. Se l’utente aveva una card in
 * DOING, questa andrà riportata a TO_DO per essere riassegnata in seguito
 */
int quit(client_id cl) {
	int ret = unregister_client(cl);
	if(ret == 0) printf("Deregistrato client %d\n", cl);

	return ret < 0 ? -1 : 0;
}

/*
 * Una card viene spostata da una colonna ad un altra 
 */
int move_card(card_id id, col_id to) {
	col_id from;
	int idx = find_card(id, &from);
	if(idx < 0) return -1;

	insert_card(columns[from][idx], to);
	columns[from][idx] = NULL;
	printf("Carta di indice %d spostata dalla colonna %s alla colonna %s\n", 
			id, ctoa(from), ctoa(to));

	return 0;
}

/*
 * Mostra una rappresentazione grafica della lavagna
 */
int show_lavagna() {
	// intestazione 
	for(int i = 0; i < NUM_COLS; i++) {
		printf("%-30s", ctoa(i));
	}
	printf("\n");

	// separatore
	for(int i = 0; i < NUM_COLS; i++) printf(SEP_30);
	printf("\n");

	// contenuto
	
	// layout card:
	// <id> : <user> : <timestamp>
	// <desc>
	// \n
	
	for(int j = 0; j < MAX_CARDS_PER_COL; j++) {
		for(int l = 0; l < 4; l++) {
			for(int i = 0; i < NUM_COLS; i++) {
				struct card* c = columns[i][j];

				if(c != NULL) {
					switch(l) {
						case 0: printf("id: %-26d", c->id); break;
						case 1: printf("user: %-24d", c->user); break;
						case 2: {
							// passa il timestamp a stringa con strftime
							char tm_buf[64];
							strftime(tm_buf, sizeof(tm_buf), "%Y-%m-%d %H:%M:%S", 
									&c->timestamp);
							
							printf("time: %-24s", tm_buf); 
							break;
						}
						case 3: printf("%-30s", c->desc); break;
					}	
				} else {
					// tot. 4 righe vuote
					printf("%-30s", "");
				}
			}

			printf("\n");
		}

		printf("\n");
	}

	return 0;
}

/*
 * La lavagna restituisce la lista delle porte degli utenti registrati ad un
 * utente
 */
int request_user_list(client_id cl) {
	int argc = 1;
	char buf[MAX_CLIENTS][6]; // 5 caratteri + terminatore per 65535
	
	char* argv[MAX_CLIENTS + 1];
	argv[0] = "SEND_USER_LIST";

	for(int i = 0; i < MAX_CLIENTS; i++) {
		client_id cl = clients[i];
		if(cl != 0) {
			snprintf(buf[argc], 6, "%d", cl);
			argv[argc] = buf[argc];
			argc++;
		}
	}

	reply(cl, argc, argv);
	printf("Inviata lista utenti a %d\n", cl);

	return 0;
}

/*
 * Per le card che sono in DOING da più di un certo tempo, la lavagna cerca 
 * di contattare l’utente. Se l’utente non risponde entro un certo tempo, 
 * la card viene rimessa in TO_DO 
 */
int ping_user() {
	// TODO: implementa
}

/*
 * Helper che invia una card ad un dato client. In particolare, basta inviare
 * id della carta e descrizione
 */
void send_card(client_id cl, struct card* c) {
	int argc = 3;
	char* argv[3];

	argv[0] = "HANDLE_CARD";

	char buf[6];
	snprintf(buf, 6, "%d", c->id);
	argv[1] = buf;

	argv[2] = c->desc;

	reply(cl, argc, argv);
	printf("Inviata card %d a %d\n", c->id, cl);
}

/*
 * La lavagna invia, in ordine di porta, ad ogni utente connesso una card della
 * colonna TO_DO. Questo comando, oltre alla card, include la lista delle porte
 * degli utenti presenti (escluso il destinatario del messaggio), e il numero 
 * degli utenti presenti
 */
int handle_card() {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		client_id cl = clients[i];
		if(cl != 0) {
			int idx = get_first_card(TO_DO);
			if(idx < 0) return 0; // finite le card

			struct card* c = columns[TO_DO][idx];

			// invia la card
			send_card(cl, c);

			insert_card(c, DOING);			
			columns[TO_DO][idx] = NULL;
			
			// invia anche lista utenti
			request_user_list(cl);
		}
	}

	return 0;
}

/*
 * L'utente al quale è assegnata la card, dopo aver ricevuto la review da tutti
 * gli altri utenti, comunica che l’attività nella card è terminata
 */
int card_done(card_id id) {
	return move_card(id, DONE);
}

// ==== INTERPRETAZIONE COMANDI ==== 

/*
 * Rappresenta il tipo di comando richiesto
 */
typedef enum {
	CREATE_CARD,
	HELLO,
	QUIT,
	MOVE_CARD, // TODO: è più un helper che un comando
	SHOW_LAVAGNA,
	REQUEST_USER_LIST,
	PING_USER,
	HANDLE_CARD, // TODO: non è propriamente un comando, il server lo fa autonomamente
							 // (dovrebbe essere qualcosa come push_card(client_id cl))
	CARD_DONE,
	// valore di errore	
	INVALID
} cmd_type;

/*
 * Entrata di tabella per la mappa comandi, cioè:
 * - da stringa che rappresenta la parola chiave del comando
 * - al tipo del comando stesso
 */
typedef struct {
	const char* keyword;
	cmd_type type;
} cmd;

/*
 * Tabella di entrate cmd che rappresenta la mappa comandi 
 */
static const cmd cmd_table[] = {
	{"CREATE_CARD", CREATE_CARD},
	{"HELLO", HELLO},
	{"QUIT", QUIT},
	{"MOVE_CARD", MOVE_CARD},
	{"SHOW_LAVAGNA", SHOW_LAVAGNA},
	{"REQUEST_USER_LIST", REQUEST_USER_LIST},
	{"PING_USER", PING_USER},
	{"HANDLE_CARD", HANDLE_CARD},
	{"CARD_DONE", CARD_DONE}
};

/*
 * Macro per il numero di entrate nella mappa comandi 
 */
#define NUM_CMDS (int)(sizeof(cmd_table) / sizeof(cmd_table[0]))

/*
 * Ottiene il tipo di comando a partire dalla stringa che rappresenta la parola
 * chiave del comando effettuando una ricerca sulla mappa comandi 
 */
cmd_type get_cmd_type( char* keyword) {
	for(int i = 0; i < NUM_CMDS; i++) {
		const cmd* entry = &cmd_table[i];
		if(strcmp(entry->keyword, keyword) == 0) {
			return entry->type;
		}
	}

	return INVALID;
}

int parse_command(client_id cl, int argc, char* argv[]) {
	if(argc < 1) return -2; // comando vuoto 	

	cmd_type type = get_cmd_type(argv[0]); 
	switch (type) {
		case CREATE_CARD:
			if(argc < 4) return -4; // troppi pochi argomenti 
			return create_card(atoi(argv[1]), atoc(argv[2]), argv[3]);

		case HELLO: return hello(cl);
		case QUIT: return quit(cl);
		case MOVE_CARD:
			if(argc < 3) return -4;
			return move_card(atoi(argv[1]), atoc(argv[2]));
		
		case SHOW_LAVAGNA: return show_lavagna();
		case REQUEST_USER_LIST: return request_user_list(cl);
		case PING_USER: return ping_user();
		case HANDLE_CARD: return handle_card();
		case CARD_DONE: 
			if(argc < 2) return -4;
			return card_done(atoi(argv[1]));

		default: return -3; // comando non valido
	}
}
