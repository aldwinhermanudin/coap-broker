import logging
import signal
import sys
import asyncio
import smbus
import time
import subprocess
import platform
import Adafruit_DHT

from aiocoap import *

logging.basicConfig(level=logging.INFO)

# Get I2C bus
bus = smbus.SMBus(1)
sensor = Adafruit_DHT.DHT22
pin = 26

operating_sys = platform.system()

def getBMP280Data():
	# BMP280 address, 0x76(118)
	# Read data back from 0x88(136), 24 bytes
	b1 = bus.read_i2c_block_data(0x76, 0x88, 24)

	# Convert the data
	# Temp coefficents
	dig_T1 = b1[1] * 256 + b1[0]
	dig_T2 = b1[3] * 256 + b1[2]
	if dig_T2 > 32767 :
		dig_T2 -= 65536
	dig_T3 = b1[5] * 256 + b1[4]
	if dig_T3 > 32767 :
		dig_T3 -= 65536

	# Pressure coefficents
	dig_P1 = b1[7] * 256 + b1[6]
	dig_P2 = b1[9] * 256 + b1[8]
	if dig_P2 > 32767 :
		dig_P2 -= 65536
	dig_P3 = b1[11] * 256 + b1[10]
	if dig_P3 > 32767 :
		dig_P3 -= 65536
	dig_P4 = b1[13] * 256 + b1[12]
	if dig_P4 > 32767 :
		dig_P4 -= 65536
	dig_P5 = b1[15] * 256 + b1[14]
	if dig_P5 > 32767 :
		dig_P5 -= 65536
	dig_P6 = b1[17] * 256 + b1[16]
	if dig_P6 > 32767 :
		dig_P6 -= 65536
	dig_P7 = b1[19] * 256 + b1[18]
	if dig_P7 > 32767 :
		dig_P7 -= 65536
	dig_P8 = b1[21] * 256 + b1[20]
	if dig_P8 > 32767 :
		dig_P8 -= 65536
	dig_P9 = b1[23] * 256 + b1[22]
	if dig_P9 > 32767 :
		dig_P9 -= 65536

	# BMP280 address, 0x76(118)
	# Select Control measurement register, 0xF4(244)
	#		0x27(39)	Pressure and Temperature Oversampling rate = 1
	#					Normal mode
	bus.write_byte_data(0x76, 0xF4, 0x27)
	# BMP280 address, 0x76(118)
	# Select Configuration register, 0xF5(245)
	#		0xA0(00)	Stand_by time = 1000 ms
	bus.write_byte_data(0x76, 0xF5, 0xA0)

	time.sleep(0.5)

	# BMP280 address, 0x76(118)
	# Read data back from 0xF7(247), 8 bytes
	# Pressure MSB, Pressure LSB, Pressure xLSB, Temperature MSB, Temperature LSB
	# Temperature xLSB, Humidity MSB, Humidity LSB
	data = bus.read_i2c_block_data(0x76, 0xF7, 8)

	# Convert pressure and temperature data to 19-bits
	adc_p = ((data[0] * 65536) + (data[1] * 256) + (data[2] & 0xF0)) / 16
	adc_t = ((data[3] * 65536) + (data[4] * 256) + (data[5] & 0xF0)) / 16

	# Temperature offset calculations
	var1 = ((adc_t) / 16384.0 - (dig_T1) / 1024.0) * (dig_T2)
	var2 = (((adc_t) / 131072.0 - (dig_T1) / 8192.0) * ((adc_t)/131072.0 - (dig_T1)/8192.0)) * (dig_T3)
	t_fine = (var1 + var2)
	cTemp = (var1 + var2) / 5120.0

	# Pressure offset calculations
	var1 = (t_fine / 2.0) - 64000.0
	var2 = var1 * var1 * (dig_P6) / 32768.0
	var2 = var2 + var1 * (dig_P5) * 2.0
	var2 = (var2 / 4.0) + ((dig_P4) * 65536.0)
	var1 = ((dig_P3) * var1 * var1 / 524288.0 + ( dig_P2) * var1) / 524288.0
	var1 = (1.0 + var1 / 32768.0) * (dig_P1)
	p = 1048576.0 - adc_p
	p = (p - (var2 / 4096.0)) * 6250.0 / var1
	var1 = (dig_P9) * p * p / 2147483648.0
	var2 = p * (dig_P8) / 32768.0
	pressure = (p + (var1 + var2 + (dig_P7)) / 16.0) / 100

	return cTemp,pressure

def getDHT22Data():
	

	# Try to grab a sensor reading.  Use the read_retry method which will retry up
	# to 15 times to get a sensor reading (waiting 2 seconds between each retry).
	humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)

	# Note that sometimes you won't get a reading and
	# the results will be null (because Linux can't
	# guarantee the timing of calls to read the sensor).
	# If this happens try again!
	if humidity is not None and temperature is not None:
		return humidity, temperature
	else:
		return -1,-1

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
	
	broker_host = 'fc00::1:1'
	room_path = "room10"
	max_age_value = 10 

	signal.signal(signal.SIGINT, signal_handler)
	
	print ("pinging "+broker_host)
	
	while (not ping(broker_host)):
		pass
	print (broker_host, "is alive")
	
	print ("creating topic...")
	while(True):
		try:
			await createTopic('<'+room_path+'>;if="sensor";ct=40;rt="sensor-count"',broker_host)
			await createTopic('<'+room_path+'/humidity>;if="sensor";ct=0;rt="percent"',broker_host)
			await createTopic('<'+room_path+'/temperature>;if="sensor";ct=0;rt="temperature-c"',broker_host)
			await createTopic('<'+room_path+'/pressure>;if="sensor";ct=0;rt="percent"',broker_host)
		except :
			print ("create topic failed")
			continue
		else:
			print ("create topic success")
			break
				
	while(True):
		try:
			# Receive Temperature and Humidity from DHT22
			humidity,cTempDHT22 = getDHT22Data()
			
			# Receive Temperature and Air Pressure from BMP280
			cTempBMP280,pressure = getBMP280Data()
			
			cTemp = (cTempBMP280+cTempDHT22)/2
			
			# Temperature
			await publishToTopic(cTemp,broker_host,room_path, "temperature", max_age_value)
			
			# Air Pressure		
			await publishToTopic(pressure,broker_host,room_path, "pressure", max_age_value)
				
			# Humidity		
			await publishToTopic(humidity,broker_host,room_path, "humidity", max_age_value)
		
		except :
			print ("error occured")
			continue
		
		
if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
