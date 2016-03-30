all:
	gcc -o server.out server.c

server-6lo:
	gcc -o server-6lo.out server-6lo.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)
