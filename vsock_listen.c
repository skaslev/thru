#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "vsock.h"
#include "xtime.h"

#define USAGE		"Usage: %s [-p port]"

int main(int argc, char **argv)
{
	int opt, port = 4242;
	struct sockaddr_vm addr, peer_addr;
	socklen_t peer_addr_len;
	struct timespec t0, t1, dt;
	char buf[4096];
	ssize_t nread;
	size_t total;
	int sd, cd;

	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
		case 'p':
			port = atoi(optarg);
			break;
		default:
			errx(-1, USAGE, argv[0]);
		}
	}

	printf("local CID: %d\n", get_local_cid());

	addr = (struct sockaddr_vm) {
		.svm_family = AF_VSOCK,
		.svm_cid = VMADDR_CID_ANY,
		.svm_port = port,
	};

	sd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sd < 0)
		err(-1, "socket");

	if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)))
		err(-1, "bind");

	if (listen(sd, SOMAXCONN))
		err(-1, "listen");

	while (1) {
		peer_addr_len = sizeof(peer_addr);
		cd = accept(sd, (struct sockaddr *)&peer_addr, &peer_addr_len);
		if (cd < 0)
			err(-1, "accept");

		printf("connected CID: %d port: %d\n",
		       peer_addr.svm_cid, peer_addr.svm_port);

		total = 0;
		clock_gettime(CLOCK_MONOTONIC, &t0);
		while (1) {
			nread = read(cd, buf, sizeof(buf));
			if (nread < 0) {
				if (errno == EINTR)
					continue;
				err(-1, "read");
			}
			if (nread == 0)
				break;

			total += nread;
		}
		clock_gettime(CLOCK_MONOTONIC, &t1);
		timespec_diff(&t0, &t1, &dt);
		printf("read %f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

		close(cd);
	}

	return 0;
}
