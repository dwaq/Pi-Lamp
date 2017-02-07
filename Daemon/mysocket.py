#!/usr/bin/env python

# https://pymotw.com/2/socket/uds.html

import socket
import sys
import os

server_address = '/tmp/pi-lamp-status'

# Make sure the socket does not already exist
try:
    os.unlink(server_address)
except OSError:
    if os.path.exists(server_address):
        raise
