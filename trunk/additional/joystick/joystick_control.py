import pygame
from pygame import locals

pygame.init()

pygame.joystick.init() # main joystick device system

try:
	j = pygame.joystick.Joystick(0) # create a joystick instance
	j.init() # init instance
	print 'Enabled joystick: ' + j.get_name()
except pygame.error:
	print 'no joystick found.'




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



# Mainloop

while 1:
	for e in pygame.event.get(): # iterate over event stack
		print 'event : ' + str(e.type)
		if e.type == pygame.locals.JOYAXISMOTION: # 7
			x , y = j.get_axis(0), j.get_axis(1)
			print 'x and y : ' + str(x) +' , '+ str(y)
			
			sock.send("set_param joystick_x # %f \n" % (x))

			#print 'read from tcp1'

			try:
				data=sock.recv(1000)
			except:
				print 'nothing to read'

			sock.send("set_param joystick_y # %f \n" % (y))

			#print 'read from tcp2'

			try:
				data=sock.recv(1000)
			except:
				print 'nothing to read'


		elif e.type == pygame.locals.JOYBALLMOTION: # 8
			print 'ball motion'
		elif e.type == pygame.locals.JOYHATMOTION: # 9
			print 'hat motion'
		elif e.type == pygame.locals.JOYBUTTONDOWN: # 10
			print 'button down'
		elif e.type == pygame.locals.JOYBUTTONUP: # 11
			print 'button up'



