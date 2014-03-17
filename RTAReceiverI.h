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

#include "RTAReceiver.h"
#include "RTAMonitor.h"
#include "RTAWave.h"
#include <CTADecoder.h>
#include <vector>
#include "RTAMonitorThread.h"

class RTAReceiverI : public CTA::RTAReceiver
{
public:

	RTAReceiverI(RTATelem::CTADecoder& decoder, std::vector<CTA::RTAWavePrx>& streams, size_t& byteSent, IceUtil::Mutex& mutex)
		: _decoder(decoder), _streams(streams), _byteSent(byteSent), _mutex(mutex)
	{
	}

virtual void send(const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur);

private:

	RTATelem::CTADecoder& _decoder;
	std::vector<CTA::RTAWavePrx>& _streams;
	size_t& _byteSent;
	IceUtil::Mutex& _mutex;
};

#endif
