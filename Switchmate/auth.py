#! /usr/bin/env python

"""switchmate.py

A python-based command line utility for controlling Switchmate switches

Usage:
	./switchmate.py <mac_address> auth
"""

import struct
import sys

from docopt import docopt
from bluepy.btle import DefaultDelegate, Peripheral, ADDR_TYPE_RANDOM
from binascii import hexlify

NOTIFY_VALUE = struct.pack('<BB', 0x01, 0x00)

AUTH_NOTIFY_HANDLE = 0x0017
AUTH_HANDLE = 0x0016
AUTH_INIT_VALUE = struct.pack('<BBBBBB', 0x00, 0x00, 0x00, 0x00, 0x01, 0x00)

class NotificationDelegate(DefaultDelegate):
	def __init__(self):
		DefaultDelegate.__init__(self)

	def handleNotification(self, handle, data):
		print('Auth key is {}'.format(hexlify(data[3:]).upper()))
		device.disconnect()
		sys.exit()

if __name__ == '__main__':
	arguments = docopt(__doc__)

	device = Peripheral(arguments['<mac_address>'], ADDR_TYPE_RANDOM)

	notifications = NotificationDelegate()
	device.setDelegate(notifications)

	device.writeCharacteristic(AUTH_NOTIFY_HANDLE, NOTIFY_VALUE, True)
	device.writeCharacteristic(AUTH_HANDLE, AUTH_INIT_VALUE, True)
	print('Press button on Switchmate to get auth key')

	print('Waiting for response')
	while True:
		device.waitForNotifications(1.0)
		print('.')
		sys.stdout.flush()
