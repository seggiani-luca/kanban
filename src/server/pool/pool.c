#include "pool.h"

/*
 * Macro per il numero massimo di card supportate
 */
#define MAX_CARDS (NUM_COLS * MAX_CARDS_PER_COL)

/*
 * Pool di card presenti nel sistema
 */
card cards[MAX_CARDS] = {0};

card *alloc_card() {
  // cerca una card libera
  for (int i = 0; i < MAX_CARDS; i++) {
    if (cards[i].id == 0) {
      return &cards[i];
    }
  }

  // se sei qui  non ci sono card libere
  return NULL;
}

/*
 * Dealloca una card nella pool. Non fa nulla se si fornisce NULL
 */
void free_card(card *p) {
  // gestisci puntatori NULL
  if (p == NULL) {
    return;
  }

  // libera la card impostando id a 0
  p->id = 0;
}

int check_card_id(card_id id) {
  for (int i = 0; i < MAX_CARDS; i++) {
    if (cards[i].id == id)
      return 0;
  }

  return 1;
}
