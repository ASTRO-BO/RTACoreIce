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
#include "RTAWave.h"
#include <CTACameraTriggerData0.h>
#include <vector>

class RTAReceiverI : public CTA::RTAReceiver
{
public:

	RTAReceiverI(RTATelem::CTACameraTriggerData0& trtel, std::vector<CTA::RTAWavePrx>& streams) : _trtel(trtel), _streams(streams)
	{
	}

	virtual void send(const CTA::ByteSeq& seq, const Ice::Current& curr);

private:

	RTATelem::CTACameraTriggerData0 _trtel;
	std::vector<CTA::RTAWavePrx> _streams;
};

#endif
