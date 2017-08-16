#!/bin/bash

#start only if not running yet
pidof -x ngrok > /dev/null
if [ $? -eq 1 ]; then
	#while :
        #do
            echo "Start tunnelling..."
            /opt/ngrok/ngrok tcp --log stdout -region ap 22 > /dev/null &
            echo "Running tunnel in background..."
           # sleep 5
       # done
fi
