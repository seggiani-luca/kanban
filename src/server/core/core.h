#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include "../../shared/card/card.h"
#include <stdint.h>

// ==== TIPI INTERFACCIA ====

/*
 * Un client è identificato dal suo numero di porta (su 16 bit)
 */
typedef uint16_t client_id;

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
struct client {
	client_id id;
	client_sts sts;
	struct card* handling;
};

/*
 * Numero massimo di utenti supportati
 */
#define MAX_CLIENTS 4

/*
 * Rappresenta una funzione di callback per la risposta al client
 */
typedef void (*reply_cback)(
	client_id cl,
	int argc,
	const char* argv[]
);

// ==== FUNZIONI INTERFACCIA ====

/*
 * Imposta la funzione di callback usata per rispondere alle richieste
 */
void set_reply_callback(reply_cback new_reply);

/*
 * Gestisce un comando di un client chiamando l'hook giusto con la lista di 
 * argomenti fornita. Può chiamare il callback per rispondere al client.
 */
void parse_command(client_id cl_id, int argc, char* argv[]);

#endif
