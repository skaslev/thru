#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "compiler.h"
#include "xtime.h"

#define EOF_MSG		"EOF"
#define USAGE		"Usage: %s [-p port]"

int main(int argc, char **argv)
{
	int opt, port = 4242;
	struct sockaddr_in addr;
	struct timespec t0, t1, dt;
	char buf[4096];
	ssize_t nread;
	size_t total;
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

	while (1) {
		total = 0;
		while (1) {
			nread = read(sd, buf, sizeof(buf));
			if (nread < 0) {
				if (errno == EINTR)
					continue;
				err(-1, "read");
			}

			if (unlikely(!total))
				clock_gettime(CLOCK_MONOTONIC, &t0);

			if (nread == 0)
				break;

			total += nread;

			if (unlikely(nread == sizeof(EOF_MSG)) &&
			    strncmp(buf, EOF_MSG, sizeof(EOF_MSG)) == 0)
				break;
		}
		clock_gettime(CLOCK_MONOTONIC, &t1);
		timespec_diff(&t0, &t1, &dt);
		printf("read %f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));
	}

	close(sd);

	return 0;
}
