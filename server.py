import socket
import sys
import struct

from time import sleep

PORT = 5080
BUFF_SIZE = 4096
#
# Get a random integer from /dev/urandom
#
f = open("/dev/urandom", "rb")
random = struct.unpack("<B",f.read(1))[0]

print "Will send key at " + str(random)


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("0.0.0.0", PORT))
s.listen(1)
conn, addr = s.accept()
srv_hello = "\x0d\x0e\x0a\x0d\x0b\x0e\x0e\x0f"

print "Started connection with " + str(addr)
data = conn.recv(BUFF_SIZE)

conn.send(srv_hello)

print "Reading HB request from server"
buff = conn.recv(BUFF_SIZE)

print "Read " + str(len(buff))

#
# Reads a file which contains the key scrambled with some random
# garbage and sends everything off to the client.
#

buff = f.read(1024)
count = 0
while True:
    count = count + 1
    if count == random:
        key = open("key_scrambled", "rb")
        buff = key.read(1024)

    sys.stdout.write("Sending packet %s, len %s \r"  % (str(count),str(len(buff)) ))
    try:
        conn.send(buff)
    except Exception, e:
        print "\nConnection closed"
        sys.exit(1)
    #
    # Receiving another HB request
    #
    conn.recv(BUFF_SIZE)
    f.read(1024)
    

print "Sleeping..."
sleep(10)
f.close()
s.close()


