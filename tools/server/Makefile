all:./build cli upper reverse

cli:./build
	gcc -o ./build/cli cli.c

upper:./build
	gcc -o ./build/upper upper.c

reverse:./build
	gcc -o ./build/reverse reverse.c

build:
	mkdir build

clean:
	rm ./build/* 
