CXX=g++
CXXFLAGS=-I. -std=c++11 -pthread

DEPS=pi-lamp.h status.h clickButton/clickButton.h RCSwitch/RCSwitch.h
OBJS=pi-lamp.o status.o clickButton/clickButton.o RCSwitch/RCSwitch.o

LIBS=-lwiringPi -lbluetooth

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: pi-lamp

pi-lamp: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(LIBS)

clean:
	$(RM) *.o pi-lamp
