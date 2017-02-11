#! /usr/bin/env python

"""switchmate.py

A python-based command line utility for controlling Switchmate switches

Usage:
	./switch.py <mac_address> <auth_key> [on | off]
"""

import struct
import sys
import ctypes

from docopt import docopt
from bluepy.btle import DefaultDelegate, Peripheral, ADDR_TYPE_RANDOM
from binascii import unhexlify

STATE_NOTIFY_HANDLE = 0x000F
NOTIFY_VALUE = struct.pack('<BB', 0x01, 0x00)

STATE_HANDLE = 0x000E

def c_mul(a, b):
	'''
	Multiplication function with overflow
	'''
	return ctypes.c_int64((long(a) * b) &0xffffffffffffffff).value

def sign(data, key):
	'''
	Variant of the Fowler-Noll-Vo (FNV) hash function
	'''
	blob = data + key
	x = ord(blob[0]) << 7
	for c in blob:
		x1 = c_mul(1000003, x)
		x = x1 ^ ord(c) ^ len(blob)

	# once we have the hash, we append the data
	shifted_hash = (x & 0xffffffff) << 16
	shifted_data_0 = ord(data[0]) << 48
	shifted_data_1 = ord(data[1]) << 56
	packed = struct.pack('<Q', shifted_hash | shifted_data_0 | shifted_data_1)[2:]
	return packed

class NotificationDelegate(DefaultDelegate):
	def __init__(self):
		DefaultDelegate.__init__(self)

	def handleNotification(self, handle, data):
		print('Switched!')
		device.disconnect()
		sys.exit()

if __name__ == '__main__':
	arguments = docopt(__doc__)

	# try to connect
	try:
		device = Peripheral(arguments['<mac_address>'], ADDR_TYPE_RANDOM)
	except:
		print "Unable to find device"
		sys.exit()

	notifications = NotificationDelegate()
	device.setDelegate(notifications)

	auth_key = unhexlify(arguments['<auth_key>'])
	device.writeCharacteristic(STATE_NOTIFY_HANDLE, NOTIFY_VALUE, True)
	if arguments['on']:
		val = '\x01'
	else:
		val = '\x00'
	device.writeCharacteristic(STATE_HANDLE, sign('\x01' + val, auth_key))

	print('Waiting for response...')
	while True:
		device.waitForNotifications(1.0)
		print('.')
		sys.stdout.flush()
