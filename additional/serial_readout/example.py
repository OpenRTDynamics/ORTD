#!/usr/bin/env python

import sys
import os
import time
import threading
import socket
import codecs
import serial

from decimal import Decimal

# Open TCP-Socket to rt_server

import socket
import sys
 
HOST = 'localhost'
PORT = 10000
 
try:
  sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, msg:
  sys.stderr.write("[ERROR] %s\n" % msg[1])
  sys.exit(1)
 
try:
  sock.connect((HOST, PORT))
except socket.error, msg:
  sys.stderr.write("[ERROR] %s\n" % msg[1])
  sys.exit(2)
 

sock.setblocking(0)


# Open Serial Port
ser = serial.Serial()
ser.port     = "/dev/force_sensor"
ser.baudrate = 115200
ser.parity   = 'N'
ser.rtscts   = False
ser.xonxoff  = False
#ser.stopbits = STOPBITS_TWO
ser.timeout  = None     # required so that the reader thread can exit

try:
    ser.open()
except serial.SerialException, e:
    sys.stderr.write("Could not open serial port %s: %s\n" % (ser.portstr, e))
    sys.exit(1)


while True:
  ser.write("?")

  #line = ser.readline(size=None, eol='\r\n')

  line = ser.readline()
  force = Decimal(line);
  
  sock.send("set_param force_sensor # %f \n" % (force))
  print force
