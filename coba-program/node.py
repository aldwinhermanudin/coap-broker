import serial
import struct
import sys
ser = serial.Serial('/dev/ttyUSB0', 9600)
pin = int(sys.argv[2])
val = int(sys.argv[3])
if sys.argv[1] == 'GPIO':
	interface = 0
	ser.write(struct.pack('>BBB', interface, pin, val))
elif sys.argv[1] == 'ADC':
	interface = 1
	val = 0
	ser.write(struct.pack('>BBB', interface, pin, val))
	res = ser.readline()
	sys.stdout.write(res)
elif sys.argv[1] == 'PWM':
	interface = 2
	ser.write(struct.pack('>BBB', interface, pin, val))
