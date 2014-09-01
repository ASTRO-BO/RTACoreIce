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

//#define USE_ICESTORM 1
//#define SHOWROOTCANVAS 1
//#define PRINTALG 1

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <limits>
#include <RTAWave.h>
#include <RTAViewCamera.h>
#include "packet/PacketLibDefinition.h"

#ifdef USE_ICESTORM
#include <IceStorm/IceStorm.h>
#endif

#include <TCanvas.h>
#include <TH1D.h>
#include <TLine.h>

using namespace std;
using namespace CTA;
using namespace PacketLib;

class WaveFormAlgorithm {
	
public:
	
	WaveFormAlgorithm() {
	};

	bool iszero(double someValue) {
		if(someValue == 0)
			return true;
		if (someValue <  std::numeric_limits<double>::epsilon() &&
			someValue > -std::numeric_limits<double>::epsilon())
			return true;
		return false;
	};

	void printCamera(word* c, int ssc, int npixels, int nsamples) {
		cout << "ssc " << ssc << endl;
		for(int pixel = 0; pixel<npixels; pixel++) {
			cout << pixel << " ";
			for(int j=0; j<nsamples; j++)
				cout << c[pixel * nsamples + j] << " ";
			cout << endl;
		}
		
	};

	void printBuffer(word* c, int npixels, int nsamples) {
		for(int pixel = 0; pixel<npixels; pixel++) {
			cout << pixel << " ";
			for(int j=0; j<nsamples; j++)
				cout << c[pixel * nsamples + j] << " ";
			cout << endl;
		}
	};

	//unsigned short maxres[3000];
	//double timeres[3000];
	
	void calcWaveformExtraction4(byte* buffer, int npixels, int nsamples, int ws, unsigned short * maxresext, unsigned short * timeresext) {
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
		
		unsigned short* maxres  = new unsigned short[npixels];
		unsigned short* timeres = new unsigned short[npixels];
		//maxresext = maxres;
		//timeresext = time;
		
		
		//word bl[npixels*nsamples];
		//memcpy(bl, b, npixels*nsamples*sizeof(word));

#ifdef PRINTALG
		std::cout << "npixels = " << npixels << std::endl;
		std::cout << "nsamples = " << nsamples << std::endl;
#endif

		for(int pixel = 0; pixel<npixels; pixel++) {
			word* s = b + pixel * nsamples;

#ifdef PRINTALG
			cout << "pixel " << pixel << " samples ";
			for(int k=0; k<nsamples; k++)
				cout << s[k] << " ";
			cout << endl;
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
			
			//calculate sumall
			/*
			long sumall = 0;
			sumall = sumd;
			for(int j=ws; j<npixels; j++)
			sumall += s[j];
			*/
			
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
			
			
			maxres[pixel] = max; //or sumall
			timeres[pixel] =  maxt; //or maxj
			
#ifdef PRINTALG
			cout << "result " << maxt << " " << maxres[pixel] << " " << timeres[pixel] << " " << endl;
#endif
			/*
			 for(int k=0; k<nsamples; k++)
			 cout << s[k] << " ";
			 cout << endl;
			 */
		}
		//SharedPtr<double> shtime(maxt);
		//return maxres;
		//maxresext = maxres;
		//timeresext = timeres;
		//
		
		memcpy(maxresext, maxres, sizeof(unsigned short) * npixels);
		//memcpy(timeresext, timeres, sizeof(unsigned short) * npixels);
	};
	
	void calcWaveformExtraction3(byte* buffer, int npixels, int nsamples, int ws, dword * maxresext, float * timeresext) {
		word *b = (word*) buffer; //should be pedestal subtractred
		
		//dword* maxres = new dword[2000];
		//double* timeres = new double[2000];
		dword maxres[npixels];
		float timeres[npixels];

#ifdef PRINTALG
		std::cout << "npixels = " << npixels << std::endl;
		std::cout << "nsamples = " << nsamples << std::endl;
#endif
		for(int pixel = 0; pixel<npixels; pixel++) {

			word* s = b + pixel * nsamples;

#ifdef PRINTALG
			cout << "pixel " << pixel << " samples ";
			for(int k=0; k<nsamples; k++)
				cout << s[k] << " ";
			cout << endl;
#endif
			
			long maxd = 0;
			long sumn = 0;
			long sumd = 0;
			int maxj = 0;
			float maxt = 0;
			
			
			for(int j=0; j<=ws-1; j++) {
				sumd += s[j];
				maxj=0;
			}
			maxd = sumd;
			
			//calculate sumall
			/*
			 long sumall = 0;
			 sumall = sumd;
			 for(int j=ws; j<npixels; j++)
			 sumall += s[j];
			 */
			
			for(int j=1; j<nsamples-ws; j++) {
				
				sumd = sumd - (s[j-1]) + (s[j+ws-1]);
				if(sumd > maxd) {
					maxd = sumd;
					maxj = j;
					
				}
			}
			
			if(!iszero(maxd)) {
				sumn=0;
				for(int k=maxj; k<maxj+ws; k++)
				sumn +=  s[k] * k;
				maxt = sumn / (float)maxd;
			}
			
			maxres[pixel] = maxd; //or sumall
			timeres[pixel] = maxj; //or maxj
			
#ifdef PRINTALG
			cout << "result " << maxt << " " << maxres[pixel] << " " << timeres[pixel] << " " << endl;
#endif

		}

		memcpy(maxresext, maxres, sizeof(dword) * npixels);
		memcpy(timeresext, timeres, sizeof(float) * npixels);
		
		//delete[] maxres;
		//delete[] timeres;
	}

	
};

#include <TApplication.h>

class RTAWaveThread : public IceUtil::Thread
{
public:
	virtual void run()
	{
		TApplication theApp("App", 0, 0);
		c1 = new TCanvas();
		h1 = 0;
		theApp.Run();
	}
	
	virtual void draw(byte* buffer, int npixels, int nsamples, int ws, dword * maxresext, float * timeresext) {
		//cout << "draw "  << endl;
		c1->cd();
		if(h1 == 0)
			h1 = new TH1D("h1", "h1", nsamples, 0, nsamples);
		dword max = maxresext[0];
		unsigned short maxi = 0;
		for(int i=0; i<npixels; i++) {
			//cout << i << " " << maxresext[i] << " " << endl;
			if(maxresext[i] > max) {
				max = maxresext[i];
				maxi = i;
			}
		}
		//cout << maxi << endl;
		for(int i=0; i<nsamples; i++)
			h1->SetBinContent(i+1, buffer[nsamples * maxi + i]);
		h1->Draw("HIST");
		TLine* l = new TLine(timeresext[maxi], 0, timeresext[maxi], 200);
		l->Draw();
		c1->Update();
	}
private:
	TH1D* h1;
	TCanvas* c1;
};

class RTAWaveI : public RTAWave
{
public:
	RTAWaveI(int serverNum, CTA::RTAViewCameraPrx& viewCamera) : _serverNum(serverNum), _viewCamera(viewCamera)
	{
		wave = new WaveFormAlgorithm;
		nevents = 0;
#ifdef SHOWROOTCANVAS
		if(_serverNum == 0) {
			threadDraw= new RTAWaveThread();
			threadDraw->start();
		} else
#endif
			threadDraw = 0;
		
		
	}

    virtual void send(Ice::Int nPixels, Ice::Int nSamples, const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
		//ByteStreamPtr cameraPtr = ByteStreamPtr(new ByteStream((byte*)seqPtr.first, seqPtr.second-seqPtr.first, false));
        //cout << pixelNum << endl;
		int ws = 6;
		/*
		unsigned short * maxres = new unsigned short[nPixels];
		unsigned short* timeres = new unsigned short[nPixels];
		wave->calcWaveformExtraction4((byte*)seqPtr.first, nPixels, nSamples, ws, maxres, timeres);
		*/
		dword * maxres = new dword[nPixels];
		float* timeres = new float[nPixels];
		wave->calcWaveformExtraction3((byte*)seqPtr.first, nPixels, nSamples, ws, maxres, timeres);
		
		
#ifdef SHOWROOTCANVAS
		 nevents++;
		if(nevents == 10000) {
			//cout << "draw 1" << endl;
			if(threadDraw) threadDraw->draw((byte*)seqPtr.first, nPixels, nSamples, ws, maxres, timeres);
			nevents = 0;
		}
#endif
		nevents++;
		if(nevents == 10000)
		if(_viewCamera)
		{
			try {
				std::vector<short> tmp;
				tmp.resize(nPixels);
				for(unsigned int i=0; i<nPixels; i++)
					tmp[i]=maxres[i];
				_viewCamera->update(tmp);
			}
			catch(Ice::ConnectionRefusedException& e)
			{
				// something goes wrong with the monitor
				std::cout << "The monitor has gone.." << std::endl;
				_viewCamera = 0;
			}
			nevents = 0;
		}

		delete[] maxres;
		delete[] timeres;
    }
	virtual void sendGPU(Ice::Int nPixels, Ice::Int nSamples, const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
    {
        
    }
	
	

private:
	int _serverNum;
	long nevents;
	//display
	RTAWaveThread* threadDraw;
	WaveFormAlgorithm* wave;
    CTA::RTAViewCameraPrx& _viewCamera;
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
    return app.main(argc, argv);
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

    // get a RTAViewCamera proxy
    CTA::RTAViewCameraPrx viewcamera = 0;
    try
    {
         viewcamera = CTA::RTAViewCameraPrx::checkedCast(communicator()->propertyToProxy("RTAViewCamera.Proxy"))->ice_oneway();
    }
    catch(...)
    {
    }

	// Create an adapter for RTAWave
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("RTAWave");
    RTAWavePtr servant = new RTAWaveI(serverNum, viewcamera);
    adapter->add(servant, communicator()->stringToIdentity("wave"));
    adapter->activate();
#endif

    shutdownOnInterrupt();
    communicator()->waitForShutdown();

    return EXIT_SUCCESS;
}
