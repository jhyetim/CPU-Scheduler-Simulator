CC = gcc

CFLAGS += -std=c99
CFLAGS += -Wall -g

main: schsim.c
	${CC} ${CFLAGS} -o schsim schsim.c

clean:
	-rm -f schsim
