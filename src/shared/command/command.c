#include "command.h"
#include <string.h>

/*
 * Entrata di tabella per la mappa comandi, cioè:
 * - da stringa che rappresenta la parola chiave del comando
 * - al tipo del comando stesso
 */
typedef struct {
	const char* keyword;
	cmd_type type;
} cmd_entry;

/*
 * La mappa comandi
 */
cmd_entry cmd_table[] = {
    // client -> server
    {"CREATE_CARD", CREATE_CARD},
    {"HELLO", HELLO},
    {"QUIT", QUIT},
    {"PONG_LAVAGNA", PONG_LAVAGNA},
    {"ACK_CARD", ACK_CARD},
    {"REQUEST_USER_LIST", REQUEST_USER_LIST},
    {"CARD_DONE", CARD_DONE},

    // console -> server
    {"SHOW_LAVAGNA", SHOW_LAVAGNA},
    {"SHOW_CLIENTS", SHOW_CLIENTS},

    // server -> done
    {"SEND_USER_LIST", SEND_USER_LIST},
    {"PING_USER", PING_USER},
    {"HANDLE_CARD", HANDLE_CARD},

    // client -> client
    {"REVIEW_CARD", REVIEW_CARD},
    {"ACK_REVIEW_CARD", ACK_REVIEW_CARD},

    // general
    {"OK", OK},
    {"ERR", ERR}
};

cmd_type get_cmd_type(const char* keyword) {
	for(int i = 0; i < NUM_CMDS; i++) {
		const cmd_entry* entry = &cmd_table[i];
		if(strcmp(entry->keyword, keyword) == 0) {
			return entry->type;
		}
	}

	return ERR;
}

const char* get_cmd_string(cmd_type cmd) {
	for(int i = 0; i < NUM_CMDS; i++) {
		const cmd_entry* entry = &cmd_table[i];
		if(entry->type == cmd) {
			return entry->keyword;
		}
	}

	return "";
}
