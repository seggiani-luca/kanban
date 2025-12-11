#include "command.h"
#include <string.h> // utilità stringa

// ==== TIPI COMANDO ====

/*
 * Entrata di tabella per la mappa comandi, cioè:
 * - da stringa che rappresenta la parola chiave del comando
 * - al tipo del comando stesso
 */
typedef struct {
  const char *keyword;
  cmd_type type;
} cmd_entry;

/*
 * La mappa comandi
 */
cmd_entry cmd_table[] = {
    // client -> server
    {"CREATE_CARD", CREATE_CARD},
    {"HELLO", HELLO},
    {"QUIT", QUIT},
    {"PONG_LAVAGNA", PONG_LAVAGNA},
    {"ACK_CARD", ACK_CARD},
    {"REQUEST_USER_LIST", REQUEST_USER_LIST},
    {"CARD_DONE", CARD_DONE},

    // console -> server
    {"SHOW_LAVAGNA", SHOW_LAVAGNA},
    {"SHOW_CLIENTS", SHOW_CLIENTS},
    {"MOVE_CARD", MOVE_CARD},

    // server -> done
    {"SEND_USER_LIST", SEND_USER_LIST},
    {"PING_USER", PING_USER},
    {"HANDLE_CARD", HANDLE_CARD},

    // client -> client
    {"REVIEW_CARD", REVIEW_CARD},
    {"ACK_REVIEW_CARD", ACK_REVIEW_CARD},

    // general
    {"OK", OK},
    {"ERR", ERR}};

/*
 * Macro per il numero di comandi
 */
#define NUM_CMDS (ERR + 1)

cmd_type str_to_type(const char *keyword) {
  // scansiona la mappa comandi per stringa
  for (int i = 0; i < NUM_CMDS; i++) {
    const cmd_entry *entry = &cmd_table[i];
    if (strcmp(entry->keyword, keyword) == 0) {
      return entry->type;
    }
  }

  return ERR;
}

const char *type_to_str(cmd_type cmd) {
  // scansiona la mappa comandi per tipo
  for (int i = 0; i < NUM_CMDS; i++) {
    const cmd_entry *entry = &cmd_table[i];
    if (entry->type == cmd) {
      return entry->keyword;
    }
  }

  return "";
}

int get_argc(const cmd *cm) {
  int i = 0;
  while (i < MAX_NET_ARGS) {
    if (cm->args[i] == NULL)
      break;
    i++;
  }

  return i;
}

void cmd_to_buf(const cmd *cm, char *buf) {
  int pos = 0;

  // copia il nome di comando
  const char *str = type_to_str(cm->type);
  int len = strlen(str);

  if (pos + len + 1 >= NET_BUF_SIZE)
    return;
  memcpy(buf + pos, str, len);
  pos += len;

  // copia gli argomenti
  for (int i = 0; i < get_argc(cm); i++) {
    if (i >= MAX_NET_ARGS)
      return;

    const char *str = cm->args[i];
    int len = strlen(str);

    if (pos + len + 2 >= NET_BUF_SIZE)
      return;
    buf[pos++] = ' '; // inserisci spazio
    memcpy(buf + pos, str, len);
    pos += len;
  }

  // inserisci terminatore
  buf[pos] = '\0';
}

void buf_to_cmd(char *buf, cmd *cm) {
  // tokenizza la stringa
  int argc = -1;
  char *token = strtok(buf, " ");
  while (token && argc < MAX_NET_ARGS + 1) {
    if (argc == -1) {
      cm->type = str_to_type(token);
      argc++;
    } else {
      cm->args[argc++] = token;
    }

    token = strtok(NULL, " ");
  }
}
