FROM resin/rpi-raspbian

#switch on systemd init system in container
ENV INITSYSTEM on

# install tools for C
RUN apt-get -q update && apt-get install -yq --no-install-recommends \
	build-essential \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

# install WiringPi
RUN git clone git://git.drogon.net/wiringPi && \
	cd wiringPi && \
	./build

COPY . /usr/src/app
WORKDIR /usr/src/app

RUN make all

CMD ./pi-lamp
