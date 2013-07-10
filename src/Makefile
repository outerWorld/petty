CC=g++
AR=ar
RANLIB=ranlib
CFLAGS=-I../include -g -Wall -fPIC
LFLAGS=-lpthread -lpcap -lcrypto
LIBS=libpetty.so libpetty.a
lib_OBJS=

include Makefile.am
#lib_OBJS=$(subst .cpp, .o, $(lib_FILES))
lib_OBJS=$(lib_FILES:.cpp=.o)

libpetty.so:${lib_OBJS}
	@echo ${lib_OBJS}
	${CC} -shared -o $@ $^ ${LFLAGS}

libpetty.a:${lib_OBJS}
	@echo ${lib_OBJS}
	${AR} -rcs $@ $^

#${lib_OBJS}:${lib_FILES}
%.o:%.cpp
	${CC} -c $< ${CFLAGS}

libs:${LIBS}

.PHONY:clean libs

clean:
	rm -f *.o
	rm -f ${LIBS}
