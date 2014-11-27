#include "Streamer.hpp"     // CTATools streamer
#include "L0.pb.h"          // CTATools L0 data model
#include "RTAConfigLoad.h"
#include "RTAWave.h"
#include "RTAMonitorThread.h"

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

class RTAReceiverZMQ : public Ice::Application
{
public:

    RTAReceiverZMQ() {}

    virtual int run(int, char*[]);
};

int RTAReceiverZMQ::run(int argc, char* argv[])
{
	// load config
	const char* str = std::getenv("CTARTA");
	if(!str)
	{
		std::cerr << "Error: CTARTA env variable is not set." << std::endl;
		return EXIT_FAILURE;
	}
	std::string ctarta = str;
	RTAConfig::RTAConfigLoad ctaconf(ctarta + "/share/rtatelem/PROD2_telconfig.fits.gz");

	// shared variables with monitor thread
	size_t byteSent = 0;
	IceUtil::Mutex mutex;

	// start a monitor thread to send data to the monitor
    try
    {
		CTA::RTAMonitorPrx monitor = CTA::RTAMonitorPrx::checkedCast(communicator()->propertyToProxy("RTAMonitor.Proxy"))->ice_oneway();
		IceUtil::ThreadPtr monitorThread = new RTAMonitorThread(monitor, byteSent, mutex);
		monitorThread->start();
	}
	catch(...)
	{
		// ignore if there isn't a monitor
	}

	// connect to the RTAWave servers
	std::vector<CTA::RTAWavePrx> streams;
    try
    {
		CTA::RTAWavePrx wave1 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave1.Proxy"))->ice_oneway();
		streams.push_back(wave1);
		CTA::RTAWavePrx wave2 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave2.Proxy"))->ice_oneway();
		streams.push_back(wave2);
		CTA::RTAWavePrx wave3 = CTA::RTAWavePrx::checkedCast(communicator()->propertyToProxy("RTAWave3.Proxy"))->ice_oneway();
		streams.push_back(wave3);
    }
    catch(...)
    {
		std::cerr << appName() << ": cannot connect to the 3 RTAWave servers." << std::endl;
		return EXIT_FAILURE;
    }

	// open a ZMQ_PULL connection with the DAQ
	zmq::context_t context;
	zmq::socket_t* sock = new zmq::socket_t(context, ZMQ_PULL);
	sock->connect("tcp://localhost:12345");
	zmq::message_t message;

	while(true)
	{
		sock->recv(&message);

		CTADataModel::CTAMessage ctaMsg;
		if (!ctaMsg.ParseFromArray(message.data(), message.size()))
		{
			std::cerr << "Warning: Wrong message type, are sure you are connected to a CTATools Streamer?" << std::endl;
			return 1;
		}

		CTADataModel::CameraEvent event;
		RepeatedField<int> types = ctaMsg.payload_type();
		RepeatedPtrField<std::string> buffers = ctaMsg.payload_data();
		for(int i=0; i<	buffers.size(); i++)
		{
			if(types.Get(i) == CTADataModel::CAMERA_EVENT)
			{
				event.ParseFromString(buffers.Get(i));

				// get telescope type
				RTAConfig::RTAConfigLoad::Telescope* telStruct = ctaconf.getTelescopeStruct(event.telescopeid());
				int64_t telTypeSim = telStruct->fromTeltoTelType.TelType;
				int teltype;
				switch(telTypeSim) {
					case 138704810:
					{
						teltype = 0; // large
						break;
					}
					case 10408418:
					{
						teltype = 1; // medium
						break;
					}
					case 3709425:
					{
						teltype = 2; // small
						break;
					}
					default:
					{
						std::cerr << "Warning: bad telescope type, skipping." << std::endl;
						continue;
					}
				}
				std::cout << "teltype: " << teltype << std::endl;

				// get number of pixels
				struct RTAConfig::RTAConfigLoad::TelescopeType *telTypeStruct = ctaconf.getTelescopeTypeStruct(telTypeSim);
				struct RTAConfig::RTAConfigLoad::CameraType* camTypeStruct = ctaconf.getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
				int npixels = (*camTypeStruct).NPixel;

				// get samples and number of samples
				const CTADataModel::PixelsChannel& higain = event.higain();
				const CTADataModel::WaveFormData& waveforms = higain.waveforms();
				const CTADataModel::CTAArray& samples = waveforms.samples();
				int nsamples = waveforms.num_samples();

				cout << "pixels: " << npixels << " samples: " << nsamples << endl;

				// send data to the right stream
				unsigned char *buff = (unsigned char*) samples.data().c_str();
				unsigned int buffSize = samples.data().size();
				std::pair<unsigned char*, unsigned char*> seqPtrCamera(buff, buff+buffSize);
				streams[teltype]->send(npixels, nsamples, seqPtrCamera);

				mutex.lock();
				byteSent += buffSize;
				mutex.unlock();
			}
	    }
	}

    communicator()->waitForShutdown();

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
    RTAReceiverZMQ receiver;
    return receiver.main(argc, argv);
}
