CC = cc
CFLAGS = -O2 -Wall -Winline
AR = ar
LIBS =

#
# For debugging, uncomment the next one
#
# CFLAGS += -O0 -DDEBUG -g3 -gdwarf-2

PROGRAMS = thru

LIB_H = compiler.h core.c vsock.h xrandom.h xtime.h
LIB_OBJS = core.o fifo.o tcp.o udp.o vsock.o \
 fifo_listen.o tcp_listen.o udp_listen.o vsock_listen.o
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

thru: thru.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)

thru.o: $(LIB_H)
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
