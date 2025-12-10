#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include "../../shared/card/card.h"				// tipo card
#include "../../shared/command/command.h"	// tipo cmd
#include <stdint.h>												// tipo uint16_t (per client_id)

// ==== GESTIONE CLIENT ====

/*
 * Un client è identificato dal suo numero di porta (su 16 bit)
 */
typedef uint16_t client_id;


// ==== FUNZIONI INTERFACCIA ====

/*
 * Gestisce un comando di un client chiamando l'hook giusto per il comando 
 * fornito. Può usare il callback per rispondere al client.
 */
void exec_command(client_id cl_id, const cmd* cm);

/*
 * Mostra una rappresentazione grafica della lavagna
 */
int show_lavagna();

/*
 * Mostra i client attualmente registrati
 */
int show_clients();

#endif
