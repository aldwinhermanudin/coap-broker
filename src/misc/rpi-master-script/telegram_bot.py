import time
import random
import datetime
import telepot
import subprocess
import requests

"""
After **inserting token** in the source code, run it:

```
$ python2.7 diceyclock.py
```

[Here is a tutorial](http://www.instructables.com/id/Set-up-Telegram-Bot-on-Raspberry-Pi/)
teaching you how to setup a bot on Raspberry Pi. This simple bot does nothing
but accepts two commands:

- `/roll` - reply with a random integer between 1 and 6, like rolling a dice.
- `/time` - reply with the current time, like a clock.
"""

def handle(msg):
    chat_id = msg['chat']['id']
    command = msg['text']

    print 'Got command: %s' % command

    if command == '/ngrok_start':
		subprocess.Popen(["nohup", "/opt/ngrok/start.sh"])
		bot.sendMessage(chat_id, 'Starting tunnel....')
		time.sleep(10)
		r = requests.get('http://localhost:4040/api/tunnels')
		reply = "Your tunnel URL is " + str(r.json()['tunnels'][0]['public_url'])
		bot.sendMessage(chat_id, reply)
		
    elif command == '/ngrok_stop':
        subprocess.Popen(["nohup", "/opt/ngrok/terminator.sh"])
        bot.sendMessage(chat_id, 'Tunnel Killed')
    elif command == '/time':
        bot.sendMessage(chat_id, str(datetime.datetime.now()))
    elif command == 'waddup':
        bot.sendMessage(chat_id, "Good man, thanks.")
    elif command == '/ngrok_url':
		try:
			r = requests.get('http://localhost:4040/api/tunnels')
			reply = "Your tunnel URL is " + str(r.json()['tunnels'][0]['public_url'])
			bot.sendMessage(chat_id, reply)
		
		except requests.exceptions.RequestException as e:
			#bot.sendMessage(chat_id, str(format(e)))
			bot.sendMessage(chat_id, 'An error occur. Probably tunnel isn\'t running.')

bot = telepot.Bot('376465837:AAGNeFri5-taeC1sPiqoPkPomTGzcG-NSlQ')
bot.message_loop(handle)
print 'I am listening ...'

while 1:
    time.sleep(10)

