#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "xtime.h"
#include "xrandom.h"

#define EOF_MSG		"EOF"
#define USAGE		"Usage: %s [-n nr_packets]"

int main(int argc, char **argv)
{
	int opt, nr_packets = 1 << 12;
	struct timespec t0, t1, dt;
	size_t total = 0;
	char buf[4096];
	ssize_t nwrote;
	int fd, i;

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		switch (opt) {
		case 'n':
			nr_packets = atoi(optarg);
			break;
		default:
			errx(-1, USAGE, argv[0]);
		}
	}

	fill_random(buf, sizeof(buf));

	fd = open("/dev/virtio-ports/fifo0", O_WRONLY);
	if (fd < 0)
		err(-1, "open");

	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (i = 0; i < nr_packets; i++) {
		nwrote = write(fd, buf, sizeof(buf));
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

	if (write(fd, EOF_MSG, sizeof(EOF_MSG)) < 0)
		err(-1, "write");

	close(fd);

	return 0;
}
