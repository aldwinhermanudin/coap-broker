index:
	gcc -o index.cgi client-asli.c
server-ipc:
	gcc -o server cache.c soc_server.c -lpthread
server-detach:
	gcc -o server-buang cache.c soc_server_detach.c -lpthread
server-abal:
	gcc coap_list.c serv-exp.c sensorNode.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o server-tes
client-abal:
	gcc coap_list.c client-exp.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o client-tes
client-coap:
	gcc coap_list.c client.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coap-client
server-coap:
	gcc coap_list.c coap-server.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coap-server
coap-serv:
	gcc coap_list.c CoAP_server.c sensorNode.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coap-server
