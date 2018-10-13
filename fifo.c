#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "cmd.h"
#include "core.h"

#define USAGE	"[-n nr_packets] [-f fifo]"

static int fifo_main(int argc, char **argv)
{
	const char *filename = "/dev/virtio-ports/fifo0";
	int opt, nr_packets = 1 << 12;
	int fd;

	while ((opt = getopt(argc, argv, "n:f:")) != -1) {
		switch (opt) {
		case 'n':
			nr_packets = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		default:
			goto usage;
		}
	}

	if (optind < argc)
		goto usage;

	fd = open(filename, O_WRONLY);
	if (fd < 0)
		err(-1, "open");

	do_write_dgram(fd, nr_packets);
	close(fd);

	return 0;

usage:
	errx(-1, "Usage: %s " USAGE, argv[0]);
}

struct cmd fifo_cmd = {
	.name = "fifo",
	.main = fifo_main,
	.usage = USAGE,
};
