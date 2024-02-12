#!/usr/bin/env python

import socket
import sys
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

if len(sys.argv) != 3:
    print >>sys.stderr, "ERROR: expected exactly 2 parameters:"
    print >>sys.stderr, "    %s <HOST> <PORT>" % sys.argv[0]
    exit(1)

# print >>sys.stderr, 'connecting to %s port %s' % server_address
x = (sys.argv[1], int(sys.argv[2]))
sock.connect(x)

sock.sendall("Start sending...")
# but do not continue

time.sleep(20) # will wait to be killed
