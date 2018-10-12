#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmd.h"

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

extern struct cmd fifo_cmd,
	fifo_listen_cmd,
	tcp_cmd,
	tcp_listen_cmd,
	udp_cmd,
	udp_listen_cmd,
	vsock_cmd,
	vsock_listen_cmd;

static struct cmd *cmds[] = {
	&fifo_cmd,
	&fifo_listen_cmd,
	&tcp_cmd,
	&tcp_listen_cmd,
	&udp_cmd,
	&udp_listen_cmd,
	&vsock_cmd,
	&vsock_listen_cmd,
};

static void usage()
{
	size_t i;

	printf("Usage: thru cmd\n");
	printf("  where `cmd` can be any of:\n");
	for (i = 0; i < ARRAY_SIZE(cmds); i++)
		printf("\t%s\t\t%s\n", cmds[i]->name, cmds[i]->usage);

	exit(-1);

}

int main(int argc, char **argv)
{
	size_t i;

	if (argc < 2)
		usage();

	for (i = 0; i < ARRAY_SIZE(cmds); i++)
		if (strcmp(argv[1], cmds[i]->name) == 0)
			exit(cmds[i]->main(argc - 1, argv + 1));

	usage();

	return 0;
}
