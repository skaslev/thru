#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "cmd.h"
#include "core.h"
#include "vsock.h"

#define USAGE	"[-n nr_packets] [-c cid] [-p port]"

static int vsock_main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12, cid = VMADDR_CID_HOST, port = 4242;
	struct sockaddr_vm addr;
	int sd;

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
			goto usage;
		}
	}

	if (optind < argc)
		goto usage;

	printf("local CID: %d\n", get_local_cid());

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

	do_write(sd, nr_packets);
	close(sd);

	return 0;

usage:
	errx(-1, "Usage: %s " USAGE, argv[0]);
}

struct cmd vsock_cmd = {
	.name = "vsock",
	.main = vsock_main,
	.usage = USAGE,
};
