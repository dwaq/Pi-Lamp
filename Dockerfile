FROM resin/rpi-raspbian

#switch on systemd init system in container
ENV INITSYSTEM on

# install tools for C
RUN apt-get -q update && apt-get install -yq --no-install-recommends \
	build-essential \
	git-core \
	libcurl4-gnutls-dev \
	bluez bluez-firmware \
	libbluetooth3 libbluetooth3-dbg libbluetooth-dev libglib2.0-dev \
	python python-pip \
	avahi-utils libnss-mdns \
	iputils-ping \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

# install WiringPi
RUN git clone git://git.drogon.net/wiringPi && \
	cd wiringPi && \
	./build

# install tools for Python
RUN pip install docopt bluepy

# copy all files in our root to the working directory
COPY . /usr/src/app
WORKDIR /usr/src/app

# use host dbus
ENV DBUS_SYSTEM_BUS_ADDRESS=unix:path=/host/run/dbus/system_bus_socket

# resolve hosts from the .home domain
#COPY resolv.conf /etc/resolv.conf
RUN echo "nameserver 192.168.1.1" >> /etc/resolv.conf
RUN echo "domain home" >> /etc/resolv.conf

# build application
RUN make all

# make hciconfig executable
RUN ["chmod", "+x", "hciconfig.sh"]

# connect to bluetooth, then start application if successful
CMD ./hciconfig.sh && ./pi-lamp
