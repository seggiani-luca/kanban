#ifndef CARD_H
#define CARD_H

#include <time.h> // data in timestamp

// ==== TIPI CARD ====

/*
 * Massimo numero di caratteri nella descrizione di una carta (incluso \0)
 */
#define CARD_DESC_LEN 20

/*
 * Una carta è identificata da un intero su 16 bit
 */
typedef unsigned short card_id;

/*
 * Rappresenta le possibili colonne a cui può appartenere una carta
 */
typedef enum { TO_DO, DOING, DONE } col_id;

/*
 * Vettore contenente i nomi delle colonne
 */
extern const char *col_names[];

/*
 * Macro per il numero di colonne
 */
#define NUM_COLS (DONE + 1)

/*
 * Converte una stringa in un indice di colonna. In caso di errore restituisce
 * TO_DO (0) come valore di default
 */
col_id str_to_col(const char *str);

/*
 * Converte un indice di colonna in una stringa
 */
const char *col_to_str(col_id id);

/*
 * Rappresenta una carta, identificata da:
 * - id (se è 0 la card è nulla)
 * - colonna
 * - testo attività
 * - utente (porta) che la sta implementando (DOING) o l'ha implementata (DONE)
 * - timestamp dell'ultima modifica
 */
typedef struct {
  card_id id;
  char desc[CARD_DESC_LEN];
  int user;
  struct tm timestamp;
} card;

#endif
