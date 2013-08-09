CC=g++
CFLAGS=-I../include -g
LFLAGS=-L../lib -lpetty -lpcap -lcrypto
BINS=getarp getpkt_ip test_dupchk test_tlog test_xcap test_daemon test_radius_dpi test_aes test_cmd test_socket test_heapsort
RANLIB=ranlib

getarp:test_arp.o
	${CC} -o $@ $^ ${LFLAGS}

getpkt_ip:test_pkt_ip.o
	${CC} -o $@ $^ ${LFLAGS}

test_dupchk:test_dupchk.o
	${CC} -o $@ $^ ${LFLAGS}

test_heapsort:test_heapsort.o
	${CC} -o $@ $^ ${LFLAGS}

test_socket:test_socket.o
	${CC} -o $@ $^ ${LFLAGS}

test_getopt:test_getopt.o
	${CC} -o $@ $^ ${LFLAGS}

test_cmd:test_cmd.o
	${CC} -o $@ $^ ${LFLAGS}

test_aes:test_aes.o
	${CC} -o $@ $^ ${LFLAGS}

test_daemon:test_daemon.o
	${CC} -o $@ $^ ${LFLAGS}

test_radius_dpi:test_radius_dpi.o
	${CC} -o $@ $^ ${LFLAGS}

test_tlog:test_tlog.o
	${CC} -o $@ $^ ${LFLAGS}

test_xcap:test_xcap.o 
	${CC} -o $@ $^ ${LFLAGS}

%.o:%.cpp
	${CC} -c $< ${CFLAGS}

tests:${BINS}

.PHONY:clean tests

clean:
	rm -f *.o
	rm -f ${BINS}
