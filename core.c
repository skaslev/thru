#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "compiler.h"
#include "xrandom.h"
#include "xtime.h"

#define EOM		"EOM"

void do_serve_dgram(int fd)
{
	struct timespec t0, t1, dt;
	char buf[4096];
	ssize_t nread;
	size_t total;

	while (1) {
		total = 0;
		while (1) {
			nread = read(fd, buf, sizeof(buf));
			if (nread < 0) {
				if (errno == EINTR)
					continue;
				err(-1, "read");
			}

			if (unlikely(!total))
				clock_gettime(CLOCK_MONOTONIC, &t0);

			if (nread == 0)
				break;

			total += nread;

			if (nread == sizeof(EOM) &&
			    strncmp(buf, EOM, sizeof(EOM)) == 0)
				break;
		}
		clock_gettime(CLOCK_MONOTONIC, &t1);
		timespec_diff(&t0, &t1, &dt);
		printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));
	}
}

static void log_connected(struct sockaddr *addr)
{
	switch (addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *a = (struct sockaddr_in *)addr;
		printf("connected %s:%d\n",
		       inet_ntoa(a->sin_addr), a->sin_port);
		break;
	}
	case AF_VSOCK: {
		struct sockaddr_vm *a = (struct sockaddr_vm *)addr;
		printf("connected CID: %d port: %d\n",
		       a->svm_cid, a->svm_port);
		break;
	}
	}
}

void do_serve(int sd)
{
	struct timespec t0, t1, dt;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	char buf[4096];
	ssize_t nread;
	size_t total;
	int cd;

	while (1) {
		peer_addr_len = sizeof(peer_addr);
		cd = accept(sd, (struct sockaddr *)&peer_addr, &peer_addr_len);
		if (cd < 0)
			err(-1, "accept");

		log_connected((struct sockaddr *)&peer_addr);

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
		printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

		close(cd);
	}
}

void do_write(int fd, int nr_packets)
{
	struct timespec t0, t1, dt;
	size_t total = 0;
	char buf[4096];
	ssize_t nwrote;
	int i;

	fill_random(buf, sizeof(buf));

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
	printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));
}

void do_write_dgram(int fd, int nr_packets)
{
	do_write(fd, nr_packets);
	if (write(fd, EOM, sizeof(EOM)) < 0)
		err(-1, "write");
}
