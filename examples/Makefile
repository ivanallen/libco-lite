CC=gcc
CFLAGS=-g -I../include -Wno-int-to-pointer-cast
LFLAGS=-L../src/libs -lpthread -lco-lite

all:demo01 demo02 demo03

demo01:demo01.c ../src/libs/libco-lite.a
	${CC} ${CFLAGS} -o $@ $^ ${LFLAGS} 
demo02:demo02.c ../src/libs/libco-lite.a
	${CC} ${CFLAGS} -o $@ $^ ${LFLAGS} 
demo03:demo03.c ../src/libs/libco-lite.a
	${CC} ${CFLAGS} -o $@ $^ ${LFLAGS} 

clean:
	rm demo01 demo02 demo03

.PHONY:all

.PHONY:clean
