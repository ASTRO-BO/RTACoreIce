// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module CTA
{

sequence<byte> ByteSeq;

interface RTAWave
{
    void send(int pixelNum, int pixelSize, ByteSeq seq);
};

};
