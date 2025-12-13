#include "watch.h"
#include "../net/net.h" // gestione di rete client
#include "stdio.h"      // printf

void handle_ping() {
  printf("[%d]\t: Rispondo al ping della lavagna\n", port);

  // invia ping
  cmd cm = {.type = PONG_LAVAGNA, .args = {"sono sempre in linea"}};
  send_server(&cm);
}
