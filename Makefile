CXX=g++
CXXFLAGS=-I. -std=c++11 -pthread -Werror -Wall -Wextra

DEPS=pi-lamp.h
OBJS=pi-lamp.o clickButton/clickButton.o switchmate/switchmate.o hue/hue.o

LIBS=-lwiringPi -lbluetooth -lcurl

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: pi-lamp

pi-lamp: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(LIBS)

clean:
	$(RM) $(OBJS) pi-lamp
