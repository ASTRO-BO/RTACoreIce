# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Prefix for each installed program. Only ABSOLUTE PATH
prefix=/usr/local
exec_prefix=$(prefix)
# The directory to install the binary files in.
bindir=$(exec_prefix)/bin
# The directory to install the libraries in.
libdir=$(exec_prefix)/lib
# The directory to install the include files in.
includedir=$(exec_prefix)/include
# The directory to install the local configuration file.
datadir=$(exec_prefix)/share

top_srcdir	= .

#ENV= root ctatools
ENV ?= root 

PUBLISHER	= RTAReceiver_Ice
PUBLISHER2  = RTAReceiverZMQ
SUBSCRIBER	= RTAWaveServer

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

ifneq (, $(findstring ctatools, $(ENV)))
TARGETS += $(PUBLISHER2)
endif

OBJS		= RTAWave.o RTAReceiver.o RTAMonitor.o RTAViewer.o RTAViewCamera.o

POBJS		= RTAReceiverI.o RTAReceiver_Ice.o

P2OBJS		= RTAReceiverZMQ.o

SOBJS		= RTAWaveServer.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(P2OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= RTAWave.ice RTAReceiver.ice RTAMonitor.ice RTAViewer.ice RTAViewCamera.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS += -I.
LIBS := $(LIBS) -lRTAtelem -lpacket -lRTAconfig -lQLBase -lcfitsio
ifneq (, $(findstring root, $(ENV)))
        ROOTCFLAGS   := `root-config --cflags`
        ROOTLIBS     := `root-config --libs`
        ROOTGLIBS    := `root-config --glibs`
        ROOTCONF= -pipe -Wall -W -fPIC -D_REENTRANT
        LIBS += $(ROOTGLIBS) -lMinuit
        CPPFLAGS += $(ROOTCONF) $(ROOTCFLAGS)
endif
CPPFLAGS += -std=c++11

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(PUBLISHER2): $(OBJS) $(P2OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(P2OBJS) $(LIBS) $(P2LIBS) -lCTAToolsCore -lprotobuf -lzmq

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

clean::
	-rm -rf db/*

install:
	test -d $(bindir) || mkdir -p $(bindir)
	cp -pf RTAReceiver_Ice $(bindir)
	cp -pf RTAWaveServer $(bindir)
	test -d $(datadir)/core/waveserver || mkdir -p $(datadir)/core/waveserver
	cp -pf config.server* $(datadir)/core/waveserver
	test -d $(datadir)/core/storm || mkdir -p $(datadir)/core/storm
	cp -pf config.icebox config.receiver config.sub config.service $(datadir)/core/storm
	test -d $(datadir)/core/receiver || mkdir -p $(datadir)/core/receiver
	cp -pf config.receiverNoStorm $(datadir)/core/receiver
	if [ -z RTAReceiverZMQ ] ; then cp -pf RTAReceiverZMQ $(bindir); fi

include .depend
