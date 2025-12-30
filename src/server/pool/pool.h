#ifndef POOL_H
#define POOL_H

#include "../../shared/card/card.h"

/*
 * Numero massimo di card supportate per colonna
 */
#define MAX_CARDS_PER_COL 10

/*
 * Alloca una card dalla pool. Restituisce NULL se non ce ne sono libere
 */
card *alloc_card();

/*
 * Dealloca una card nella pool. Non fa nulla se si fornisce NULL
 */
void free_card(card *p);

/*
 * Valuta se un indice di card non è ancora stato usato nel sistema
 */
int check_card_id(card_id id);

#endif
