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
#include "packet/PacketLibDefinition.h"

using namespace std;
using namespace CTA;
using namespace PacketLib;

bool iszero(double someValue) {
	if(someValue == 0)
		return true;
	if (someValue <  std::numeric_limits<double>::epsilon() &&
    	someValue > -std::numeric_limits<double>::epsilon())
    	return true;
    return false;
}

void printCamera(word* c, int ssc, int npixels, int nsamples) {
	cout << "ssc " << ssc << endl;
	for(int pixel = 0; pixel<npixels; pixel++) {
		cout << pixel << " ";
		for(int j=0; j<nsamples; j++)
			cout << c[pixel * nsamples + j] << " ";
		cout << endl;
	}
	
}

void printBuffer(word* c, int npixels, int nsamples) {
	for(int pixel = 0; pixel<npixels; pixel++) {
		cout << pixel << " ";
		for(int j=0; j<nsamples; j++)
			cout << c[pixel * nsamples + j] << " ";
		cout << endl;
	}
}


//#define PRINTALG 1
int flag = 0;
//unsigned short maxres[3000];
//double timeres[3000];

void calcWaveformExtraction1(byte* buffer, int npixels, int nsamples, int ws, unsigned short * maxresext, double * timeresext) {
	word *b = (word*) buffer; //should be pedestal subtractred
	//printBuffer(b, npixels, nsamples);
	
	/*
	 vector<int> maxresv;
	 maxresv.reserve(npixels);
	 vector<double> timev;
	 timev.reserve(npixels);
	 int* maxres = &maxresv[0];
	 double* time = &timev[0];
	 */
	
	unsigned short* maxres = new unsigned short[npixels];
	double* timeres = new double[npixels];
	//maxresext = maxres;
	//timeresext = time;
	
	
	//word bl[npixels*nsamples];
	//memcpy(bl, b, npixels*nsamples*sizeof(word));
	
	for(int pixel = 0; pixel<npixels; pixel++) {
		word* s = b + pixel * nsamples;
		
#ifdef PRINTALG
		if(flag == 0) {
			
			cout << pixel << " ";
			for(int k=0; k<nsamples; k++)
				cout << s[k] << " ";
			cout << endl;
		}
#endif
		
		unsigned short max = 0;
		double maxt = 0;
		long sumn = 0;
		long sumd = 0;
		long maxj = 0;
		double t = 0;
		//long sumres[nsamples-ws];
		
		for(int j=0; j<=ws-1; j++) {
			sumn += s[j] * j;
			sumd += s[j];
		}
		
		max = sumd;
		if(!iszero(sumd))
			t = sumn / (double)sumd;
		maxt = t;
		maxj = 0;
		
		//cout << sumn << " " << sumd << endl;
		//sumres[0] = sum;
		for(int j=1; j<nsamples-ws; j++) {
			
			sumn = sumn - s[j-1] * (j-1) + s[j+ws-1] * (j+ws-1);
			sumd = sumd - s[j-1] + s[j+ws-1];
			//cout << sumn << " " << sumd << endl;
			
			//sumres[j] = sum;
			if(sumd > max) {
				max = sumd;
				if(!iszero(sumd))
					t = sumn / (double)sumd;
				maxt = t;
				maxj = j;
			}
			
			
		}
		
		/*for(int j=0; j<nsamples-ws; j++)
		 if(sumres[j]>max) {
		 max = sumres[j];
		 maxj = j;
		 }
		 */
		
		
		//maxres.push_back(max);
		//time.push_back(maxt);
		
		
		maxres[pixel] = max;
		timeres[pixel] = maxt;
		
#ifdef PRINTALG
		//>9000
		if(flag == 0) cout << pixel << " " << maxt << " " << maxres[pixel] << " " << time[pixel] << " " << endl;
#endif
		/*
		 for(int k=0; k<nsamples; k++)
		 cout << s[k] << " ";
		 cout << endl;
		 */
	}
	//SharedPtr<double> shtime(maxt);
	
	flag++;
	//return maxres;
	//maxresext = maxres;
	//timeresext = timeres;
	memcpy(maxresext, maxres, sizeof(unsigned short) * npixels);
	//memcpy(timeresext, timeres, sizeof(double) * npixels);
}




class RTAWaveI : public RTAWave
{
public:

    virtual void send(Ice::Int nPixels, Ice::Int nSamples, const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
		//ByteStreamPtr cameraPtr = ByteStreamPtr(new ByteStream((byte*)seqPtr.first, seqPtr.second-seqPtr.first, false));
        //cout << pixelNum << endl;
		unsigned short * maxres = new unsigned short[nPixels];
		double* timeres = new double[nPixels];
		calcWaveformExtraction1((byte*)seqPtr.first, nPixels, nSamples, 6, maxres, timeres);
		delete[] maxres;
		delete[] timeres;
    }
	virtual void send2(const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
        //cout << pixelNum << endl;
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
