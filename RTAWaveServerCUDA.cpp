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

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>

#include <limits>
#include <RTAWave.h>
#include "packet/PacketLibDefinition.h"

#include <memory>
#include "CtaCuda.h"


//#define USE_ICESTORM 1


using namespace std;
using namespace CTA;
using namespace PacketLib;



class RTAWaveI : public RTAWave
{
public:
	RTAWaveI(int serverNum) : _serverNum(serverNum)
	{

	}

    virtual void send(Ice::Int nPixels, Ice::Int nSamples, const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
		
    }
	virtual void sendGPU(Ice::Int nPixels, Ice::Int nSamples, const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
        //cout << pixelNum << endl;
		//if nSamples = 30, use 32 words = 64 bytes for each pixel
		//if nSamples = 40, ise 64 words = 128 byte for each pixel
		if(nSamples == 40) //only 30 samples are allowed in this version, see initCUDA()
			exit(0);
		//convert the data
		unsigned long long maxWaveforms = nPixels;
		
		cta->ProcessData( maxWaveforms );
    }
	
	void initCUDA() {
		std::vector< std::shared_ptr< unsigned short > > myVector( 3 );
		unsigned long long maxBufferLength = 32 * 2000 * sizeof(unsigned short);
		cta = new ctaCuda( maxBufferLength, 30 );
		myVector = cta->getHostPointers();
		
		hostWaveformBuffer = myVector[ 0 ].get();
		
		hostOutputData_maximum = *myVector[ 1 ].get();
		hostOutputData_time = *myVector[ 2 ].get();
		
		cout << " Max: " << hostOutputData_maximum << "\t Time: " << hostOutputData_time << endl;
	}

private:
	int _serverNum;
	long nevents;
		
	//CUDA
	unsigned short* hostWaveformBuffer;
	unsigned short hostOutputData_maximum;
	unsigned short hostOutputData_time;
	ctaCuda* cta;
};

class RTAWaveServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    RTAWaveServer app;
	
#ifdef USE_ICESTORM
    return app.main(argc, argv, "config.sub");
#else
    return app.main(argc, argv, "config.server1");
#endif

}

void
usage(const string& n)
{
     cerr << "Usage: " << n
          << " [--batch] [--datagram|--twoway|--ordered|--oneway] [--retryCount count] [--id id] [topic]" << endl;
}

int
RTAWaveServer::run(int argc, char* argv[])
{
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = communicator()->getProperties()->parseCommandLineOptions("RTAWaveServer", args);
    Ice::stringSeqToArgs(args, argc, argv);

    bool batch = false;
    enum Option { None, Datagram, Twoway, Oneway, Ordered};
    Option option = None;
    string topicName = "time";
    string id;
    string retryCount;
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
        else if(optionString == "--ordered")
        {
            option = Ordered;
        }
        else if(optionString == "--batch")
        {
            batch = true;
        }
        else if(optionString == "--id")
        {
            ++i;
            if(i >= argc)
            {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            id = argv[i];
        }
        else if(optionString == "--retryCount")
        {
            ++i;
            if(i >= argc)
            {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            retryCount = argv[i];
        }
        else if(optionString.substr(0, 2) == "--")
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            topicName = argv[i++];
            break;
        }

        if(oldoption != option && oldoption != None)
        {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if(i != argc)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(!retryCount.empty())
    {
        if(option == None)
        {
            option = Twoway;
        }
        else if(option != Twoway && option != Ordered)
        {
            cerr << argv[0] << ": retryCount requires a twoway proxy" << endl;
            return EXIT_FAILURE;
        }
    }

    if(batch && (option == Twoway || option == Ordered))
    {
        cerr << argv[0] << ": batch can only be set with oneway or datagram" << endl;
        return EXIT_FAILURE;
    }

#ifdef USE_ICESTORM
    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator()->propertyToProxy("TopicManager.Proxy"));
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    IceStorm::TopicPrx topic;
    try
    {  
        topic = manager->retrieve(topicName);
    }
    catch(const IceStorm::NoSuchTopic&)
    {
        try
        {
            topic = manager->create(topicName);
        }
        catch(const IceStorm::TopicExists&)
        {
            cerr << appName() << ": temporary failure. try again." << endl;
            return EXIT_FAILURE;
        }
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTAWave.Subscriber");

    //
    // Add a servant for the Ice object. If --id is used the identity
    // comes from the command line, otherwise a UUID is used.
    //
    // id is not directly altered since it is used below to detect
    // whether subscribeAndGetPublisher can raise AlreadySubscribed.
    //
    Ice::Identity subId;
    subId.name = id;
    if(subId.name.empty())
    {
        subId.name = IceUtil::generateUUID();
    }
    Ice::ObjectPrx wave = adapter->add(new RTAWaveI(0), subId);
    adapter->activate();

    IceStorm::QoS qos;
    if(!retryCount.empty())
    {
        qos["retryCount"] = retryCount;
    }

    //
    // Set up the proxy.
    //
    if(option == Datagram)
    {
        if(batch)
        {
            wave = wave->ice_batchDatagram();
        }
        else
        {
            wave = wave->ice_datagram();
        }
    }
    else if(option == Twoway)
    {
        // Do nothing to the wave proxy. Its already twoway.
    }
    else if(option == Ordered)
    {
        // Do nothing to the wave proxy. Its already twoway.
        qos["reliability"] = "ordered";
    }
    else if(option == Oneway || option == None)
    {
        if(batch)
        {
            wave = wave->ice_batchOneway();
        }
        else
        {
            wave = wave->ice_oneway();
        }
    }

    try
    {
        topic->subscribeAndGetPublisher(qos, wave);
    }
    catch(const IceStorm::AlreadySubscribed&)
    {
        // If we're manually setting the wave id ignore.
        if(id.empty())
        {
            throw;
        }
        cout << "reactivating persistent wave" << endl;
    }

    topic->unsubscribe(wave);
#else

	Ice::PropertiesPtr props = communicator()->getProperties();
	int serverNum = props->getPropertyAsInt("RTAWave.ServerNumber");


	// Create an adapter for RTAWave
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTAWave");
    RTAWavePtr servant = new RTAWaveI(serverNum);
    adapter->add(servant, communicator()->stringToIdentity("wave"));
    adapter->activate();
#endif

    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    return EXIT_SUCCESS;
}
