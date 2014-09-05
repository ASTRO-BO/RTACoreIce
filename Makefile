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

PUBLISHER	= RTAReceiver_Ice
SUBSCRIBER	= RTAWaveServer

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= RTAWave.o RTAReceiver.o RTAMonitor.o RTAViewer.o RTAViewCamera.o

POBJS		= RTAReceiverI.o RTAReceiver_Ice.o

SOBJS		= RTAWaveServer.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= RTAWave.ice RTAReceiver.ice RTAMonitor.ice RTAViewer.ice RTAViewCamera.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS ?= -O2
LIBS := -lIceStorm $(LIBS) -lRTAtelem -lpacket -lRTAconfig -lQLBase -lcfitsio

LINKERENV= root
ifneq (, $(findstring root, $(LINKERENV)))
        ROOTCFLAGS   := `root-config --cflags`
        ROOTLIBS     := `root-config --libs`
        ROOTGLIBS    := `root-config --glibs`
        ROOTCONF=-O2 -pipe -Wall -W -fPIC -D_REENTRANT
        LIBS += $(ROOTGLIBS) -lMinuit
        CPPFLAGS += $(ROOTCONF) $(ROOTCFLAGS)
endif

CPPFLAGS += -I.

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

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

include .depend
