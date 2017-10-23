#!/usr/bin/env python3

# This file is part of the Python aiocoap library project.
#
# Copyright (c) 2012-2014 Maciej Wasilak <http://sixpinetrees.blogspot.com/>,
#               2013-2014 Christian Amsüss <c.amsuess@energyharvesting.at>
#
# aiocoap is free software, this file is published under the MIT license as
# described in the accompanying LICENSE file.

"""This is a usage example of aiocoap that demonstrates how to implement a
simple client. See the "Usage Examples" section in the aiocoap documentation
for some more information."""
import signal  
import sys  
import logging
import asyncio

from aiocoap import *

async def getTopicData(topic):
    protocol = await Context.create_client_context()

    request = Message(code=GET, uri=broker_uri+topic)
    request.opt.content_format = 0

    try:
        response = await protocol.request(request).response
    except Exception as e:
        return 'error'
    else:
        return response.code,response.payload
        
def sensorHandler(io_object,handler_type,output_value=0,sensor_input=0,sensor_threshold=0,threshold_type = "on"):
    print (io_object,handler_type,output_value,sensor_input,sensor_threshold,threshold_type)
        
async def actuatorHandler(sensor_topic,actuator_topic,io_object, io_object_threshold, handler_type,thres_type):
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
        r_code, r_payload = await getTopicData(topic_path+sensor_topic+'/channel')
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
    sys.exit(0)

loop = asyncio.get_event_loop()  
logging.basicConfig(level=logging.INFO)
relay_one = 1
relay_one_threshold = 32.1
relay_two = 2
relay_two_threshold = 12.3
l298n = 2
l298n_threshold = 12.3
broker_uri = 'coap://192.168.0.100/'
topic_path = 'ps/room10/'

signal.signal(signal.SIGINT, signal_handler)

asyncio.ensure_future(actuatorHandler("lux","relay-one",relay_one, relay_one_threshold, "internal","off"))  
asyncio.ensure_future(actuatorHandler("temperature","relay-two",relay_two, relay_two_threshold, "internal","off"))  
asyncio.ensure_future(actuatorHandler("gas","l298n",l298n, l298n_threshold, "internal","on"))  
asyncio.ensure_future(actuatorHandler("lux","relay-one",relay_one, relay_one_threshold, "external","off"))  
asyncio.ensure_future(actuatorHandler("temperature","relay-two",relay_two, relay_two_threshold, "external","off"))  
asyncio.ensure_future(actuatorHandler("gas","l298n",l298n, l298n_threshold, "external","on"))  
loop.run_forever()  
