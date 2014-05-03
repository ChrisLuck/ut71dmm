###########################################
# Uni-T UT71C DMM-Software
#
# Christian Lück
# April 2014
###########################################

all: ut71dmm

CC=gcc
CXX=g++
COBJS=./hidapi/libusb/hid.o
CPPOBJS=./main.o ./ut71.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=-I./hidapi/hidapi -Wall -g -c `pkg-config libusb-1.0 --cflags`
LIBS=`pkg-config libusb-1.0 libudev --libs`


ut71dmm: $(OBJS)
	g++ -Wall -g $^ $(LIBS) -o $@ -l pthread

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) ut71dmm

.PHONY: clean
