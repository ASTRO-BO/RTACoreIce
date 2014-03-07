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

using namespace CTA;

void RTAReceiverI::send(const ByteSeq& seq, const Ice::Current& curr)
{
	std::cout << "Routing a new sequence" << std::endl;
	std::cout << "Size: " << seq.size() << std::endl;
}
