#!/usr/bin/env python

import socket
import sys
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

if len(sys.argv) < 4:
    print >>sys.stderr, "ERROR: expected exactly 3 parameters:"
    print >>sys.stderr, "    %s <HOST> <PORT> <FILE> [<DELAY>]" % sys.argv[0]
    exit(1)

if len(sys.argv) == 5:
    DELAY = float(sys.argv[4])
else:
    DELAY = 0.1

f = open(sys.argv[3], 'rb')

endpoint = (sys.argv[1], int(sys.argv[2]))
sock.connect(endpoint)

buf = f.read(499)
while buf:
    sock.send(buf)
    time.sleep(DELAY)
    buf = f.read(499)
