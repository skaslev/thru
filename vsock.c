#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "vsock.h"
#include "xrandom.h"
#include "xtime.h"

#define USAGE		"Usage: %s [-n nr_packets] [-c cid] [-p port]"

int main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12, cid = VMADDR_CID_HOST, port = 4242;
	struct timespec t0, t1, dt;
	struct sockaddr_vm addr;
	size_t total = 0;
	char buf[4096];
	ssize_t nwrote;
	int sd, i;

	while ((opt = getopt(argc, argv, "n:c:p:")) != -1) {
		switch (opt) {
		case 'n':
			nr_packets = atoi(optarg);
			break;
		case 'c':
			cid = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			errx(-1, USAGE, argv[0]);
		}
	}

	printf("local CID: %d\n", get_local_cid());
	fill_random(buf, sizeof(buf));

	addr = (struct sockaddr_vm) {
		.svm_family = AF_VSOCK,
		.svm_cid = cid,
		.svm_port = port,
	};

	sd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sd < 0)
		err(-1, "socket");

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

	close(sd);

	return 0;
}
