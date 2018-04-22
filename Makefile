all:libco-lite demo 
libco-lite:./src
	${MAKE} -C ./src
demo:./examples
	${MAKE} -C ./examples
.PHONY:all
