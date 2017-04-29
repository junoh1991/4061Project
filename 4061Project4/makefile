CC = gcc
CFLAGS = -D_REENTRANT
LDFLAGS = -lpthread

web_server: server.c
	${CC} -o web_server server.c util.o ${LDFLAGS}

clean:
	rm -Rf web_server webserver_log

