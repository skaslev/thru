#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

#include "core.h"
#include "compiler.h"
#include "xrandom.h"
#include "xtime.h"

#define USE_MEMFD
/* #define DEBUG */

#ifdef DEBUG
#define pr(...)	printf(__VA_ARGS__)
#else
#define pr(...)
#endif

#define EOM		"EOM"
#define BUF_SIZE	4096

static void do_serve_dgram_splice(int fd)
{
	struct timespec t0, t1, dt;
	ssize_t nread, ret;
	loff_t md_off = 0;
	int md, brass[2];
	size_t total;
	int i __attribute__((unused)) = 0;

#ifdef USE_MEMFD
	md = memfd_create("buf", 0);
	if (md < 0)
		err(-1, "memfd_create");
#else
	md = open("foo", O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (md < 0)
		err(-1, "open");
#endif

	if (pipe(brass))
		err(-1, "pipe");

	total = 0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	while (1) {
		loff_t save_off = md_off;

		nread = splice(fd, NULL, brass[1], NULL, PIPE_BUF, SPLICE_F_MOVE);
		pr("s1[%d]: %ld\n", i++, nread);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			err(-1, "splice");
		}
		if (nread == 0)
			break;

		total += nread;

	again:
		ret = splice(brass[0], NULL, md, &md_off, nread, 0);
		pr("s2[%d]: %ld\n", i, ret);
		if (ret < 0) {
			if (errno == EINTR)
				goto again;
			err(-1, "splice");
		}
		nread -= ret;
		if (nread)
			goto again;

		if (md_off - save_off == sizeof(EOM)) {
			char buf[sizeof(EOM)];

			if (pread(md, buf, sizeof(buf), save_off) != sizeof(buf))
				err(-1, "pread");

			buf[sizeof(buf)-1] = '\0';
			if (strcmp(buf, EOM) == 0)
				break;
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	timespec_diff(&t0, &t1, &dt);
	printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

	close(brass[0]);
	close(brass[1]);
	close(md);
}

static void do_serve_dgram_read(int fd)
{
	struct timespec t0, t1, dt;
	char buf[BUF_SIZE];
	ssize_t nread;
	size_t total;
	int i;

	while (1) {
		total = i = 0;
		while (1) {
			nread = read(fd, buf, sizeof(buf));
			pr("r[%d]: %ld\n", i++, nread);
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

void do_serve_dgram(int fd, unsigned flags)
{
	while (1) {
		if (flags & SERVE_SPLICE)
			do_serve_dgram_splice(fd);
		else
			do_serve_dgram_read(fd);
	}
}

static void log_connected(struct sockaddr *addr)
{
	char buf[INET6_ADDRSTRLEN];

	switch (addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *a = (struct sockaddr_in *)addr;
		printf("connected %s:%d\n",
		       inet_ntop(a->sin_family, &a->sin_addr, buf, sizeof(buf)),
		       a->sin_port);
		break;
	}
	case AF_INET6: {
		struct sockaddr_in6 *a = (struct sockaddr_in6 *)addr;
		printf("connected %s:%d\n",
		       inet_ntop(a->sin6_family, &a->sin6_addr, buf, sizeof(buf)),
		       a->sin6_port);
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

static void do_read(int fd)
{
	struct timespec t0, t1, dt;
	char buf[BUF_SIZE];
	ssize_t nread;
	size_t total;
	int md;
	int i __attribute__((unused)) = 0;

#ifdef USE_MEMFD
	md = memfd_create("buf", 0);
	if (md < 0)
		err(-1, "memfd_create");
#else
	md = open("foo", O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (md < 0)
		err(-1, "open");
#endif

	total = 0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	while (1) {
		nread = read(fd, buf, sizeof(buf));
		pr("r[%d]: %ld\n", i++, nread);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			err(-1, "read");
		}
		if (nread == 0)
			break;

		total += nread;

		if (write(md, buf, nread) != nread)
			err(-1, "write");
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	timespec_diff(&t0, &t1, &dt);
	printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

	close(md);
}

static void do_splice_read(int fd)
{
	struct timespec t0, t1, dt;
	ssize_t nread, ret;
	size_t total;
	int md, brass[2];
	int i __attribute__((unused)) = 0;

#ifdef USE_MEMFD
	md = memfd_create("buf", 0);
	if (md < 0)
		err(-1, "memfd_create");
#else
	md = open("foo", O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (md < 0)
		err(-1, "open");
#endif

	if (pipe(brass))
		err(-1, "pipe");

	total = 0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	while (1) {
		nread = splice(fd, NULL, brass[1], NULL, PIPE_BUF, SPLICE_F_MOVE);
		pr("s1[%d]: %ld\n", i++, nread);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			err(-1, "splice");
		}
		if (nread == 0)
			break;

		total += nread;

	again:
		ret = splice(brass[0], NULL, md, NULL, nread, 0);
		pr("s2[%d]: %ld\n", i, ret);
		if (ret < 0) {
			if (errno == EINTR)
				goto again;
			err(-1, "splice");
		}
		nread -= ret;
		if (nread)
			goto again;
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	timespec_diff(&t0, &t1, &dt);
	printf("%f MB/s\n", total / timespec_sec(&dt) / (1024.0 * 1024.0));

	close(brass[0]);
	close(brass[1]);
	close(md);
}

void do_serve(int sd, unsigned flags)
{
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	int cd;

	while (1) {
		peer_addr_len = sizeof(peer_addr);
		cd = accept(sd, (struct sockaddr *)&peer_addr, &peer_addr_len);
		if (cd < 0)
			err(-1, "accept");

		log_connected((struct sockaddr *)&peer_addr);
		if (flags & SERVE_SPLICE)
			do_splice_read(cd);
		else
			do_read(cd);
		close(cd);
	}
}

void do_write(int fd, int nr_packets)
{
	struct timespec t0, t1, dt;
	size_t total = 0;
	char buf[BUF_SIZE];
	ssize_t nwrote;
	int i;

	/* fill_random(buf, sizeof(buf)); */
	memset(buf, 'h', sizeof(buf));

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
