main:main.c coroutine.c coroutine.h sched.c switch.s
	gcc -g main.c coroutine.c sched.c switch.s -o main
