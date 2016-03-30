all:
	gcc -o server.out server.c

server-6lo:
	gcc scan_access_points.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)
