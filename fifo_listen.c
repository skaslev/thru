#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "compiler.h"
#include "xtime.h"

#define EOF_MSG		"EOF"

int main(int argc, char **argv)
{
	struct timespec t0, t1, dt;
	char buf[4096];
	ssize_t nread;
	size_t total;
	int fd;

	fd = open("/var/lib/test/fifo0.out", O_RDONLY);
	if (fd < 0)
		err(-1, "open");

	while (1) {
		total = 0;
		while (1) {
			nread = read(fd, buf, sizeof(buf));
			if (nread < 0) {
				if (errno == EINTR)
					continue;
				err(-1, "read");
			}
			if (nread == 0)
				break;

			if (unlikely(!total))
				clock_gettime(CLOCK_MONOTONIC, &t0);
			total += nread;

			if (nread == sizeof(EOF_MSG) &&
			    strncmp(buf, EOF_MSG, sizeof(EOF_MSG)) == 0)
				break;
		}
		clock_gettime(CLOCK_MONOTONIC, &t1);
		timespec_diff(&t0, &t1, &dt);
		printf("read %f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));
	}

	close(fd);

	return 0;
}
