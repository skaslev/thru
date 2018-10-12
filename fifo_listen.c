#include <err.h>
#include <fcntl.h>
#include <unistd.h>

#include "core.h"

int main(int argc, char **argv)
{
	int fd;

	fd = open("/var/lib/test/fifo0.out", O_RDONLY);
	if (fd < 0)
		err(-1, "open");

	do_serve_dgram(fd);

	close(fd);

	return 0;
}
