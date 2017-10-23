#!/usr/bin/env python3

# This file is part of the Python aiocoap library project.
#
# Copyright (c) 2012-2014 Maciej Wasilak <http://sixpinetrees.blogspot.com/>,
#               2013-2014 Christian Amsüss <c.amsuess@energyharvesting.at>
#
# aiocoap is free software, this file is published under the MIT license as
# described in the accompanying LICENSE file.

import signal 
import RPi.GPIO as GPIO
import time 
import sys  
import logging
import asyncio
import subprocess
import platform
from aiocoap import * 

def controlActuator(io_object, output_value):
    if (type(io_object) is GPIO.PWM):
        if output_value >= 0 and output_value <= 100:
            io_object.ChangeDutyCycle(output_value)

    if (type(io_object) is int):
        if output_value == 1:
            GPIO.output(io_object, GPIO.HIGH)
        if output_value == 0:
            GPIO.output(io_object, GPIO.LOW)  

def sensorHandler(io_object,handler_type,output_value=0,sensor_input=0,sensor_threshold=0,threshold_type = "on"):
    if (handler_type == "internal"):
        print ('handler_type : internal')
        if threshold_type == 'on':
            print ('threshold_type : on')
            if sensor_input > sensor_threshold:
                print ('actuator : on')
                if (type(io_object) is GPIO.PWM):
                    controlActuator(io_object, 100)
                if (type(io_object) is int):
                    controlActuator(io_object, 0)
            else:
                print ('actuator : off')
                if (type(io_object) is GPIO.PWM):
                    controlActuator(io_object, 0)
                if (type(io_object) is int):
                    controlActuator(io_object, 1)
        if threshold_type == 'off':
            print ('threshold_type : off')
            if sensor_input < sensor_threshold:
                print ('actuator : on')
                if (type(io_object) is GPIO.PWM):
                    controlActuator(io_object, 100)
                if (type(io_object) is int):
                    controlActuator(io_object, 0)
            else:
                print ('actuator : off')
                if (type(io_object) is GPIO.PWM):
                    controlActuator(io_object, 0)
                if (type(io_object) is int):
                    controlActuator(io_object, 1)

    if (handler_type == "external"):
        print ('handler_type : external')
        print ('output value :',output_value)
        controlActuator(io_object,output_value)

async def getTopicData(broker_uri, topic):
    protocol = await Context.create_client_context()

    request = Message(code=GET, uri=broker_uri+topic)
    request.opt.content_format = 0

    try:
        response = await protocol.request(request).response
    except Exception as e:
        return 'error'
    else:
        return response.code,response.payload
        
async def actuatorHandler(broker_uri,topic_path, sensor_topic,actuator_topic,io_object, io_object_threshold, handler_type,thres_type):
    protocol = await Context.create_client_context()

    if handler_type == "internal":
        request = Message(code=GET, uri=broker_uri+topic_path+sensor_topic, observe=0)
        
    if handler_type == "external":
        request = Message(code=GET, uri=broker_uri+topic_path+actuator_topic, observe=0)
    request.opt.content_format = 0
	
    pr = protocol.request(request)

    # Note that it is necessary to start sending
    r = await pr.response
    print("First response: %s\n%r"%(r, r.payload))

    async for r in pr.observation:
        r_code, r_payload = await getTopicData(broker_uri,topic_path+sensor_topic+'/channel')
        if (r_payload == handler_type.encode('UTF-8')) :
            try:
                num_payload = float(r.payload)
                print ('Data Received')
                print (r_code,num_payload)
                if handler_type == "internal":
                    sensorHandler(io_object, handler_type,sensor_input=num_payload,sensor_threshold=io_object_threshold,threshold_type=thres_type)
                if handler_type == "external":
                    sensorHandler(io_object, handler_type, num_payload)

            except ValueError:
                print ("Payload is not number or float")
                
def signal_handler(signal, frame):  
    loop.stop() 
    GPIO.cleanup() # cleanup all GPIO
    l298n.stop()
    l298n_gnd.stop()
    sys.exit(0)
    
def ping(ip):
    ping_command = ['ping6', ip, '-c 1']
    shell_needed = False

    ping_output = subprocess.run(ping_command,shell=shell_needed,stdout=subprocess.PIPE)
    success = ping_output.returncode
    return True if success == 0 else False
    
async def createTopic(payload,host): 	
    context = await Context.create_client_context()

    await asyncio.sleep(1)
    payload = payload.encode('UTF-8')
    request = Message(code=POST, payload=payload)
    request.opt.uri_host = host
    request.opt.uri_path = ("ps")
    request.opt.content_format = 40

    response = await context.request(request).response

    print('POST Result: %s \n'%(response))

async def publishToTopic(payload,host,topic: tuple): 	
		
	context = await Context.create_client_context()

	await asyncio.sleep(1)

	payload = payload.encode('UTF-8')
	request = Message(code=PUT, payload=payload)
	request.opt.uri_host = host
	request.opt.uri_path = topic
	request.opt.content_format = 0

	response = await context.request(request).response
	print('Result: %s \n%r'%(response.code, response.payload))
		
async def brokerCheck(broker_host, room_path):
	print ("pinging "+broker_host)
		
	while (not ping(broker_host)):
		pass
	print (broker_host, "is alive")
		
	print ("creating topic...")
	while(True):
		try: 
			await createTopic('<'+room_path+'/temperature/channel>;if="sensor";ct=0;rt="channel"',broker_host)
			await createTopic('<'+room_path+'/lux/channel>;if="sensor";ct=0;rt="channel"',broker_host)
			await createTopic('<'+room_path+'/gas/channel>;if="sensor";ct=0;rt="channel"',broker_host)
			await createTopic('<'+room_path+'/relay-one>;if="actuator";ct=0;rt="digital"',broker_host)
			await createTopic('<'+room_path+'/relay-two>;if="actuator";ct=0;rt="digital"',broker_host)
			await createTopic('<'+room_path+'/l298n>;if="actuator";ct=0;rt="pwm"',broker_host)
			
			await publishToTopic('internal',broker_host,("ps",room_path,"temperature","channel"))
			await publishToTopic('internal',broker_host,("ps",room_path,"lux","channel"))
			await publishToTopic('internal',broker_host,("ps",room_path,"gas","channel"))
				
		except :
			print ("create topic failed")
			continue
		else:
			print ("create topic success")
			break

loop = asyncio.get_event_loop()  
logging.basicConfig(level=logging.INFO)
relay_one = 19
relay_one_threshold = 500
relay_two = 26
relay_two_threshold = 28
l298n_primary = 13
l298n_secondary = 12
l298n_threshold = 30

# Pin Setup:
GPIO.setmode(GPIO.BCM) # Broadcom pin-numbering scheme
GPIO.setup(relay_one, GPIO.OUT) # LED pin set as output
GPIO.setup(relay_two, GPIO.OUT) # PWM pin set as output
GPIO.setup(l298n_primary, GPIO.OUT) # PWM pin set as output
GPIO.setup(l298n_secondary, GPIO.OUT) # PWM pin set as output

# Initial state for LEDs:
GPIO.output(relay_one, GPIO.HIGH)
GPIO.output(relay_two, GPIO.HIGH) 
 
l298n = GPIO.PWM(l298n_primary, 50)  # Initialize PWM on pwmPin 100Hz frequency
l298n_gnd = GPIO.PWM(l298n_secondary, 50)  # Initialize PWM on pwmPin 100Hz frequency
l298n.start(0)
l298n_gnd.start(0)

broker_host = 'fc00::1:1'
broker_uri = 'coap://['+broker_host+']/'
room_name = 'room10'
room_path = 'ps/'+room_name+'/'

signal.signal(signal.SIGINT, signal_handler)

asyncio.get_event_loop().run_until_complete(brokerCheck(broker_host, room_name))
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"lux","relay-one",relay_one, relay_one_threshold, "internal","off"))  
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"temperature","relay-two",relay_two, relay_two_threshold, "internal","off"))  
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"gas","l298n",l298n, l298n_threshold, "internal","on"))  
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"lux","relay-one",relay_one, relay_one_threshold, "external","off"))  
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"temperature","relay-two",relay_two, relay_two_threshold, "external","off"))  
asyncio.ensure_future(actuatorHandler(broker_uri,room_path,"gas","l298n",l298n, l298n_threshold, "external","on"))  
loop.run_forever()  
