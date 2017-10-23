import logging
import asyncio
import smbus
import time
import Adafruit_MCP3008
import subprocess
import platform
import signal
import sys

from aiocoap import *

logging.basicConfig(level=logging.INFO)

# Get I2C bus
bus = smbus.SMBus(1)

# Software SPI configuration:
CLK  = 6
MISO = 13
MOSI = 19
CS   = 26
mcp = Adafruit_MCP3008.MCP3008(clk=CLK, cs=CS, miso=MISO, mosi=MOSI)

def getTSL2561Data():
	# TSL2561 address, 0x39(57)
	# Select control register, 0x00(00) with command register, 0x80(128)
	#		0x03(03)	Power ON mode
	bus.write_byte_data(0x39, 0x00 | 0x80, 0x03)
	# TSL2561 address, 0x39(57)
	# Select timing register, 0x01(01) with command register, 0x80(128)
	#		0x02(02)	Nominal integration time = 402ms
	bus.write_byte_data(0x39, 0x01 | 0x80, 0x02)

	time.sleep(0.5)

	# Read data back from 0x0C(12) with command register, 0x80(128), 2 bytes
	# ch0 LSB, ch0 MSB
	data = bus.read_i2c_block_data(0x39, 0x0C | 0x80, 2)

	# Read data back from 0x0E(14) with command register, 0x80(128), 2 bytes
	# ch1 LSB, ch1 MSB
	data1 = bus.read_i2c_block_data(0x39, 0x0E | 0x80, 2)

	# Convert the data
	ch0 = data[1] * 256 + data[0]
	ch1 = data1[1] * 256 + data1[0]

	# Output data to screen
	#print "Full Spectrum(IR + Visible) :%d lux" %ch0
	#print "Infrared Value :%d lux" %ch1
	#print "Visible Value :%d lux" %(ch0 - ch1)

	return ch0,ch1,(ch0 - ch1)

def getMQ2Data():
		
	time.sleep(0.5)
	return mcp.read_adc(0)


def ping(ip):

	ping_command = ['ping6', ip, '-c 1']
	shell_needed = False

	ping_output = subprocess.run(ping_command,shell=shell_needed,stdout=subprocess.PIPE)
	success = ping_output.returncode
	return True if success == 0 else False
    
def signal_handler(signal, frame):  
	sys.exit(0)
    
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

async def publishToTopic(sensor_data,host,room,topic, max_age_opt): 	
		
		context = await Context.create_client_context()

		await asyncio.sleep(1)

		payload = "{:.2f}".format(sensor_data)
		payload = payload.encode('UTF-8')
		request = Message(code=PUT, payload=payload)
		request.opt.uri_host = host
		request.opt.uri_path = ("ps",room,topic)
		request.opt.content_format = 0
		max_age = optiontypes.UintOption(14, value=max_age_opt)
		request.opt.add_option(max_age)

		response = await context.request(request).response

		print(topic+' Result: %s with value of %.2f \n%r'%(response.code, sensor_data, response.payload))

async def main():
	max_age_value = 10 	
	broker_host = 'fc00::1:1'
	room_path = "room10"

	signal.signal(signal.SIGINT, signal_handler)
	
	print ("pinging "+broker_host)
	
	while (not ping(broker_host)):
		pass
	print (broker_host, "is alive")
	
	print ("creating topic...")
	while(True):
		try:
			await createTopic('<'+room_path+'>;if="sensor";ct=40;rt="sensor-count"',broker_host)
			await createTopic('<'+room_path+'/lux>;if="sensor";ct=0;rt="candela"',broker_host)
			await createTopic('<'+room_path+'/gas>;if="sensor";ct=0;rt="analog"',broker_host)
		except :
			print ("create topic failed")
			continue
		else:
			print ("create topic success")
			break
				
	while(True):
		try:
			# Receive lux from TSL2561
			fullSpec,infraSpec, visibleSpec = getTSL2561Data()
			
			# Receive gas data from MQ2 through MCP3008
			mq2Analog = getMQ2Data() 
			
			# Lux
			await publishToTopic(fullSpec,broker_host,room_path, "lux", max_age_value)
			
			# MQ2 Gas Sensor		
			await publishToTopic(mq2Analog,broker_host,room_path, "gas", max_age_value) 
		
		except :
			print ("error occured")
			continue		
		
if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())

