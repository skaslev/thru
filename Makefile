CC = cc
CFLAGS = -O2 -Wall -Winline
AR = ar
LIBS =

#
# For debugging, uncomment the next one
#
# CFLAGS += -O0 -DDEBUG -g3 -gdwarf-2

PROGRAMS = fifo fifo_listen tcp tcp_listen udp udp_listen vsock vsock_listen

LIB_H = compiler.h vsock.h xrandom.h xtime.h
LIB_OBJS =
LIB_FILE = libvsock.a

#
# Pretty print
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_GEN     = $(Q:@=@echo    '     GEN      '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)

all: $(PROGRAMS)

fifo: fifo.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
fifo_listen: fifo_listen.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
tcp: tcp.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
tcp_listen: tcp_listen.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
udp: udp.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
udp_listen: udp_listen.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
vsock: vsock.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)
vsock_listen: vsock_listen.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)

fifo.o: $(LIB_H)
fifo_listen.o: $(LIB_H)
tcp.o: $(LIB_H)
tcp_listen.o: $(LIB_H)
udp.o: $(LIB_H)
udp_listen.o: $(LIB_H)
vsock.o: $(LIB_H)
vsock_listen.o: $(LIB_H)

$(LIB_FILE): $(LIB_OBJS)
	$(QUIET_AR)$(AR) rcs $@ $(LIB_OBJS)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -f *.[oa] *.so $(PROGRAMS) $(LIB_FILE)
