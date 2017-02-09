CXX=g++
CXXFLAGS=-I. -std=c++11 -pthread

DEPS=clickButton/clickButton.h RCSwitch/RCSwitch.h
OBJ=pi-lamp.o clickButton/clickButton.o RCSwitch/RCSwitch.o

LIBS=-lwiringPi

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: pi-lamp

pi-lamp: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(LIBS)

clean:
	$(RM) *.o pi-lamp
