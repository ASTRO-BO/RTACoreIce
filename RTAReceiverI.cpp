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

#include "RTAReceiverI.h"
#include <iostream>
#include <packet/ByteStream.h>
#include <CTACameraTriggerData1.h>

using namespace CTA;
using namespace PacketLib;

void RTAReceiverI::send(const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
{
	ByteStreamPtr streamPtr = ByteStreamPtr(new ByteStream((byte*)seqPtr.first, seqPtr.second-seqPtr.first, false));

	RTATelem::CTAPacket& packet = _decoder.getPacket(streamPtr);
	enum RTATelem::CTAPacketType type = packet.getPacketType();

	// skipping packets of camera type different from triggerdata1
	if(type != RTATelem::CTA_CAMERA_TRIGGERDATA_1)
	{
		std::cout << "Warning: Skipping packet from a camera of type " << type << std::endl;
		return;
	}

	RTATelem::CTACameraTriggerData1& trtel = (RTATelem::CTACameraTriggerData1&) packet;

	if(_viewer)
	{
		word tt = trtel.getIndexOfCurrentTriggeredTelescope();
		word ntt = trtel.getNumberOfTriggeredTelescopes();

		if(tt+1 < ntt) {
			if(_nevent % 100 == 0) {
				word telid = 50;
			    _triggeredEvent.push_back(telid);
			}
		}
		else {
			if(_nevent % 100 == 0) {
				word evtnum = trtel.getEventNumber();
		        _viewer->update(_triggeredEvent, evtnum);

/*				std::cout << "[";
				for(unsigned int i=0; i<_triggeredEvent.size(); i++)
					std::cout << _triggeredEvent[i] << ", ";
				std::cout << "] evtnum = " << evtnum << std::endl;*/

			    _triggeredEvent.resize(0);
			}
		    _nevent++;
		}
	}

/*	const int LARGE = 0;
	const int MEDIUM = 1;
	const int SMALL = 2;

	int npixels = trtel.getNumberOfPixels();
	int nsamples = trtel.getNumberOfSamples(0);

	int teltype = LARGE; // TODO settare il type in base al numero di pixel/samples

	if(teltype == LARGE)
		std::cout << "% - A Large Telescope triggered - Process A activating" << std::endl;
	else if(teltype == MEDIUM)
		std::cout << "% - A Medium Telescope triggered - Process B activating" << std::endl;
	else if(teltype == SMALL)
		std::cout << "% - A Small Telescope triggered - Process C activating" << std::endl;

	_streams[teltype]->send(0, 0, seqPtr);*/

	_mutex.lock();
	_byteSent += streamPtr->size();
	_mutex.unlock();
}
