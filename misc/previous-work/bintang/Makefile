all:
	gcc -o server.out server.c

server-6lo:
	gcc -o server-6lo.out server-6lo.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)

coapserver-bintang:
	gcc coapserver-bintang.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coapserver-bintang

client-coap:
	gcc coap_list.c client-coap.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o client-coap
