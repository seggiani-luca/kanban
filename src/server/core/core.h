#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include "../../shared/card/card.h"				// tipo card
#include "../../shared/command/command.h"	// tipo cmd
#include <stdint.h>												// tipo uint16_t (per client_id)

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
typedef struct {
	client_id id;
	client_sts sts;
	card* handling;
} client;

/*
 * Rappresenta una funzione di callback per la risposta al client
 */
typedef void (*reply_cback)(
	client_id cl_id,
	const cmd* cm
);

// ==== FUNZIONI INTERFACCIA ====

/*
 * Imposta la funzione di callback usata per rispondere alle richieste
 */
void set_reply_callback(reply_cback new_reply);

/*
 * Gestisce un comando di un client chiamando l'hook giusto per il comando 
 * fornito. Può usare il callback per rispondere al client.
 */
void exec_command(client_id cl_id, const cmd* cm);

/*
 * Inizializza il core mostrando la lavagna
 */
int show_lavagna();

#endif
