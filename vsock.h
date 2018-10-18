#ifndef VSOCK_H_
#define VSOCK_H_

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/vm_sockets.h>

static inline int get_local_cid(void)
{
	int fd, cid = -1;

	fd = open("/dev/vsock", O_RDONLY);
	if (fd < 0)
		return cid;

	ioctl(fd, IOCTL_VM_SOCKETS_GET_LOCAL_CID, &cid);
	close(fd);

	return cid;
}

#endif // VSOCK_H_
