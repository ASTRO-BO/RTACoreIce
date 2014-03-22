/***************************************************************************
    begin                : Mar 05 2014
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

#pragma once

module CTA
{

sequence<byte> ByteSeq;

interface RTAWave
{
    void send(int pixelNum, int pixelSize, ["cpp:array"] ByteSeq seq);
	void send2(["cpp:array"] ByteSeq seq);
};

};
