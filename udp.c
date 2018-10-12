#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "xrandom.h"
#include "xtime.h"

#define EOF_MSG		"EOF"
#define USAGE		"Usage: %s [-n nr_packets] [-h host] [-p port] [-d]"

int main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12, port = 4242, dont_route = 0;
	const char *hostname = "localhost";
	struct timespec t0, t1, dt;
	struct sockaddr_in addr;
	struct hostent *host;
	size_t total = 0;
	char buf[4096];
	ssize_t nwrote;
	int sd, i;

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
			errx(-1, USAGE, argv[0]);
		}
	}

	fill_random(buf, sizeof(buf));

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

	if (dont_route) {
		if (setsockopt(sd, SOL_SOCKET, SO_DONTROUTE,
			       &dont_route, sizeof(dont_route)))
			err(-1, "setsockopt");
	}

	if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)))
		err(-1, "connect");

	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (i = 0; i < nr_packets; i++) {
		nwrote = write(sd, buf, sizeof(buf));
		if (nwrote < 0) {
			if (errno == EINTR)
				continue;
			err(-1, "write");
		}
		if (nwrote == 0)
			break;

		total += nwrote;

		if (nwrote != sizeof(buf))
			errx(-1, "write clamped size");
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	timespec_diff(&t0, &t1, &dt);
	printf("wrote %f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

	if (write(sd, EOF_MSG, sizeof(EOF_MSG)) < 0)
		err(-1, "write");

	close(sd);

	return 0;
}
