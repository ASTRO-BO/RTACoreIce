    /***************************************************************************
        begin                : Mar 07 2014
        copyright            : (C) 2014 Andrea Zoli
        email                : zoli@iasfbo.inaf.it
    ***************************************************************************/

    /***************************************************************************
    *                                                                         *
    *   This program is free software for non commercial purpose              *
    *   and for public research institutes; you can redistribute it and/or    *
    *   modify it under the terms of the GNU General Public License.          *
    *   For commercial purpose see appropriate license terms                  *
    *                                                                         *
    ***************************************************************************/

#ifndef _RTARECEIVER_I_H
#define _RTARECEIVER_I_H

//#define USESHM 1

#include "RTAReceiver.h"
#include "RTAMonitor.h"
#include "RTAViewer.h"
#include "RTAWave.h"
#include <CTADecoder.h>
#include <vector>
#include "RTAMonitorThread.h"
#include "RTAConfigLoad.h"

#ifdef USESHM
// semaphore
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#endif

class RTAReceiverI : public CTA::RTAReceiver
{
public:

	RTAReceiverI(RTATelem::CTADecoder& decoder, std::vector<CTA::RTAWavePrx>& streams, size_t& byteSent, IceUtil::Mutex& mutex, CTA::RTAViewerPrx& viewer, std::vector<int>& triggeredEvent, int& nevent)
		: _decoder(decoder), _streams(streams), _byteSent(byteSent), _mutex(mutex), _viewer(viewer), _triggeredEvent(triggeredEvent), _nevent(nevent)
	{
		collectevt = false;
		lastEvtNum = 0;
		
		string ctarta;
        const char* home = getenv("CTARTA");
	
		if (!home)
		{
			std::cerr << "CTARTA environment variable is not defined." << std::endl;
		}
		
		ctarta = home;
        
		
		ctaconf = new RTAConfig::RTAConfigLoad( ctarta + "/share/rtatelem/PROD2_telconfig.fits.gz" );
		cout << "Loaded RTA config" << endl;
#ifdef USESHM
		cout << "start SHM initialization" << endl;
		if(initShm())
			sendShm();
#endif

	}
	
	~RTAReceiverI() {
		cout << "destroy" << endl;
	}

	virtual void send(const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur);

private:

	RTATelem::CTADecoder& _decoder;
	std::vector<CTA::RTAWavePrx>& _streams;
	size_t& _byteSent;
	IceUtil::Mutex& _mutex;
	CTA::RTAViewerPrx& _viewer;
	std::vector<int>& _triggeredEvent;
	int& _nevent;
	bool collectevt;
	dword lastEvtNum;
	RTAConfig::RTAConfigLoad* ctaconf;
	
#ifdef USESHM
protected:
	
	virtual int initShm();
	virtual void sendShm();
	
	//shm
	dword* sizeShmPtr;
	byte* bufferShmPtr;
	sem_t* full;
	sem_t* empty;
#endif
	
};

#endif
