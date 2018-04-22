all:demo01 demo02 demo03
demo01:demo01.c coroutine.c coroutine.h sched.c switch.s cosyscall.c
	gcc -g demo01.c coroutine.c sched.c switch.S cosyscall.c -o demo01 -lpthread
demo02:demo02.c
	gcc -g demo02.c -o demo02
demo03:demo03.c coroutine.c sched.c switch.S cosyscall.c cosyscall.h coeventloop.c
	gcc -g demo03.c coroutine.c sched.c switch.S cosyscall.c coeventloop.c -o demo03

.PHONY:all
