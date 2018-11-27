server:server.c socket.h
	gcc server.c -o server -L/usr/lib/mysql/ -lmysqlclient -lpthread
client:client.c socket.h
	gcc client.c -o client -lpthread
clean:
	rm server client
