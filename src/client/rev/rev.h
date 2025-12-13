#ifndef REV_H
#define REV_H

#include "../net/net.h"

/*
 * Gestisce un comando di revisione da un client
 */
void handle_rev(unsigned short who);

/*
 * Chiede una revisione ad un client
 */
int req_review(unsigned short who);

#endif
