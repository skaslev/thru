#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "cmd.h"
#include "core.h"
#include "vsock.h"

#define USAGE		"[-p port]"

static int vsock_listen_main(int argc, char **argv)
{
	int opt, port = 4242;
	struct sockaddr_vm addr;
	int sd;

	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
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

	do_serve(sd);
	close(sd);

	return 0;

usage:
	errx(-1, "Usage: %s " USAGE, argv[0]);
}

struct cmd vsock_listen_cmd = {
	.name = "vsock-listen",
	.main = vsock_listen_main,
	.usage = USAGE,
};
