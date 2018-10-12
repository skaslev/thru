#ifndef CORE_H_
#define CORE_H_

void do_serve(int fd);
void do_serve_dgram(int fd);

void do_write(int fd, int nr_packets);
void do_write_dgram(int fd, int nr_packets);

#endif // CORE_H_
