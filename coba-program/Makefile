client:
	gcc coap_list.c client.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coap-client
server:
	gcc coap_list.c server.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o coap-server
serial-prog:
	gcc -o serProg sensorNode.c serial.c
server-abal:
	gcc coap_list.c serv-exp.c sensorNode.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o server-tes
client-abal:
	gcc coap_list.c client-exp.c -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX -lcoap-1 -g -o client-tes
