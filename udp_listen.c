#include <err.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core.h"

#define USAGE		"Usage: %s [-p port]"

int main(int argc, char **argv)
{
	int opt, port = 4242;
	struct sockaddr_in addr;
	int sd;

	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
		case 'p':
			port = atoi(optarg);
			break;
		default:
			errx(-1, USAGE, argv[0]);
		}
	}

	addr = (struct sockaddr_in) {
		.sin_family = AF_INET,
		.sin_port = port,
	};

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
		err(-1, "socket");

	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)))
		err(-1, "bind");

	do_serve_dgram(sd);

	close(sd);

	return 0;
}
