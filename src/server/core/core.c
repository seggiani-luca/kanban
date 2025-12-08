#include "core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../../shared/command/command.h"

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
 * Il messaggio di errore corrente, impostato durante la gestione dei comandi 
 */
const char* err_mess;

/*
 * Vettore client registrati 
 */
static struct client clients[MAX_CLIENTS] = {0};

/*
 * Registra un nuovo client nel vettore client registrati e restituisce il suo
 * indice
 */
int register_client(client_id cl) {
	if(cl == 0) {
		err_mess = "id utente non valido";
		return -1;
	}

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i].id == 0) {
			clients[i].id = cl;
			clients[i].sts = IDLE;
			return i;
		}
	}

	err_mess = "spazio esaurito";
	return -1; 
}

// dichiarate in anticipo per unregister_client()
void set_timestamp(struct card* c);
int move_card(card_id id, col_id to);

/*
 * Deregistra un client dal vettore client registrati
 */
int unregister_client(struct client* cl) {
	cl->id = 0;

	// se possedeva una card, liberala
	if(cl->sts != IDLE) {
		struct card* c = cl->handling;
		c->user = 0;
		set_timestamp(c);
		
		// card posseduta non trovata
		return move_card(c->id, TO_DO);
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

	err_mess = "client non trovato";
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

	err_mess = "pool card esaurita";
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
 */
int insert_card(struct card* c, col_id where) {
	struct card** column = columns[where];

	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] == NULL) {
			column[i] = c;
			return 0;
		}
	}

	err_mess = "spazio esaurito";
	return -1;
}

/*
 * Trova l'indice di una card in una determinata colonna
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

	err_mess = "card non trovata";
	return -1;
}

/*
 * Trova l'indice della prima card da gestire in TO_DO 
 */
int get_first_card() {
	struct card** column = columns[TO_DO];

	for(int i = 0; i < MAX_CARDS_PER_COL; i++) {
		if(column[i] != NULL && column[i]->user == 0) {
			return i;
		}
	}

	err_mess = "colonna vuota";
	return -1;
}

// ==== FUNZIONI HELPER ====

/*
 * Prepara un comando, fornito il tipo di comando e un commento, su una coppia
 * argc / argv
 */
void prepare_reply(cmd_type cmd, int argc, const char* argv[], 
		const char* comment) {
	argv[0] = get_cmd_string(cmd);

	argv[argc - 1] = comment;
}

/*
 * Risponde al client con un messaggio di ok con un certo commento
 */
void reply_ok(client_id who, const char* comment) {
	int argc = 2;
	const char* argv[argc];
	prepare_reply(OK, argc, argv, comment);

	reply(who, argc, argv);
}

/*
 * Risponde al client con un messaggio di errore con un certo commento
 */
void reply_err(client_id who, const char* comment) {
	int argc = 2;
	const char* argv[argc];
	prepare_reply(ERR, argc, argv, comment);
	
	reply(who, argc, argv);
}

/*
 * Risponde al client con un messaggio di errore con un certo commento, 
 * generato a partire dal valore corrente di err_mess
 */
void reply_err_mess(client_id who) {
	int argc = 3;
	const char* argv[argc];
	prepare_reply(ERR, argc, argv, err_mess);
	argv[1] = "errore esecuzione comando:";

	reply(who, argc, argv);
}

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
	int argc = 4;
	const char* argv[4];
	prepare_reply(HANDLE_CARD, argc, argv, "fornita card da gestire, invio "
		"SEND_USER_LIST e aspetto ACK_CARD");

	char buf[6];
	snprintf(buf, 6, "%d", c->id);
	argv[1] = buf;

	argv[2] = c->desc;

	reply(cl->id, argc, argv);

	// registra card gestita in utente
	cl->sts = SENT_CARD;
	cl->handling = c;

	// aggiorna card con utente
	c->user = cl->id;
}

// dichiarata in anticipo per push_card()
int request_user_list(struct client* cl);

/*
 * Helper di push_cards che invia una card ad un singolo client. Viene usata da
 * hello() e card_done() per provare ad inviare una card ad un singolo client
 */
int push_card(struct client* cl) {
	// ottieni card
	int idx = get_first_card();
	if(idx < 0) return -1;

	struct card* c = columns[TO_DO][idx];

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
int ack_card(struct client* cl, card_id id) {
	if(cl->sts != SENT_CARD) {
		err_mess = "non in attesa di questo client";
		return -1;
	}

	if(cl->handling->id != id) {
		err_mess = "card non assegnata";
		return -1;
	}

	// ottieni card
	col_id where;
	int idx = find_card(id, &where);
	if(idx < 0) return -1;

	if(where != TO_DO) {
		err_mess = "card già confermata";
		return -1;
	}

	struct card* c = columns[TO_DO][idx];

	// versione ridotta di move_card, conosciamo già l'indice
	if(insert_card(c, DOING)< 0) return -1;
	columns[TO_DO][idx] = NULL;

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
	for(int i = 0; i < MAX_CLIENTS; i++) {
		struct client* cl = &clients[i];
		if(cl->id != 0 && cl->handling == NULL) {
			if(push_card(cl) < 0) return; // finite le card
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
int create_card(struct client* cl, card_id id, col_id col, char* desc) {
	if(id == 0) {
		err_mess = "id card non valido";
		return -1;
	}

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

	// rispondi
	reply_ok(cl->id, "creata card richiesta");
	
	// assegna card se necessario
	push_cards();

	return 0;
}

/*
 * Un utente notifica la sua presenza (registrazione) alla lavagna
 */
int hello(client_id cl) {
	int ret = register_client(cl);
	if(ret >= 0) {	
		// rispondi
		reply_ok(cl, "client registrato, seguira' una card");
		
		// invia la prima card
		push_card(&clients[ret]);
		
		return 0;
	} else return -1;
}

/*
 * U  utente notifica la sua uscita alla lavagna. Se l’utente aveva una card in
 * DOING, questa andrà riportata a TO_DO per essere riassegnata in seguito
 */
int quit(struct client* cl) {
	// mantieni id
	int id = cl->id;
	
	// deregistra
	int ret = unregister_client(cl);
	if(ret >= 0) reply_ok(id, "client deregistrato, arrivederci");

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
		switch(cl->sts) {
			case IDLE: printf("IDLE"); break;
			case SENT_CARD: printf("SENT_CARD "); break;
			case BUSY: printf("BUSY "); break;
		}

		if(cl->sts != IDLE) {
			printf("%d", cl->handling->id);
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
	char buf[MAX_CLIENTS][6]; // 5 caratteri + terminatore per 65535
	
	// realizza e invia messaggio
	int argc = 1;
	const char* argv[MAX_CLIENTS + 2];

	for(int i = 0; i < MAX_CLIENTS; i++) {
		if(cl == &clients[i]) continue;

		client_id o_cl = clients[i].id;
		if(o_cl != 0) {
			snprintf(buf[argc], 6, "%d", o_cl);
			argv[argc] = buf[argc];
			argc++;
		}
	}
	
	argc++;
	prepare_reply(SEND_USER_LIST, argc, argv, "fornita lista utenti");

	reply(cl->id, argc, argv);

	return 0;
}

/*
 * L'utente al quale è assegnata la card, dopo aver ricevuto la review da tutti
 * gli altri utenti, comunica che l’attività nella card è terminata
 */
int card_done(struct client* cl) {
	if(cl->sts != BUSY) {
		err_mess = "client non in stato di wait";
		return -1;
	}

	// mantieni l'id della card gestita
	int id = cl->handling->id;
	
	// sposta la card in DONE
	if(move_card(id, DONE) < 0) return -1;
	
	// rispondi
	reply_ok(cl->id, "card processata, ne seguira' un'altra");

	// fornisci una nuova card
	cl->sts = IDLE;
	cl->handling = NULL;
	push_card(cl);

	return 0;
}

// ==== INTERPRETAZIONE COMANDI ==== 

void parse_command(client_id cl_id, int argc, char* argv[]) {
	if(argc < 1) {
		reply_err(cl_id, "comando vuoto");
		return;
	}

	// discrimina su tipo comando
	cmd_type type = get_cmd_type(argv[0]); 

	// ottieni puntatore client
	struct client* cl;
	if(type != HELLO) {
		cl = find_client(cl_id);
		if(cl == NULL) {
			reply_err(cl_id, "registrarsi prima di fare richieste");
			return;
		}
	}

	int ret;
	switch (type) {
		// client -> server
		case CREATE_CARD:
			if(argc < 4) {
				reply_err(cl_id, "troppi pochi argomenti per CREATE_CARD");
				return;
			}
			ret = create_card(cl, atoi(argv[1]), atoc(argv[2]), argv[3]);
			break;

		case HELLO: ret = hello(cl_id); break;
		case QUIT: ret = quit(cl); break;
		case ACK_CARD:
			if(argc < 2) {
				reply_err(cl_id, "troppi pochi argomenti per ACK_CARD");
				return;
			}
			ret = ack_card(cl, atoi(argv[1]));
			break;

		case REQUEST_USER_LIST: ret = request_user_list(cl); break;
		case CARD_DONE: ret = card_done(cl); break;
		
		// console -> server
		case SHOW_LAVAGNA: ret = show_lavagna(); break;
		case SHOW_CLIENTS: ret = show_clients(); break;

		default: 
			reply_err(cl_id, "comando non valido");
			return;
	}

	if(ret < 0) {
		reply_err_mess(cl_id);
	}
}
