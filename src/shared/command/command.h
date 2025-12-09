#ifndef COMMAND_H
#define COMMAND_H

#include "../net_const.h"	// MAX_NET_ARGS

// ==== TIPI COMANDO ====

/*
 * Enum dei comandi permessi dal sistema, distinti in categorie rispetto a chi
 * invia e chi riceve il comando.
 */
typedef enum {
	// client -> server
	CREATE_CARD,
	HELLO,
	QUIT,
	PONG_LAVAGNA,
	ACK_CARD,
	REQUEST_USER_LIST,
	CARD_DONE,

	// console -> server
	SHOW_LAVAGNA,
	SHOW_CLIENTS,
	MOVE_CARD,

	// server -> done
	SEND_USER_LIST,
	PING_USER,
	HANDLE_CARD,

	// client -> client
	REVIEW_CARD,
	ACK_REVIEW_CARD,

	// general
	OK,
	ERR
} cmd_type;

/*
 * Ottiene il tipo di comando a partire dalla stringa che rappresenta la parola
 * chiave del comando effettuando una ricerca sulla mappa comandi 
 */
cmd_type lit_to_typ(const char* keyword);

/*
 * Ottiene la stringa che rappresenta la parola chiave di un comando a partire 
 * dal tipo di comando effettuando una ricerca sulla mappa comandi 
 */
const char* typ_to_lit(cmd_type cmd);

/*
 * Un comando è rappresentato da:
 * - il suo tipo
 * - i suoi argomenti, come una lista terminata da NULL
 */
typedef struct {
	cmd_type typ;
	const char* args[MAX_NET_ARGS];
} cmd;

/*
 * Ottiene il numero di argomenti in un comando
 */
int get_argc(const cmd* cm);

/*
 * Serializza un comando su una stringa (si assume che la dimensione sia
 * NET_BUF_SIZE)
 */
void cmd_to_buf(const cmd* cm, char* buf);

/*
 * Deserializza un comando da una stringa (si assume che il comando sia 
 * zero-inizializzato)
 */
void buf_to_cmd(char* buf, cmd* cm);

#endif
