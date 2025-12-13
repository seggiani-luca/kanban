#ifndef CLIENT_WATCH_H
#define CLIENT_WATCH_H

#include "../core/core.h" // logica server

/*
 * Eseguito ai tick del timer (ogni secondo) per controllare lo stato dei client.
 * Il valore di ritorno discrimina se:
 * -1: il client va disconnesso
 *  0: non c'è da fare nulla
 *  1: un client ha avuto un cambio di stato (ridisegna interfaccia) 
 */
int tick_timer(client_id cl_id);

/*
 * Gestisce un ping dai client
 */
void handle_pong(client_id cl_id);

#endif
