#! /usr/bin/env python

"""scan.py

A python-based command line utility for scanning for Switchmate switches

Usage:
	sudo ./scan.py
"""

import sys

from bluepy.btle import Scanner

if __name__ == '__main__':
	print('Scanning...')
	sys.stdout.flush()

	scanner = Scanner()
	devices = scanner.scan(10.0)

	SERVICES_AD_TYPE = 7
	SWITCHMATE_SERVICE = '23d1bcea5f782315deef121223150000'

	switchmates = []
	for dev in devices:
		for (adtype, desc, value) in dev.getScanData():
			is_switchmate = adtype == SERVICES_AD_TYPE and value == SWITCHMATE_SERVICE
			if is_switchmate and dev not in switchmates:
				switchmates.append(dev)

	if len(switchmates):
		print('Found Switchmates:')
		for switchmate in switchmates:
			print(switchmate.addr)
	else:
		print('No Switchmate devices found');

	sys.exit()
