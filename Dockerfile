FROM resin/rpi-raspbian

#switch on systemd init system in container
ENV INITSYSTEM on

# install tools for C
RUN apt-get -q update && apt-get install -yq --no-install-recommends \
	build-essential \
	git-core \
	avahi-daemon avahi-utils libnss-mdns \
	libcurl4-gnutls-dev \
	bluez bluez-firmware \
	libbluetooth3 libbluetooth3-dbg libbluetooth-dev libglib2.0-dev \
	python python-pip \
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

# build application
RUN make all

# resolve hosts from the .local domain
RUN systemctl enable avahi-daemon
COPY nsswitch.conf /etc/nsswitch.conf

# make hciconfig executable
RUN ["chmod", "+x", "hciconfig.sh"]

# connect to bluetooth, then start application if successful
CMD ./hciconfig.sh && ./pi-lamp
