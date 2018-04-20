main:main.c coroutine.c coroutine.h sched.c switch.s cosyscall.c
	gcc -g main.c coroutine.c sched.c switch.s cosyscall.c -o main -lpthread
