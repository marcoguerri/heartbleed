import socket
import sys
import struct
import errno
from time import sleep

PORT = 5080
BUFF_SIZE = 4096


def run():
    
    # Get a random integer from /dev/urandom, based on which the key will be sent
    try:
        f = open("/dev/urandom", "rb")
        random = struct.unpack("<H",f.read(2))[0]
    except IOError:
        sys.stderr.write("Error while reading from /dev/urandom\n");
        sys.exit(1);

    sys.stderr.write("Will send key at %s\n" % str(random));

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(("0.0.0.0", PORT))
    except Exception, e:
        sys.stderr.write("Error while binding socket: %s\n" % str(e));
        sys.exit(1)

    s.listen(1)
    conn, addr = s.accept()
    srv_hello = "\x0d\x0e\x0a\x0d\x0b\x0e\x0e\x0f"
    
    sys.stderr.write("Started connection with %s\n" % str(addr));

    try:
        data = conn.recv(BUFF_SIZE)
        conn.send(srv_hello)
    except Exception, e:
        sys.stderr.write("Error while performing SSL handshake: %s\n" % str(e));
        f.close()
        conn.close()
        sys.exit(1)
    
    try:
        buff = conn.recv(BUFF_SIZE)
    except Exception, e:
        sys.stderr.write("Error while reading from socket: %s\n" % str(e));
        f.close()
        conn.close()
        sys.exit(1)

    # Send to the client random data coming from /dev/urandom and eventually
    # the private key read from key_scrambled (which contains the key with
    # some further random data. When hexdumping key_scrambled looking for the
    # key, keep in mind that it was written as a buffer of little endian uint16).

    try:
        buff = f.read(1024)
    except IOError, e:
       sys.stderr.write("Error while reading from /dev/urandom")
       f.close()
       sys.exit(1)
        
    count = 0
    while True:
        count = count + 1
        if count == random:
            try:
                key = open("key_scrambled", "rb")
                buff = key.read(1024)
            except IOError:
                sys.stderr.write("Error while reading private key from file\n")
                f.close()
                conn.close()
                exit(1)
                     
        sys.stdout.write("Sending packet %s, len %5s \r"  % (str(count),str(len(buff))))
        try:
            conn.send(buff)
        except KeyboardInterrupt, e:
            sys.stderr.write("send: interrupted\n")
            exit(1)
        except Exception, e:
            if(e.errno == errno.EPIPE):
                sys.stderr.write("\nRemote end has closed the connection\n")
            else:
                sys.stderr.write("\nsend error: %s\n" % str(errno.errorcode[e.errno]))
            sys.exit(1)

        # Receiving next HB request
        try:
            conn.recv(BUFF_SIZE)
            f.read(1024)
        except KeyboardInterrupt, e:
            sys.stderr.write("\nrecv: interrupted\n")
            sys.exit(1)
        except Exception, e:
            sys.stderr.write("\nError while reading client request\n")
            sys.exit(1)
    
    try:
        f.close()
        conn.close()
        key.close()
    except Exception, e:
        pass

if __name__ == '__main__':
    run()

