#!/usr/bin/env python

import sys, time
from daemon import Daemon
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
            print ("off", "on")[(int(data, 16) >> 8) & 1]

class MyDaemon(Daemon):
    def run(self):
        scanner = Scanner().withDelegate(ScanDelegate())
        scanner.scan(0)

if __name__ == "__main__":
    daemon = MyDaemon('/tmp/pi-lamp-scan.pid')
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            daemon.start()
        elif 'stop' == sys.argv[1]:
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            daemon.restart()
        else:
            print "Unknown command"
            sys.exit(2)
        sys.exit(0)
    else:
        print "usage: %s start|stop|restart" % sys.argv[0]
        sys.exit(2)

