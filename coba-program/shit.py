import serial
import sys
ser = serial.Serial('/dev/ttyUSB1', 9600)
b = int(sys.argv[2])
c = int(sys.argv[3])
if sys.argv[1] == 'ADC':
	a = 0
	d = (a << 12) | (b << 8)
	ser.write(str(123))
	res = ser.readline()
	sys.stdout.write(res)
elif sys.argv[1] == 'PWM':
	a = 2
	d = (a << 12) | (b << 8) | c
	ser.write(str(d))
elif sys.argv[1] == 'GPIO':
	a = 1
	d = (a << 12) | (b << 8) | c
	ser.write(str(d))
