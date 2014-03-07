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

using namespace CTA;

using PacketLib::ByteStream;
using PacketLib::ByteStreamPtr;
using PacketLib::byte;

void RTAReceiverI::send(const ByteSeq& seq, const Ice::Current& curr)
{
	const int LARGE = 0;
	const int MEDIUM = 1;
	const int SMALL = 2;

	ByteStreamPtr streamPtr = ByteStreamPtr(new ByteStream((byte*)&seq[0], seq.size(), false));
	_trtel.setStream(streamPtr);

	int type = LARGE; // TODO fix this using the right type decoded from trtel
	if(type == LARGE)
		std::cout << "% - A Large Telescope triggered - Process A activating" << std::endl;
	else if(type == MEDIUM)
		std::cout << "% - A Medium Telescope triggered - Process B activating" << std::endl;
	else if(type == SMALL)
		std::cout << "% - A Small Telescope triggered - Process C activating" << std::endl;

	_streams[type]->send(0, 0, seq);
}
