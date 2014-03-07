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
#include <CTACameraTriggerData.h>
#include "RTAReceiverI.h"

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
    return app.main(argc, argv, "config.receiver");
}

int RTAReceiver_Ice::run(int argc, char* argv[])
{
	if(argc != 3)
	{
		std::cerr << "Error: wrong number of arguments. Usage:" << std::endl;
		std::cerr << "./RTAReceiver_Ice file.stream file.raw" << std::endl;
		return EXIT_FAILURE;
	}

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

	// Use the CTACameraTriggerData0 class for decoding telescope types
	RTATelem::CTACameraTriggerData0 trtel(argv[1], argv[2], "");

	// Create an adapter for RTAReceiver
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTAReceiver");
    RTAReceiverPtr servant = new RTAReceiverI(trtel, streams);
    adapter->add(servant, communicator()->stringToIdentity("receiver"));
    adapter->activate();

    communicator()->waitForShutdown();

    return EXIT_SUCCESS;
}
