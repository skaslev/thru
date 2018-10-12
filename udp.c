#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cmd.h"
#include "core.h"

#define USAGE	"[-n nr_packets] [-h host] [-p port] [-d]"

static int udp_main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12, port = 4242, dont_route = 0;
	const char *hostname = "localhost";
	struct sockaddr_in addr;
	struct hostent *host;
	int sd;

	while ((opt = getopt(argc, argv, "n:h:p:d")) != -1) {
		switch (opt) {
		case 'n':
			nr_packets = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'h':
			hostname = optarg;
			break;
		case 'd':
			dont_route = 1;
			break;
		default:
			errx(-1, "Usage: %s " USAGE, argv[0]);
		}
	}

	host = gethostbyname(hostname);
	if (!host)
		err(-1, "gethostbyname");

	addr = (struct sockaddr_in) {
		.sin_family = AF_INET,
		.sin_port = port,
		.sin_addr = *(struct in_addr *)host->h_addr,
	};

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
		err(-1, "socket");

	if (dont_route &&
	    setsockopt(sd, SOL_SOCKET, SO_DONTROUTE,
		       &dont_route, sizeof(dont_route)))
		err(-1, "setsockopt");

	if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)))
		err(-1, "connect");

	do_write_dgram(sd, nr_packets);
	close(sd);

	return 0;
}

struct cmd udp_cmd = {
	.name = "udp",
	.main = udp_main,
	.usage = USAGE,
};
