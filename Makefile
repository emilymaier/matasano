CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -g3

all: main.o rijndael.o util.o
	${CC} -o matasano main.o rijndael.o util.o

main.o: main.c
	${CC} ${CFLAGS} -c main.c

rijndael.o: rijndael.c
	${CC} ${CFLAGS} -c rijndael.c

util.o: util.c
	${CC} ${CFLAGS} -c util.c
