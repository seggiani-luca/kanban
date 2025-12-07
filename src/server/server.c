#include "core/core.h"
#include "net/net.h"

int main() {
	if(configure_net()) return 1;

	listen_net();

	return 0;
}
