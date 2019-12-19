#!/usr/bin/env python

# This script needs to have access to your web server files somehow.

import select
import socket
import sys
import Queue
import os
import re
import smtplib
import string
import time

# Create a TCP/IP socket
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setblocking(0)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind the socket to the port
server_address = ('0.0.0.0', 8090)
echo_address = ('0.0.0.0',8091)

print >>sys.stderr, 'starting up on %s port %s' % server_address
server.bind(server_address)


# Listen for incoming connections
server.listen(5)

# Sockets from which we expect to read
inputs = [ server ]

# Sockets to which we expect to write
outputs = [ ]

# Outgoing message queues (socket:Queue)
message_queues = {}
connected_clients = [ ]
lastgps = ""
newtrack = False
retrycount = 0
retryloop = False
RETRY_ATTEMPTS = 12

 
def isprintable(a):
	stringok = True;
	for each in a:
		if each in string.printable:
			pass
		else:
			stringok = False;
	if stringok == True:
		return True
	else:
		return False

def do_work( forever = True):
  global newtrack
  global RETRY_ATTEMPTS
  global retrycount
  global retryloop
  
  while inputs:
      time.sleep(0.01)

      # Wait for at least one of the sockets to be ready for processing
      #print >>sys.stderr, '\nwaiting for the next event'
      readable, writeable, exceptional = select.select(inputs, outputs, inputs)
      
      # Handle inputs
      for s in readable:
          if s is server:
              # A "readable" server socket is ready to accept a connection
              connection, client_address = s.accept()
              print >>sys.stderr, 'new connection from', client_address
              connection.setblocking(0)
              inputs.append(connection)
              connected_clients.append(connection)

              # Give the connection a queue for data we want to send
              message_queues[connection] = Queue.Queue()
              
          else:
              try:
                data = s.recv(64)
              except socket.error, ex:
                print ex


              if data:
                  # A readable client socket has data
				if(isprintable(data)):
					print '> ', data
					message_queues[s].put(data)
					if (re.search("Stopped Receiving",data)):
						retryloop = True
					if ((re.search("Sleeping",data)) and (retryloop == True)):
						retrycount +=1
					# Add output channel for response
					if s not in outputs:
						outputs.append(s)

					if (re.search("lat:", data)) and (newtrack==False):
						newtrack=True

					if (retrycount > RETRY_ATTEMPTS):
						newtrack=False
						retryloop=False
						retrycount = 0
					update_html(data)
				else:
					print("X ", data )
                      
              else:
                  # Interpret empty result as closed connection
                  #print >>sys.stderr, 'closing', client_address, 'after reading no data'
                  # Stop listening for input on the connection
                  if s in outputs:
                      outputs.remove(s)
                  inputs.remove(s)
                  connected_clients.remove(s)
                  s.close()

                  # Remove message queue
                  del message_queues[s]

      for s in writeable:
        try:
          next_msg = message_queues[s].get_nowait()
        except KeyError:
          print("KeyError")
        except Queue.Empty:
          outputs.remove(s)
        else:
          for t in connected_clients:
            t.send(next_msg)



              
      # Handle "exceptional conditions"
      for s in exceptional:
          print >>sys.stderr, 'handling exceptional condition for', s.getpeername()
          # Stop listening for input on the connection
          inputs.remove(s)
          if s in outputs:
              outputs.remove(s)
          s.close()
          # Remove message queue
          del message_queues[s]


def update_html(rawcontent):
  content = rawcontent.lstrip("*")
  global lastgps
  if re.match("lat: \d\d\\.\d\d\d\d\d\d,lng: -\d\d\\.\d\d\d\d\d\d\\r\\n",content):
    if content <> "lat: 0.00000,lng: 0.000000":
      if (content <> lastgps):
		  
		# Change these linesto match where you put 
		# your index.php file on you web server  
        with open("/var/www/html/lora/index.php", "r") as f:
          lineList = f.readlines()
        with open("/var/www/html/lora/index.php","w") as f:
			
			
          for line in lineList:
            plainwrite = True
            if '//NEXT COORD' in line:
              f.write("          {")
              latlng = content.replace('\n','').strip()
              f.write(latlng)
              f.write("},\n")
              f.write("          //NEXT COORD\n")
              plainwrite = False
            if 'var currentPos =' in line:
              f.write('      var currentPos = {')
              latlng = content.replace('\n','').strip()
              f.write(latlng)
              f.write('}\n')
              plainwrite = False
            if 'center: {' in line:
              f.write('          center: {');
              latlng = content.replace('\n','').strip()
              f.write(latlng)
              f.write('},\n')
              plainwrite = False
            if plainwrite == True:
              f.write(line)
            lastgps = content
        f.close

if __name__ == '__main__':

  do_work( True)

  
