#!/bin/bash

echo "Attaching hci0..."
until /usr/bin/hciattach /dev/ttyAMA0 bcm43xx 921600 noflow -
do
    echo "Bluetooth initialization failed, retrying..."
done

echo "Bringing hci0 up..."
hciconfig hci0 up

echo "Bluetooth initialized successfully!"