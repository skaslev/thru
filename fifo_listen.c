#include <err.h>
#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "core.h"

#define USAGE	"[-f fifo]"

static int fifo_listen_main(int argc, char **argv)
{
	const char *filename = "/var/lib/test/fifo0.out";
	int opt, fd;

	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			filename = optarg;
			break;
		default:
			goto usage;
		}
	}

	if (optind < argc)
		goto usage;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		err(-1, "open");

	do_serve_dgram(fd, 0);
	close(fd);

	return 0;

usage:
	errx(-1, "Usage: %s " USAGE, argv[0]);
}

struct cmd fifo_listen_cmd = {
	.name = "fifo-listen",
	.main = fifo_listen_main,
	.usage = USAGE,
};
