#ifndef NETWORK_IMPLEMENTATION_INCLUDED
#define NETWORK_IMPLEMENTATION_INCLUDED
enum NETWORK_ERROR_CODES {
	NET_OK = 0,
	NET_ERROR = 1
};
enum NETWORK_RECEIVER_TYPES {
	NET_REC_JUMPER = 1,
	NET_REC_AIRPLANE = 2,
	NET_REC_PLAYER = 3,			// whichever actor the player is controlling. This is normally for network_id
	NET_REC_MISSION = 4
};
struct NetworkData {
	float timestamp;
	int receiver_type;		// NETWORK_RECEIVER_TYPES
	int receiver_id;		// unique id of any receiver
	int data_type;			// arbitary data type identifier
	int data_length;		// length of data
	NetworkData *next;		// pointer to next packet in linked list
	NetworkData *prev;		// pointer to previous packet in linked list
	void *data;				// data pointer
};


#include <SFML/System.hpp>
#include "SFML/Network.hpp"
#include <iostream>
#include <functional>
#include <stdio.h>
#include <string.h>
#include <sstream>
using namespace std;

#include "headers.h"
#include "../shared/ccor.h"

using namespace ccor; // don't know if needed ???

class Network {
private:
	char *_hostname;
	int _port;
	sf::Http	_http;				// HTTP socket
			
	NetworkData *_packets_sending;	// packets for sending
	NetworkData *_packets;			// received packets

	sf::Thread				*_thread_sending;
	bool					_stop_sending;		// thread checks if this is true and returns
	bool					_packets_locked;
	bool					_packets_sending_locked;

	ostringstream 		_buffer;
	ostringstream 		_aux_buffer;
	int					_buffer_packet_i;
public:
	// constructor
	Network(void);
	// destructor
	~Network(void);

	// setters / getters
	void setHostname(const char* hostname);
	const char* getHostname(void);
	void setPort(unsigned int port);
	unsigned int  getPort(void);
	void lockPackets() { _packets_locked = true; }
	void unlockPackets() { _packets_locked = false; }
	bool arePacketsLocked() { return _packets_locked; }
	void lockPacketsSending() { _packets_sending_locked = true; }
	void unlockPacketsSending() { _packets_sending_locked = false; }
	bool arePacketsSendingLocked() { return _packets_sending_locked; }

	// attempt connection
	// returns NETWORK_ERROR_CODES
	unsigned int connect(void);

	// true if connected active, false otherwise
	bool isConnected(void);

	// finds a packet by searching for receiver_type and receiver_id
	// or NULL if there aren't any left or never have existed
	NetworkData *consumePacketByReceiver(int receiver_type, int receiver_id = 0);

	// checks the stream and retrieves all new packets and stores them in _packets list
	void receivePackets(void);

	// send the packet not asynchronously (used by sendingThread())
	void sendPacket(NetworkData *packet);

	// adds a packet to sending buffer
	// which is checked and sent by sendingThread()
	// which must be enabled by beginSending()
	void sendPacketAsync(NetworkData *packet);

	// converts packet information to raw auxilary buffer ready-to-send data (html post)
	void addPacketToBuffer(NetworkData *packet);
	// switches aux buffer with main buffer. Then the main gets sent and aux can be written with new data
	void switchBuffers();
	void sendBuffer();

	// threaded sending functions
	void beginSending(void);
	void stopSending(void);
	void sendingThread();
};

#endif