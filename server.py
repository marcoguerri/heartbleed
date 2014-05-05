import socket
from time import sleep

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(("0.0.0.0", 5080))
s.listen(1)
conn, addr = s.accept()
srv_hello = "\x0d\x0e\x0a\x0d\x0b\x0e\x0e\x0f"

print "Started connection with " + str(addr)
data = conn.recv(320)
conn.send(srv_hello)

f = open("key_scrambled",  "rb")
key_buff = ''
temp = f.read(64)
key_buff = temp
while temp != '':
    temp = f.read(64)
    key_buff = key_buff + temp


conn.send(key_buff)
sleep(10)
f.close()
s.close()


