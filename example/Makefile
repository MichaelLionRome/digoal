BIN_PATH=../bin


all: example1.o example2.o example3.o

example1.o: example1.c
	$(CC) ${CFLAGS}  example1.c -o example1.o ${INCLUDES} ${BIN}

example2.o: example2.c
	$(CC) ${CFLAGS}  example2.c -o example2.o ${INCLUDES} ${BIN}

example3.o: example3.c
	$(CC) ${CFLAGS}  example3.c -o example3.o ${INCLUDES} ${BIN}

clean:
	rm -f example1.o example2.o example3.o

