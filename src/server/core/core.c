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
static struct client clients[MAX_CLIENTS] = {0};

/*
 * Registra un nuovo client nel vettore client registrati e restituisce il suo
 * indice
 *
 * Valori di errore:
 * -1: id non valido
 * -2: spazio esaurito
 */
int register_client(client_id cl) {
	if(cl == 0) return -1; // id non valido

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i].id == 0) {
			clients[i].id = cl;
			clients[i].handling = NULL;
			return i;
		}
	}

	return -2; // spazio esaurito 
}

// dichiarate in anticipo per unregister_client()
void set_timestamp(struct card* c);
int move_card(card_id id, col_id to);

/*
 * Deregistra un client dal vettore client registrati
 *
 * Valori di errore:
 * -1: card posseduta non trovata
 */
int unregister_client(struct client* cl) {
	cl->id = 0;

	// se possedeva una card, liberala
	struct card* c = cl->handling;
	if(c != NULL) {
		c->user = 0;
		set_timestamp(c);
		
		// card posseduta non trovata
		return (move_card(c->id, TO_DO) < 0) ? -1 : 0; 
	}

	return 0;
}

/*
 * Trova il client con dato id nel vettore client registrati. Restituisce NULL 
 * se non lo trova
 */
struct client* find_client(client_id cl) {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i].id == cl) {
			return &clients[i];
		}
	}

	return NULL;
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

// ==== FUNZIONI HELPER ====

/*
 * Imposta il timestamp di una card alla data corrente
 */
void set_timestamp(struct card* c) {
	time_t now = time(NULL);
	c->timestamp = *localtime(&now);
}

/*
 * Helper che invia una card ad un dato client. In particolare, basta inviare
 * id della carta e descrizione
 */
void send_card(struct client* cl, struct card* c) {
	// realizza e invia messaggio
	int argc = 3;
	char* argv[3];

	argv[0] = "HANDLE_CARD";

	char buf[6];
	snprintf(buf, 6, "%d", c->id);
	argv[1] = buf;

	argv[2] = c->desc;

	reply(cl->id, argc, argv);
	printf("Inviata card %d a %d\n", c->id, cl->id);

	// registra card gestita
	cl->handling = c;

	// aggiorna card con utente
	c->user = cl->id;
}

// dichiarata in anticipo per push_card()
int request_user_list(struct client* cl);

/*
 * Helper di push_cards che invia una card ad un singolo client. Viene usata da
 * hello() e card_done() per provare ad inviare una card ad un singolo client
 *
 * Valori di errore:
 * -1: finite le card
 */
int push_card(struct client* cl) {
	cl->handling = NULL;

	int idx = get_first_card(TO_DO);
	if(idx < 0) return -1; // finite le card

	struct card* c = columns[TO_DO][idx];

	// invia la card
	send_card(cl, c);

	// versione ridotta di move_card, conosciamo già l'indice
	insert_card(c, DOING);
	columns[TO_DO][idx] = NULL;

	// aggiorna timestamp
	set_timestamp(c);

	// invia anche lista utenti
	request_user_list(cl);

	return 0;
}

/*
 * La lavagna invia, in ordine di porta, ad ogni utente connesso una card della
 * colonna TO_DO. Questo comando, oltre alla card, include la lista delle porte
 * degli utenti presenti (escluso il destinatario del messaggio), e il numero 
 * degli utenti presenti
 */
void push_cards() {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		struct client* cl = &clients[i];
		if(cl->id != 0 && cl->handling == NULL) {
			if(push_card(cl) < 0) return; // finite le card
		}
	}
}

/*
 * Sposta una card da una colonna ad un altra 
 *
 * Valori di errore:
 * -1: card non trovata
 */
int move_card(card_id id, col_id to) {
	// trova la card da spostare
	col_id from;
	int idx = find_card(id, &from);
	if(idx < 0) return -1; // card non trovata

	// aggiorna timestamp
	set_timestamp(columns[from][idx]);

	// sposta la card
	insert_card(columns[from][idx], to);
	columns[from][idx] = NULL;
	printf("Carta di indice %d spostata dalla colonna %s alla colonna %s\n", 
			id, ctoa(from), ctoa(to));
	
	return 0;
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
	set_timestamp(c);

	// inserisci card
	if(insert_card(c, col) < 0) {
		free_card(c); // libera la card allocata
		return -1;
	}

	printf("Creata card ");
	print_card(c);
	printf(" e inserita nella colonna %s\n", ctoa(col));

	// assegna card se necessario
	push_cards();

	return 0;
}

/*
 * Un'utente notifica la sua presenza (registrazione) alla lavagna
 */
int hello(client_id cl) {
	int ret = register_client(cl);
	if(ret == 0) printf("Registrato client %d\n", cl);

	// invia la prima card
	push_card(&clients[ret]);

	return ret < 0 ? -1 : 0;
}

/*
 * Un'utente notifica la sua uscita alla lavagna. Se l’utente aveva una card in
 * DOING, questa andrà riportata a TO_DO per essere riassegnata in seguito
 */
int quit(struct client* cl) {
	int ret = unregister_client(cl);
	if(ret > 0) printf("Deregistrato client %d\n", cl->id);

	return ret;
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
	for(int j = 0; j < MAX_CARDS_PER_COL; j++) {

		// 4 linee per card
		for(int l = 0; l < 4; l++) {
			
			// prima colonne, poi righe
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
 * Mostra i client attualmente registrati
 */
int show_clients() {
	for(int i = 0; i < MAX_CLIENTS; i++) {
		struct client* cl = &clients[i];
		if(cl->id == 0) continue;

		printf("%d: ", cl->id);
		if(cl->handling != NULL) {
			print_card(cl->handling);
		} else {
			printf("waiting");
		}

		printf("\n");
	}
	
	return 0;
}

/*
 * La lavagna restituisce la lista delle porte degli utenti registrati ad un
 * utente
 */
int request_user_list(struct client* cl) {
	int argc = 1;
	char buf[MAX_CLIENTS][6]; // 5 caratteri + terminatore per 65535
	
	char* argv[MAX_CLIENTS + 1];
	argv[0] = "SEND_USER_LIST";

	for(int i = 0; i < MAX_CLIENTS; i++) {
		client_id cl = clients[i].id;
		if(cl != 0) {
			snprintf(buf[argc], 6, "%d", cl);
			argv[argc] = buf[argc];
			argc++;
		}
	}

	reply(cl->id, argc, argv);
	printf("Inviata lista utenti a %d\n", cl->id);

	return 0;
}

/*
 * L'utente al quale è assegnata la card, dopo aver ricevuto la review da tutti
 * gli altri utenti, comunica che l’attività nella card è terminata
 */
int card_done(struct client* cl) {
	if(cl->handling == NULL) return -1; // nessuna card posseduta

	// segna l'id della card gestita
	int id = cl->handling->id;

	// fornisci una nuova card
	push_card(cl);

	// sposta la card in DONE
	if(move_card(id, DONE) < 0) return -1; // card posseduta non trovata 
	
	return 0;
}

// ==== INTERPRETAZIONE COMANDI ==== 

/*
 * Rappresenta il tipo di comando richiesto
 */
typedef enum {
	CREATE_CARD,
	SHOW_LAVAGNA,
	SHOW_CLIENTS,
	HELLO,
	QUIT,
	REQUEST_USER_LIST,
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
	{"SHOW_LAVAGNA", SHOW_LAVAGNA},
	{"SHOW_CLIENTS", SHOW_CLIENTS},
	{"HELLO", HELLO},
	{"QUIT", QUIT},
	{"REQUEST_USER_LIST", REQUEST_USER_LIST},
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

int parse_command(client_id cl_id, int argc, char* argv[]) {
	if(argc < 1) return -2; // comando vuoto 	

	// discrimina su tipo comando
	cmd_type type = get_cmd_type(argv[0]); 

	// ottieni puntatore client
	struct client* cl;
	if(type != HELLO) {
		cl = find_client(cl_id);
		if(cl == NULL) return -5; // utente non registrato
	}

	switch (type) {
		case CREATE_CARD:
			if(argc < 4) return -4; // troppi pochi argomenti 
			return create_card(atoi(argv[1]), atoc(argv[2]), argv[3]);
		case SHOW_LAVAGNA: return show_lavagna();
		case SHOW_CLIENTS: return show_clients();

		case HELLO: return hello(cl_id);
		case QUIT: return quit(cl);	
		case REQUEST_USER_LIST: return request_user_list(cl);
		case CARD_DONE: return card_done(cl);

		default: return -3; // comando non valido
	}
}
