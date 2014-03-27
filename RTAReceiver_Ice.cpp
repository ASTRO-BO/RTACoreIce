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
#include <CTAStream.h>
#include <CTACameraTriggerData0.h>
#include "RTAReceiverI.h"
#include "RTAMonitorThread.h"
#include "RTAViewer.h"

//#define USE_ICESTORM 1

#ifdef USE_ICESTORM
#include <IceStorm/IceStorm.h>
#endif

using namespace std;
using namespace CTA;

class RTAReceiver_Ice : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int main(int argc, char* argv[])
{
    RTAReceiver_Ice app;
#ifdef USE_ICESTORM
    return app.main(argc, argv, "config.receiver");
#else
	return app.main(argc, argv, "config.receiverNoStorm");
#endif
}

int RTAReceiver_Ice::run(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAReceiver_Ice file.raw" << std::endl;
		return EXIT_FAILURE;
	}

std::vector<RTAWavePrx> streams;
#ifdef USE_ICESTORM
	// Get a proxy for the 3 topics
	const int TOPICNUM = 3;
	const std::string topicNames[TOPICNUM] = { "LargeTelescope", "MediumTelescope", "SmallTelescope" };

    IceStorm::TopicManagerPrx manager = IceStorm::TopicManagerPrx::checkedCast(
        communicator()->propertyToProxy("TopicManager.Proxy"));
    if(!manager)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

	for(int i=0; i<TOPICNUM; i++)
	{
	    IceStorm::TopicPrx topic;
		try
		{
			topic = manager->retrieve(topicNames[i]);
		}
		catch(const IceStorm::NoSuchTopic&)
		{
			// If the topic doesn't exists, create it now.
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
	    Ice::ObjectPrx receiver = topic->getPublisher();
	    RTAWavePrx prx = RTAWavePrx::uncheckedCast(receiver);
		streams.push_back(prx);
	}
#else
    // get RTAWave 1 proxy
    try
    {
		RTAWavePrx wave1 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave1.Proxy"))->ice_oneway();
		streams.push_back(wave1);
		RTAWavePrx wave2 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave2.Proxy"))->ice_oneway();
		streams.push_back(wave2);
		RTAWavePrx wave3 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave3.Proxy"))->ice_oneway();
		streams.push_back(wave3);
		
    }
    catch(...)
    {
		cerr << appName() << ": cannot connect to the 3 RTAWave servers." << endl;
		return EXIT_FAILURE;
    }
#endif

    // get a RTAMonitor proxy
    CTA::RTAMonitorPrx monitor = 0;
    try
    {
         monitor = CTA::RTAMonitorPrx::checkedCast(communicator()->propertyToProxy("RTAMonitor.Proxy"))->ice_oneway();
    }
    catch(...)
    {
    }

    // get a RTAViewer proxy
    CTA::RTAViewerPrx viewer = 0;
    try
    {
         viewer = CTA::RTAViewerPrx::checkedCast(communicator()->propertyToProxy("RTAViewer.Proxy"))->ice_oneway();
    }
    catch(...)
    {
    }

    // start the MonitorThread
    size_t byteSent = 0;
	std::vector<int> triggeredEvent;
	int nevent = 0;
    IceUtil::Mutex mutex;
    IceUtil::ThreadPtr monitorThread = new RTAMonitorThread(monitor, byteSent, mutex);
    monitorThread->start();

	// Use a CTADecoder
	RTATelem::CTADecoder decoder(argv[1]);

	// Create an adapter for RTAReceiver
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTAReceiver");
    RTAReceiverPtr servant = new RTAReceiverI(decoder, streams, byteSent, mutex, viewer, triggeredEvent, nevent);
    adapter->add(servant, communicator()->stringToIdentity("receiver"));
    adapter->activate();

    communicator()->waitForShutdown();

    return EXIT_SUCCESS;
}
