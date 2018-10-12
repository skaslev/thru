#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "core.h"

#define USAGE		"Usage: %s [-n nr_packets]"

int main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12;
	int fd;

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		switch (opt) {
		case 'n':
			nr_packets = atoi(optarg);
			break;
		default:
			errx(-1, USAGE, argv[0]);
		}
	}

	fd = open("/dev/virtio-ports/fifo0", O_WRONLY);
	if (fd < 0)
		err(-1, "open");

	do_write_dgram(fd, nr_packets);
	close(fd);

	return 0;
}
