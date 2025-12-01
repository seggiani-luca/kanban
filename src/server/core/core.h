#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include "../../shared/card.h"
#include <stdint.h>

// ==== TIPI INTERFACCIA ====

/*
 * Un client è identificato dal suo numero di porta (su 16 bit)
 */
typedef uint16_t client_id;

/*
 * Rappresenta un client, identificato da:
 * - id (se è 0 il client è nullo)
 * - puntatore alla card che sta gestendo (se è NULL sta aspettando una card)
 */
struct client {
	client_id id;
	struct card* handling;
};

/*
 * Rappresenta una funzione di callback per la risposta al client
 */
typedef void (*reply_cback)(
	client_id cl,
	int argc,
	char* argv[]
);

// ==== FUNZIONI INTERFACCIA ====

/*
 * Imposta la funzione di callback usata per rispondere alle richieste
 */
void set_reply_callback(reply_cback new_reply);

/*
 * Gestisce un comando chiamando l'hook giusto con la lista di argomenti 
 * fornita
 *
 * Valori di errore:
 * -1: errore esecuzione comando
 * -2: comando vuoto
 * -3: comando non valido
 * -4: troppi pochi argomenti 
 * -5: utente non registrato
 */
int parse_command(client_id cl_id, int argc, char* argv[]);

#endif
