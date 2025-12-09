#include "card.h"
#include "string.h"									// utilità stringa

// ==== TIPI CARD ====

const char* col_names[] = {
	"TO_DO",
	"DOING",
	"DONE"
};

col_id atoc(const char* str) {
	for(int i = 0; i < NUM_COLS; i++) {
		if(strcmp(col_names[i], str) == 0) {
			return i;
		}
	}

	// sarebbe errore, restituisci 0 
	return 0; // TO_DO
}

const char* ctoa(col_id id) {
	return col_names[id];
}

/*
 * Macro per il numero massimo di card supportate
 */
#define MAX_CARDS (NUM_COLS * MAX_CARDS_PER_COL)

/*
 * Pool di card presenti nel sistema
 */
card cards[MAX_CARDS] = {0};

card* alloc_card() {
	// cerca una card libera
	for(int i = 0; i < MAX_CARDS; i++) {
		if(cards[i].id == 0) {
			return &cards[i];
		}
	}

	// se sei qui  non ci sono card libere
	return NULL;
}

/*
 * Dealloca una card nella pool. Non fa nulla se si fornisce NULL
 */
void free_card(card* p) {
	// gestisci puntatori NULL
	if(p == NULL) return;

	// libera la card impostando id a 0
	p->id = 0;
}

