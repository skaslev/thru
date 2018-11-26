#include <err.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cmd.h"
#include "core.h"

#define USAGE		"[-p port] [-s]"

static int udp_listen_main(int argc, char **argv)
{
	int opt, port = 4242;
	unsigned serve_flags = 0;
	struct sockaddr_in addr;
	int sd;

	while ((opt = getopt(argc, argv, "p:s")) != -1) {
		switch (opt) {
		case 'p':
			port = atoi(optarg);
			break;
		case 's':
			serve_flags |= SERVE_SPLICE;
			break;
		default:
			goto usage;
		}
	}

	if (optind < argc)
		goto usage;

	addr = (struct sockaddr_in) {
		.sin_family = AF_INET,
		.sin_port = port,
	};

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
		err(-1, "socket");

	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)))
		err(-1, "bind");

	do_serve_dgram(sd, serve_flags);
	close(sd);

	return 0;

usage:
	errx(-1, "Usage: %s " USAGE, argv[0]);
}

struct cmd udp_listen_cmd = {
	.name = "udp-listen",
	.main = udp_listen_main,
	.usage = USAGE,
};
