#!/usr/bin/env python

import socket
import sys
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

if len(sys.argv) != 2:
    print >>sys.stderr, "ERROR: expected exactly 1 parameter:"
    print >>sys.stderr, "    %s <PORT>" % sys.argv[0]
    exit(1)

sock.bind(("0.0.0.0", int(sys.argv[1])))
sock.listen(1)

conn, addr = sock.accept()
conn.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)

print 'Accepted connection, but not going to receive any data', addr

time.sleep(30) # will wait to be killed
