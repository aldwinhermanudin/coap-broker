import sys
import serial

ser = serial.Serial('/dev/ttyUSB1', 9600)
ser.write(sys.argv[1])
while 1:
	serial_line = ser.readline()
	print(serial_line) # If using Python 2.x use: print serial_line
ser.close()	

