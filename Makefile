# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


top_srcdir	= .

PUBLISHER	= RTAReceiver_Ice
SUBSCRIBER	= RTAWaveServer

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= RTAWave.o RTAReceiver.o RTAMonitor.o RTAViewer.o

POBJS		= RTAReceiverI.o RTAReceiver_Ice.o

SOBJS		= RTAWaveServer.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= RTAWave.ice RTAReceiver.ice RTAMonitor.ice RTAViewer.ice

include $(top_srcdir)/config/Make.rules

LINKERENV= root
ifneq (, $(findstring root, $(LINKERENV)))
        ROOTCFLAGS   := $(shell root-config --cflags)
        ROOTLIBS     := $(shell root-config --libs)
        ROOTGLIBS    := $(shell root-config --glibs)
        ROOTCONF=-O3 -pipe -Wall -W -fPIC -D_REENTRANT
        LIBS += $(ROOTGLIBS) -lMinuit
        CPPFLAGS += $(ROOTCONF)
endif

CPPFLAGS	:= -O3 -I. $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS) -lRTAtelem -lpacket -lRTAconfig -lQLBase -lcfitsio -lCTA_CUDA


$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

clean::
	-rm -rf db/*

include .depend
