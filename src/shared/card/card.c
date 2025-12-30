#include "card.h"
#include "string.h" // utilità stringa

// ==== TIPI CARD ====

const char *col_names[] = {"TO_DO", "DOING", "DONE"};

col_id str_to_col(const char *str) {
  for (int i = 0; i < NUM_COLS; i++) {
    if (strcmp(col_names[i], str) == 0) {
      return i;
    }
  }

  // sarebbe errore, restituisci 0
  return 0; // TO_DO
}

const char *col_to_str(col_id id) { return col_names[id]; }
