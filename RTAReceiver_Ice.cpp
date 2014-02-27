/***************************************************************************
 * begin                : Nov 27 2013
 * copyright            : (C) 2013 Andrea Zoli
 * email                : zoli@iasfbo.inaf.it
 * *************************************************************************
 *
 * Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
 *
 * This copy of Ice is licensed to you under the terms described in the
 * ICE_LICENSE file included in this distribution.
 *
 ***************************************************************************/

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <cstdlib>
#include <map>

#include <CTACameraTriggerData.h>

#include <RTAWave.h>

using namespace std;
using namespace CTA;

class RTAReceiver_Ice : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    RTAReceiver_Ice app;
    return app.main(argc, argv, "config.pub");
}

void
usage(const string& n)
{
    cerr << "Usage: " << n << " [--datagram|--twoway|--oneway]" << endl;
}

int
RTAReceiver_Ice::run(int argc, char* argv[])
{
    enum Option { None, Datagram, Twoway, Oneway };
    Option option = None;
    string topicName = "time";
    int i;

    for(i = 1; i < argc; ++i)
    {
        string optionString = argv[i];
        Option oldoption = option;
        if(optionString == "--datagram")
        {
            option = Datagram;
        }
        else if(optionString == "--twoway")
        {
            option = Twoway;
        }
        else if(optionString == "--oneway")
        {
            option = Oneway;
        }
        else if(optionString.substr(0, 2) == "--")
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        if(oldoption != option && oldoption != None)
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator()->propertyToProxy("TopicManager.Proxy"));
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    //
    // Retrieve the streams for the topics.
    //
	const int TOPICNUM = 3;
	const std::string topicNames[TOPICNUM] = { "LargeTelescope", "MediumTelescope", "SmallTelescope" };
	std::vector<RTAWavePrx> streams;

	for(int i=0; i<TOPICNUM; i++)
	{
	    IceStorm::TopicPrx topic;
		try
		{
			topic = manager->retrieve(topicNames[i]);
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			try
			{
				topic = manager->create(topicNames[i]);
			}
			catch(const IceStorm::TopicExists&)
			{
				cerr << appName() << ": temporary failure. try again." << endl;
				return EXIT_FAILURE;
			}
		}

	    //
	    // Get the topic's receiver object, and create a RTAWave proxy with
	    // the mode specified as an argument of this application.
	    //
	    Ice::ObjectPrx receiver = topic->getPublisher();
	    if(option == Datagram)
	    {
	        receiver = receiver->ice_datagram();
	    }
		else if(option == Twoway)
		{
			// Do nothing.
		}
		else if(option == Oneway || option == None)
		{
			receiver = receiver->ice_oneway();
		}
    
	    RTAWavePrx prx = RTAWavePrx::uncheckedCast(receiver);
		streams.push_back(prx);
	}

	std::string ctarta = getenv("CTARTA");
	RTATelem::CTACameraTriggerData *trtel;
	trtel = new RTATelem::CTACameraTriggerData(ctarta + "/share/rtatelem/rta_fadc.stream", ctarta + "/data/out_fadc.raw", "");

	const int LARGE = 0;
	const int MEDIUM = 1;
	const int SMALL = 2;
	std::map<int, int> mapTelescopes; // map telescope id to types (hardcoded)
	mapTelescopes[1]=LARGE;
	mapTelescopes[2]=LARGE;
	mapTelescopes[4]=LARGE;
	mapTelescopes[5]=LARGE;
	mapTelescopes[6]=MEDIUM;
	mapTelescopes[8]=MEDIUM;
	mapTelescopes[10]=MEDIUM;
	mapTelescopes[12]=MEDIUM;
	mapTelescopes[14]=MEDIUM;
	mapTelescopes[15]=MEDIUM;
	mapTelescopes[16]=MEDIUM;
	mapTelescopes[17]=MEDIUM;
	mapTelescopes[18]=MEDIUM;
	mapTelescopes[19]=MEDIUM;
	mapTelescopes[20]=MEDIUM;
	mapTelescopes[22]=MEDIUM;
	mapTelescopes[29]=MEDIUM;
	mapTelescopes[30]=MEDIUM;
	mapTelescopes[31]=MEDIUM;
	mapTelescopes[32]=MEDIUM;
	mapTelescopes[40]=MEDIUM;
	mapTelescopes[41]=MEDIUM;
	mapTelescopes[42]=MEDIUM;
	mapTelescopes[49]=MEDIUM;
	mapTelescopes[50]=MEDIUM;
	mapTelescopes[51]=MEDIUM;
	mapTelescopes[52]=MEDIUM;
	mapTelescopes[60]=SMALL;
	mapTelescopes[62]=SMALL;
	mapTelescopes[63]=SMALL;
	mapTelescopes[64]=SMALL;
	mapTelescopes[66]=SMALL;
	mapTelescopes[67]=SMALL;
	mapTelescopes[68]=SMALL;
	mapTelescopes[69]=SMALL;
	mapTelescopes[70]=SMALL;
	mapTelescopes[71]=SMALL;
	mapTelescopes[72]=SMALL;
	mapTelescopes[73]=SMALL;
	mapTelescopes[74]=SMALL;
	mapTelescopes[75]=SMALL;
	mapTelescopes[76]=SMALL;
	mapTelescopes[77]=SMALL;
	mapTelescopes[78]=SMALL;
	mapTelescopes[79]=SMALL;
	mapTelescopes[80]=SMALL;
	mapTelescopes[81]=SMALL;
	mapTelescopes[82]=SMALL;
	mapTelescopes[83]=SMALL;
	mapTelescopes[84]=SMALL;
	mapTelescopes[85]=SMALL;
	mapTelescopes[86]=SMALL;
	mapTelescopes[87]=SMALL;
	mapTelescopes[88]=SMALL;
	mapTelescopes[89]=SMALL;
	mapTelescopes[90]=SMALL;
	mapTelescopes[91]=SMALL;
	mapTelescopes[92]=SMALL;
	mapTelescopes[93]=SMALL;
	mapTelescopes[94]=SMALL;
	mapTelescopes[95]=SMALL;
	mapTelescopes[96]=SMALL;

	byte* rawStream = trtel->readPacket()->getStream();
	while(rawStream)
	{

		int id = trtel->getTelescopeId();
		std::cout << "% - Triggered Telescope ID = " << id << std::endl;
		int type = mapTelescopes[id];
		if(type == LARGE)
			std::cout << "% - A Large Telescope triggered - Process A activating" << std::endl;
		else if(type == MEDIUM)
			std::cout << "% - A Medium Telescope triggered - Process B activating" << std::endl;
		else if(type == SMALL)
			std::cout << "% - A Small Telescope triggered - Process C activating" << std::endl;

		// Get current number of pixels
		word npixels = trtel->getNumberOfPixels();

		// Get FIXED per-telescope number of samples.
		// This could be extended to be dynamic sending an additional vector of sample sizes.
		word nsamples = trtel->getNumberOfSamples(1);

		std::cout << "% - Sending data..." << std::endl;
		std::cout << "Pixel Num: " << npixels << std::endl;
		std::cout << "Sample Num: " << nsamples << std::endl;
//		std::cout << "Raw Stream Size: " << rawStream << std::endl; FIXME

		try
		{
			ByteSeq seq;
			// TODO copy seq = rawStream;
			streams[type]->send(npixels, nsamples, seq);
		}
		catch(const Ice::CommunicatorDestroyedException&)
		{
			// Ignore
		}

		rawStream = trtel->readPacket()->getStream();
	}

    return EXIT_SUCCESS;
}
