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

#include <RTAWave.h>

using namespace std;
using namespace CTA;

class RTAWaveI : public RTAWave
{
public:

    virtual void send(Ice::Int pixelNum, Ice::Int pixelSize, const CTA::ByteSeq& seq, const Ice::Current& curr)
    {
        cout << pixelNum << endl;
    }
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
    return app.main(argc, argv, "config.sub");
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
    Ice::ObjectPrx wave = adapter->add(new RTAWaveI, subId);

    //
    // Activate the object adapter before subscribing.
    //
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

    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    topic->unsubscribe(wave);

    return EXIT_SUCCESS;
}
