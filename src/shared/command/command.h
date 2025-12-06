#ifndef COMMAND_H
#define COMMAND_H

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
 * Macro per il numero di comandi
 */
#define NUM_CMDS (ERR + 1)

/*
 * Ottiene il tipo di comando a partire dalla stringa che rappresenta la parola
 * chiave del comando effettuando una ricerca sulla mappa comandi 
 */
cmd_type get_cmd_type(const char* keyword);

/*
 * Ottiene la stringa che rappresenta la parola chiave di un comando a partire 
 * dal tipo di comando effettuando una ricerca sulla mappa comandi 
 */
const char* get_cmd_string(cmd_type cmd);

#endif
