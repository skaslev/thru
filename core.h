#ifndef CORE_H_
#define CORE_H_

#define SERVE_SPLICE	0x1u

void do_serve(int fd, unsigned flags);
void do_serve_dgram(int fd, unsigned flags);

void do_write(int fd, int nr_packets);
void do_write_dgram(int fd, int nr_packets);

#endif // CORE_H_
