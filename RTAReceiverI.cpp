/***************************************************************************
    begin                : Mar 07 2014
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

#include "RTAReceiverI.h"
#include <iostream>
#include <packet/ByteStream.h>
#include <CTACameraTriggerData1.h>

using namespace CTA;
using namespace PacketLib;

#ifdef USESHM
// shm
#include "shm_common.h"
#include <sys/ipc.h>
#include <sys/shm.h>

int RTAReceiverI::initShm() {
	// Create a virtual memory from the shm (of key shmKey)
	key_t key = shmKey;
	int shmid = shmget(key, shmSize, IPC_CREAT | 0666);
	if (shmid < 0) {
		cerr << "Failure in shmget" << endl;
		return 0;
	}
	unsigned char* shm = (unsigned char*) shmat(shmid, NULL, 0);
	unsigned char* shmPtr = shm;

	// Destroy existing semaphores
	sem_unlink(semFullName);
	sem_unlink(semEmptyName);
	
	// Create semaphores
	full = sem_open(semFullName, O_CREAT, 0644, 0);
	if (full == SEM_FAILED) {
		cerr << "Unable to create full semaphore" << endl;
		return 0;
	}
	empty = sem_open(semEmptyName, O_CREAT, 0644, 1);
	if (empty == SEM_FAILED) {
		cerr << "Unable to create empty semaphore" << endl;
		return 0;
	}
	
	// Allocate space for activatememorycopy
	byte* buffermemory = new byte[2000*50*sizeof(word)];
		
	int test = *((int*)shmPtr);
	shmPtr += sizeof(int);
	bool activatememorycopy = *((bool*)shmPtr);
	shmPtr += sizeof(bool);
	bool calcalg = *((bool*)shmPtr);
	shmPtr += sizeof(bool);
	
	sizeShmPtr = (dword*)shmPtr;
	bufferShmPtr = (byte*)shmPtr+sizeof(dword);
	cout << "SHM initialized" << endl;
	return 1;

}

void RTAReceiverI::sendShm() {
	while(1) {
		//cout << "wait..." << endl;
		sem_wait(full);
		ByteStreamPtr buffPtr = ByteStreamPtr(new ByteStream(bufferShmPtr, *sizeShmPtr, false));
		//cout << "packet received" << endl;
		//TODO check sizeShmPtr
		std::pair<unsigned char*, unsigned char*> seqPtr(buffPtr->getStream(), buffPtr->getStream()+ *sizeShmPtr);
		Ice::Current cur;
		//TODO check cur
		send(seqPtr, cur);
		sem_post(empty);
	}
}
#endif

void RTAReceiverI::send(const std::pair<const unsigned char*, const unsigned char*>& seqPtr, const Ice::Current& cur)
{
	const int LARGE = 0;
	const int MEDIUM = 1;
	const int SMALL = 2;

	
	ByteStreamPtr streamPtr = ByteStreamPtr(new ByteStream((byte*)seqPtr.first, seqPtr.second-seqPtr.first, false));
	_nevent++;
	
	//questo pezzo deve essere thread safe (fino al telId)
	_mutex.lock();
	
	RTATelem::CTAPacket& packet = _decoder.getPacket(streamPtr);
	enum RTATelem::CTAPacketType type = packet.getPacketType();

	// skipping packets of camera type different from triggerdata1
	if(type != RTATelem::CTA_CAMERA_TRIGGERDATA_1)
	{
		std::cout << "Warning: Skipping packet from a camera of type " << type << std::endl;
		return;
	}
	 
	RTATelem::CTACameraTriggerData1& trtel = (RTATelem::CTACameraTriggerData1&) packet;
	word telId = trtel.getTelescopeId();
	ByteStreamPtr camera = trtel.getCameraDataSlow();
	//cout << trtel.getTelescopeId() << endl;
	
	if(_viewer && collectevt)
	{
		word evtnum = trtel.getEventNumber();
		word tt = trtel.getIndexOfCurrentTriggeredTelescope();
		word ntt = trtel.getNumberOfTriggeredTelescopes();

		//cout << lastEvtNum << " " << evtnum << " " << tt << " " << ntt << endl;
		if(evtnum != lastEvtNum) {
			word tt = trtel.getIndexOfCurrentTriggeredTelescope();
			word ntt = trtel.getNumberOfTriggeredTelescopes();
			//cout << tt << " " << ntt << endl;
			_triggeredEvent.push_back(tt);
			
			if(_triggeredEvent.size() == ntt) {
				
				std::cout << "[";
				for(unsigned int i=0; i<_triggeredEvent.size(); i++)
					std::cout << _triggeredEvent[i] << ", ";
				std::cout << "] evtnum = " << evtnum << std::endl;
				
				_viewer->update(_triggeredEvent, evtnum);
				collectevt = false;
			}
			
		}
	}
	
	if(_viewer && _nevent % 1000 == 0) {
		collectevt = true;
		lastEvtNum = trtel.getEventNumber();
		_triggeredEvent.resize(0);
	}
	
	//TODO: add RTAConfig instead of read npixels and nsamples from source packets
	int npixels;
	int nsamples;
	npixels = trtel.getNumberOfPixels();
	int pixel = 0;
	nsamples = trtel.getNumberOfSamples(pixel);
	
	_mutex.unlock();
	
	/*
	if(_viewer && _nevent % 1000 == 0) {
		word evtnum = 4;
		cout << "send to viewer " << _nevent << " ";
		word index = (word)(rand() % 50);
		_triggeredEvent.push_back(index);
		cout << index << " ";
		index = (word)(rand() % 50);
		_triggeredEvent.push_back(index);
		cout << index << " ";
		index = (word)(rand() % 50);
		_triggeredEvent.push_back(index);
		cout << index << " ";
		index = (word)(rand() % 50);
		_triggeredEvent.push_back(index);
		cout << index << " ";
		
		_viewer->update(_triggeredEvent, evtnum);
		_triggeredEvent.resize(0);
		cout << endl;
	}
	*/

	
	//cout << npixels << " " << nsamples << endl;
	/*
	int teltype = LARGE; // TODO settare il type in base al numero di pixel/samples

	if(teltype == LARGE)
		std::cout << "% - A Large Telescope triggered - Process A activating" << std::endl;
	else if(teltype == MEDIUM)
		std::cout << "% - A Medium Telescope triggered - Process B activating" << std::endl;
	else if(teltype == SMALL)
		std::cout << "% - A Small Telescope triggered - Process C activating" << std::endl;
*/
	//int teltype = SMALL;
	RTAConfig::RTAConfigLoad::Telescope* telStruct = ctaconf->getTelescopeStruct(telId);
	//cout << npixels << " " << nsamples << " - "  << telStruct->fromTeltoTelType.fromTelTypetoCamType.NPixel << " " << telStruct->fromTeltoTelType.fromTelTypetoCamType.fromCameratoPixType.NSamples <<endl;
	int64_t telTypeSim = telStruct->fromTeltoTelType.TelType;
	
	struct RTAConfig::RTAConfigLoad::TelescopeType *telTypeStruct;
	struct RTAConfig::RTAConfigLoad::CameraType *camTypeStruct;
	struct RTAConfig::RTAConfigLoad::MirrorType *mirTypeStruct;
	
	telTypeStruct = ctaconf->getTelescopeTypeStruct(telTypeSim);
	mirTypeStruct = ctaconf->getMirrorTypeStruct((*telTypeStruct).fromTelTypetoMirType.mirType);
	
	
	camTypeStruct = ctaconf->getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
	// The attribute stores the number of pixels
	npixels =  (*camTypeStruct).NPixel;
	
	struct RTAConfig::RTAConfigLoad::PixelType *pixelTypeStruct = ctaconf->getPixelTypeStruct((*camTypeStruct).fromCameratoPixType.pixType);
	//word nsamples = (*pixelTypeStruct).NSamples;
	nsamples = (*pixelTypeStruct).NSamples;
	//cout << npixels << " " << nsamples  << endl;
	
	
	//cout << telTypeSim << endl;
	
	int teltype;// = (int)(rand() % 3);
	
	switch(telTypeSim) {
		case 10408418:
			teltype = MEDIUM;
			break;
		case 3709425:
			teltype = SMALL;
			break;
		case 138704810:
			teltype = LARGE;
			break;
	}
	
	/*if(teltype == LARGE) {
		npixels = 1141;
		nsamples = 40;
	}
	if(teltype == MEDIUM) {
		npixels = 1141;
		nsamples = 40;
	}
	if(teltype == SMALL) {
		npixels = 1141;
		nsamples = 40;
	}
	*/
	size_t buffsize = camera->size();
	std::pair<unsigned char*, unsigned char*> seqPtrCamera(camera->getStream(), camera->getStream()+buffsize);
	_streams[teltype]->send(npixels, nsamples, seqPtrCamera);
	//_streams[teltype]->send2(seqPtr);
	
	_mutex.lock();
	_byteSent += streamPtr->size();
	_mutex.unlock();
}
