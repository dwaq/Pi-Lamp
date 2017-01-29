all: pi-lamp

pi-lamp: clickButton/clickButton.o RCSwitch/RCSwitch.o pi-lamp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi

clean:
	$(RM) *.o pi-lamp
