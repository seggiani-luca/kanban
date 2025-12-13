#include "cmd.h"
#include <stdio.h>  // snprintf
#include <string.h> // utilità stringa

// ==== TIPI COMANDO ====

/*
 * Entrata di tabella per la mappa comandi, cioè un record di:
 * - tipo di comando
 * - stringa corrispondente
 * - modulo di gestione
 */
typedef struct {
  cmd_type type;
  const char *str;
  mod_type mod;
} cmd_entry;

/*
 * Mappa comandi
 */
cmd_entry cmd_table[] = {
    // client -> server
    {CREATE_CARD, "CREATE_CARD", CORE},
    {HELLO, "HELLO", CORE},
    {QUIT, "QUIT", CORE},
    {PONG_LAVAGNA, "PONG_LAVAGNA", WATCH},
    {ACK_CARD, "ACK_CARD", CORE},
    {REQUEST_USER_LIST, "REQUEST_USER_LIST", CORE},
    {CARD_DONE, "CARD_DONE", CORE},

    // server -> done
    {SEND_USER_LIST, "SEND_USER_LIST", CORE},
    {PING_USER, "PING_USER", WATCH},
    {HANDLE_CARD, "HANDLE_CARD", CORE},
    {OK, "OK", CORE},
    {ERR, "ERR", CORE},

    // console -> client
    {SHOW_LAVAGNA, "SHOW_LAVAGNA", CORE},
    {SHOW_CLIENTS, "SHOW_CLIENTS", CORE},
    {MOVE_CARD, "MOVE_CARD", CORE},

    // client -> client
    {REVIEW_CARD, "REVIEW_CARD", PEER},
    {ACK_REVIEW_CARD, "ACK_REVIEW_CARD",
     CORE} // questo è CORE in quanto è la risposta ad un comando fatto dal
           // flusso del client
};

cmd_type str_to_type(const char *str) {
  if (str == NULL)
    return ERR;

  for (int i = 0; i < NUM_CMD_TYPES; i++) {
    const cmd_entry *entry = &cmd_table[i];
    if (strcmp(entry->str, str) == 0) {
      return entry->type;
    }
  }

  return ERR;
}

const char *type_to_str(cmd_type type) { return cmd_table[type].str; }

mod_type type_to_mod(cmd_type type) { return cmd_table[type].mod; }

int get_argc(const cmd *cm) {
  int i = 0;
  while (i < MAX_CMD_ARGS) {
    if (cm->args[i] == NULL) {
      break;
    }
    i++;
  }

  return i;
}

void cmd_to_buf(const cmd *cm, char *buf) {
  int pos = 0;

  // copia tipo
  const char *type_str = type_to_str(cm->type);
  pos += snprintf(buf, CMD_BUF_SIZE, "%s", type_str);

  // copia gli argomenti
  for (int i = 0; i < get_argc(cm) && pos < CMD_BUF_SIZE; i++) {
    pos += snprintf(buf + pos, CMD_BUF_SIZE - pos, " %s", cm->args[i]);
  }
}

void buf_to_cmd(char *buf, cmd *cm) {
  // tokenizza il tipo
  char *token = strtok(buf, " ");
  cm->type = str_to_type(token);

  // tokenizza gli argomenti
  int argc = 0;
  while ((token = strtok(NULL, " ")) && argc < MAX_CMD_ARGS) {
    cm->args[argc++] = token;
  }
}
