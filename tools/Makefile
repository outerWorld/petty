CC=g++
CFLAGS=-I/usr/local/include -g
LFLAGS=-L/usr/local/lib -lpetty -lpcap
#LFLAGS=
BINS=pkt_log dev_test_st

#pkt_log:pkt_log.o /usr/local/lib/libpetty.a /usr/local/lib/libpcap.a
pkt_log:pkt_log.o
	${CC} -o $@ $^ ${LFLAGS}

dev_test_st:dev_test_st.o
	${CC} -o $@ $^ ${LFLAGS}

%.o:%.cpp
	${CC} -c $< ${CFLAGS}

all:${BINS}

.PHONY:clean all

clean:
	rm -f *.o
	rm -f ${BINS}
