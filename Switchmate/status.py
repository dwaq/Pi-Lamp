#! /usr/bin/env python

"""switchmate.py

A python-based command line utility for reading the status of Switchmate switches

Usage:
    ./status.py
"""

import socket
import sys
from bluepy.btle import Scanner, DefaultDelegate

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        AD_TYPE_UUID = 0x07
        SWITCHMATE_UUID = '23d1bcea5f782315deef121223150000'

        AD_TYPE_SERVICE_DATA = 0x16

        if (dev.getValueText(AD_TYPE_UUID) == SWITCHMATE_UUID):
            data = dev.getValueText(AD_TYPE_SERVICE_DATA)
            # the bit at 0x0100 signifies if the switch is off or on
            print "Switchmate status:", str((int(data, 16) >> 8) & 1)


if __name__ == '__main__':
  #print('Looking for switchmate status...')

  scanner = Scanner().withDelegate(ScanDelegate())

  scanner.scan(0)
