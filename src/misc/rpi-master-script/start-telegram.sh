#!/bin/bash

#start only if not running yet
pidof -x python /opt/ngrok/telegram_bot.py > /dev/null
if [ $? -eq 1 ]; then
	while :
        do
            echo "Start telegram bot..."
            python /opt/ngrok/telegram_bot.py
            echo "Running telegram bot..."
            sleep 5
        done
fi
