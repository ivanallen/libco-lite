OBJ_DIR=objs
LIB_DIR=libs

SRC=coroutine.c sched.c util.c cosyscall.c coeventloop.c
ASRC=switch.S
OBJ=$(patsubst %.c, ${OBJ_DIR}/%.o, ${SRC})
AOBJ=${OBJ_DIR}/switch.o

CC=gcc
CFLAGS=-g -I../include

${LIB_DIR}/libco-lite.a:${OBJ} ${AOBJ}
	ar -r $@ ${OBJ} ${AOBJ}

${OBJ_DIR}/%.o:%.c
	${CC} ${CFLAGS} -o $@ -c $<

${OBJ_DIR}/%.o:%.S
	${CC} ${CFLAGS} -o $@ -c $<

clean:
	rm objs/*.o libs/*.a

.PHONY:clean

