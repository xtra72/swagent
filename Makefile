###########################################
# Simple Makefile for SLABHIDtoUART HidUart
# example application
#
# Christopher Menke
# Silicon Labs
# 09-17-2012
###########################################

all: swagent

OUT      ?= build/

CC       ?= gcc
CFLAGS   ?= -Wall -g  -DNDEBUG

CXX      ?= g++
CXXFLAGS ?= -std=gnu++0x -Wall -g -DNDEBUG

COBJS     = 
CPPOBJS   = main.o config.o hiduart.o cp211x.o agent.o node.o thread.o trace.o trace_master.o exception.o locker.o object.o time.o timer.o utils.o mqttclient.o message.o messagequeue.o activeobject.o mutex.o gpio.o
OBJS      = $(COBJS) $(CPPOBJS)
LIBS      = -L/usr/local/lib -L/usr/lib/arm-linux-gnueabihf/ -L../libjson -lslabhidtouart -lslabhiddevice -lusb-1.0 -lpthread -ljson -lmosquitto
INCLUDES ?= 


swagent: $(OBJS) 
	mkdir -p $(OUT)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LIBS) -o $(OUT)swagent
	
$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -f $(OBJS) $(OUT)swagent
	rm -rf $(OUT)

.PHONY: clean

