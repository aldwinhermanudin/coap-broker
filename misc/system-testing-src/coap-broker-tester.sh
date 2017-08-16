#!/bin/sh

#broker_ip="rpibroker"
broker_ip="127.0.0.1"
EXIT_STATUS=no
if [ -z "$2" ]
  then
    echo "No argument supplied"
    EXIT_STATUS=yes
fi

if [ "$1" = "--make-topic" ] && [ "$EXIT_STATUS" = "no" ]  ; then
	
	  echo "making 10 topics....."
	   for i in `seq 1 $2`;
        do
              coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<room'$i'>;if="sensor";ct=40;rt="sensor-count"' coap://["$broker_ip"]/ps 
			  coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<temperature>;if="sensor";ct=0;rt="temperature-c"' coap://["$broker_ip"]/ps/room$i
			  coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<lux>;if="sensor";ct=0;rt="candela"' coap://["$broker_ip"]/ps/room$i 
			  coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<humidity>;if="sensor";ct=0;rt="percent"' coap://["$broker_ip"]/ps/room$i 
			  coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<pressure>;if="actuator";ct=0;rt="percent"' coap://["$broker_ip"]/ps/room$i 
			  coap-client -m post -t 40 -O 14,`shuf -i 20-100 -n 1` -e '<gas>;if="sensor";ct=0;rt="analog"' coap://["$broker_ip"]/ps/room$i 
			 # sleep 1s
        done    
	  	  
	  EXIT_STATUS=yes
	  exit 1
fi

if [ "$1" = "--resource-check-make" ] && [ "$EXIT_STATUS" = "no" ]  ; then
	
	  echo "making 10 topics....."
	   for i in `seq 1 $2`;
        do
              coap-client -m post -t 0 -e 'room'$i'' coap://["$broker_ip"]/ps
        done    
	  	  
	  EXIT_STATUS=yes
	  exit 1
fi

if [ "$1" = "--resource-check-delete" ] && [ "$EXIT_STATUS" = "no" ]  ; then
	
	  echo "making 10 topics....."
	   for i in `seq 1 $2`;
        do
              coap-client -m delete coap://["$broker_ip"]/room$i
        done    
	  	  
	  EXIT_STATUS=yes
	  exit 1
fi

if [ "$1" = "--delete-topic" ]  && [ "$EXIT_STATUS" = "no" ]; then
	 
	  echo "deleting 10 topics....."
	   for i in `seq 1 $2`;
        do
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i/temperature
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i/lux
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i/humidity
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i/pressure 
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i/gas 
			  coap-client -m delete coap://["$broker_ip"]/ps/room$i
        done    
	  	  
	  EXIT_STATUS=yes
	  exit 1
fi
#if [ "$EXIT_STATUS" = "no" ] ; then
	
#fi
