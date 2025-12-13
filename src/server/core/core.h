#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include "../../shared/card/card.h" // tipo card
#include "../../shared/cmd/cmd.h"   // tipo cmd

// ==== GESTIONE CLIENT ====

/*
 * Un client è identificato dal suo numero di porta (su 16 bit)
 */
typedef unsigned short client_id;

// ==== FUNZIONI INTERFACCIA ====

/*
 * Gestisce un comando di un client chiamando l'hook giusto per il comando
 * fornito. Può usare il callback per rispondere al client.
 */
void handle_command(client_id cl_id, const cmd *cm);

/*
 * Usata per deregistrare un client che si è disconnesso in maniera prematura
 */
void unregister_client_id(client_id cl_id);

/*
 * Mostra una rappresentazione grafica della lavagna
 */
int show_lavagna();

/*
 * Mostra i client attualmente registrati
 */
int show_clients();

#endif
