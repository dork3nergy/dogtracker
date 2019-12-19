# dogtracker
LoRa Dog Tracker Project

Thanks for your interest in this project.

The code in this project is a mish-mash of code others have written
plus some of my own special modifictaions.  It should do .5 past
light speed. I'm sorry it's not commented better. You'll need to install
the various libraries that are in the #INCLUDEs.

lora-recv.ino :
	This is uploaded to the esp32.  It listens for lora packets and
	when it finds them, sends the data to whatever port you have
	specified.
	
lora-tx.ino :
	This file simply sends out a LoRa packet containing a GPS location.
	
lora-relay.ino :
	This is a simple relay.  Listens for LoRa packets and then
	re-transmits them.
	
/webserver/loramon.py :
	This is a python script that deals with the data lora.recv.ino 
	sends to it via the port specified.  It will simply display 
	non-gps info and plots a point on a google map when good GPS data
	is received.

/webserver/index.php :
	This is the php file that is updated (rewritten) by loramon.ph
	when new gps data is received.

