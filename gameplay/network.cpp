#include "network.h"

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  size_t in_len = encoded_string.size();
  size_t i = 0;
  size_t j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

#include <iostream>
#include <sstream>
Network::Network() {
	_hostname = NULL;
	_port = 80;
	_packets = NULL;
	_thread_sending = NULL;
	_packets_sending = NULL;
	//_http.setHost("localhost");
	_http.setHost("procescontrol.com");
	_stop_sending = false;
	_packets_locked = false;
	_packets_sending_locked = false;
	
	_buffer.str("");
	_aux_buffer.str("");
	_buffer_packet_i = 0;
}
Network::~Network() {
	// delete string
	if (_hostname != NULL) {
		delete _hostname;
	}

	// stop threads
	if (_thread_sending != NULL) {
		_stop_sending = true;
		delete _thread_sending;
	}

	// delete remaining packets
	lockPackets();
	lockPacketsSending();

	if (_packets != NULL) {
		NetworkData *packet = _packets;
		while (packet != NULL) {
			NetworkData *d = packet;
			packet = packet->next;
			delete d;
		}
	}
	if (_packets_sending != NULL) {
		NetworkData *packet = _packets_sending;
		while (packet != NULL) {
			NetworkData *d = packet;
			packet = packet->next;
			delete d;
		}
	}
}

// setters / getters
void Network::setHostname(const char* hostname) {
	if (_hostname != NULL) {
		delete _hostname;
	}
	_hostname = new char[strlen(hostname)+1];
	strcpy_s(_hostname, strlen(hostname), hostname);

	_http.setHost(_hostname);
}
const char* Network::getHostname(void) {
	return _hostname;
}
void Network::setPort(unsigned int port) {
	_port = port;
}
unsigned int Network::getPort(void) {
	return _port;
}

// attempt connection
// returns NETWORK_ERROR_CODES
unsigned int Network::connect(void) {
	return NET_OK;
}

// true if connected active, false otherwise
bool Network::isConnected(void) {
	return true;
}

// finds a packet by searching for receiver_type and receiver_id
// or NULL if there aren't any left or never have existed
NetworkData *Network::consumePacketByReceiver(int receiver_type, int receiver_id) {
	////getCore()->logMessage("CONSUMER WANTS PACKETS (%d, %d)", receiver_type, receiver_id);

	// check lock
	if (arePacketsLocked()) {
		////getCore()->logMessage("CONSUMER PACKET DENIED");	
		return NULL;
	}
	// lock
	////getCore()->logMessage("LOCK PACKETS");
	lockPackets();

	NetworkData *packet = _packets;
	while (packet != NULL) {
		if (packet->receiver_type == receiver_type && (packet->receiver_id == receiver_id || receiver_id == 0)) {
			NetworkData *ret_packet = new NetworkData(*packet);
			packet->data = packet->data;
			// remove from list
			// only node
			if (packet == _packets && _packets->next == NULL) {
				delete _packets;
				_packets = NULL;
			// first node
			} else if (packet == _packets) {
				_packets = _packets->next;
				_packets->prev = NULL;
				delete packet;
			// last node
			} else if (packet->next == NULL) {
				packet->prev->next = packet->next;
				delete packet;
			// any other node
			} else {
				packet->prev->next = packet->next;
				packet->next->prev = packet->prev;
				delete packet;
			}

			// unlock and return
			////getCore()->logMessage("CONSUMER GOT PACKET");
			////getCore()->logMessage("UNLOCK PACKETS");
			unlockPackets();
			return ret_packet;
		}

		packet = packet->next;
	}

	////getCore()->logMessage("UNLOCK PACKETS");
	////getCore()->logMessage("CONSUMER PACKET NOT FOUND (%d, %d)", receiver_type, receiver_id);
	unlockPackets();
	return NULL;
}


// checks the stream and retrieves all new packets and stores them in _packets list
void Network::receivePackets() {

	// receive packets
	//NetworkData *packet = new NetworkData;
	//packet->timestamp = time;
	//packet->receiver_type = NET_REC_JUMPER;
	//packet->receiver_id = 1;
	//packet->data_type = 1;	// position vector PxVec3
	//PxVec3 pos = PxVec3(0, 100000.0f, 0);
	//packet->data = new PxVec3;
	//memcpy(packet->data, &pos, sizeof pos);

	// add to list
	//if (_packets == NULL) {
	//	_packets = packet;
	//	_packets->prev = NULL;
	//	_packets->next = NULL;
	//} else {
	//	packet->prev = NULL;
	//	packet->next = _packets;
	//	_packets->prev = packet;
	//	_packets = packet;
	//}
}

void Network::beginSending() {
	_stop_sending = false;
	if (_thread_sending == NULL) {
		_thread_sending = new sf::Thread(std::bind(&Network::sendingThread, this));
	}
	_thread_sending->launch();
}

void Network::stopSending() {
	_stop_sending = true;
}

void Network::sendingThread() {
	////getCore()->logMessage("START SENDING");

	// check lock
	//while (arePacketsSendingLocked());
	// lock
	////getCore()->logMessage("LOCK SENDING PACKETS");
	//lockPacketsSending();
	while(!_stop_sending) {
		//Sleep(500);
		if (_buffer.str().length() > 0) {
			sendBuffer();
		}

		continue;
		//NetworkData *packet = _packets_sending;
		//while (packet != NULL) {
		//	lockPacketsSending();
		//	NetworkData *next = _packets_sending->next;
		//	_packets_sending = _packets_sending->next;
		//	if (_packets_sending != NULL) {
		//		_packets_sending->prev = NULL;
		//	}

		//	// send and forget
		//	////getCore()->logMessage("UNLOCK SENDING PACKETS");
		//	//unlockPacketsSending();
		//	////getCore()->logMessage("SEND SENDING PACKETS");
		//	this->sendPacket(packet);
		//	//getCore()->logMessage("SENT PACKET (%d, %d)", packet->receiver_type, packet->data_type);
		//	delete packet->data;
		//	delete packet;

		//	packet = next;
		//	// abort
		//	if (_stop_sending) {
		//		////getCore()->logMessage("STOP SENDING");
		//		return;
		//	}
		//
		//	// check lock
		//	//while (arePacketsSendingLocked());
		//	////getCore()->logMessage("LOCK SENDING PACKETS");
		//	//lockPacketsSending();
		//}
		unlockPacketsSending();
	}
	////getCore()->logMessage("STOP SENDING");

	// unlock
	////getCore()->logMessage("UNLOCK SENDING PACKETS");
	//unlockPacketsSending();
}
void Network::sendPacketAsync(NetworkData *packet) {
	// check lock
	if (arePacketsSendingLocked()) return;
	// lock
	lockPacketsSending();

	// try finding a packet of same receiver and same data type
	NetworkData *d = _packets_sending;
	while (_packets_sending != NULL) {
		if (d->receiver_type == packet->receiver_type && d->receiver_id == packet->receiver_id
				&& packet->timestamp >= d->timestamp) {
			d->data = packet->data;
			d->timestamp = packet->timestamp;
			return;
		}
		d = d->next;
	}

	// add to list
	if (_packets_sending == NULL) {
		_packets_sending = packet;
		_packets_sending->next = _packets_sending->prev = NULL;
	} else {
		packet->next = _packets_sending;
		packet->prev = NULL;
		_packets_sending->prev = packet;
		_packets_sending = packet;
	}

	// unlock
	unlockPacketsSending();
}
// converts packet information to raw auxilary buffer ready-to-send data (html post)
void Network::addPacketToBuffer(NetworkData *packet) {
	_aux_buffer << "packets[" << _buffer_packet_i << "][timestamp]=" << packet->timestamp << "&";
	_aux_buffer << "packets[" << _buffer_packet_i << "][receiver_type]=" << packet->receiver_type << "&";
	_aux_buffer << "packets[" << _buffer_packet_i << "][receiver_id]=" << packet->receiver_id << "&";
	
	if (packet->data_type != 0) {
		_aux_buffer << "packets[" << _buffer_packet_i << "][data_type]=" << packet->data_type << "&";
		if (packet->data != NULL && packet->data_length > 0) {
			_aux_buffer << "packets[" << _buffer_packet_i << "][data]=";
			string buf;
			buf = base64_encode(reinterpret_cast<const unsigned char*>(packet->data), sizeof(int) * packet->data_length);
			_aux_buffer << buf.c_str();
			_aux_buffer << "&";
			_aux_buffer << "packets[" << _buffer_packet_i << "][data_length]=" << buf.length() << "&";
		}
	}

	++_buffer_packet_i;
	return;

	//_aux_buffer << "packets[" << _buffer_packet_i << "][timestamp]=" << packet->timestamp << "&";
	//_aux_buffer << "packets[" << _buffer_packet_i << "][receiver_type]=" << packet->receiver_type << "&";
	//_aux_buffer << "packets[" << _buffer_packet_i << "][receiver_id]=" << packet->receiver_id << "&";
	//_aux_buffer << "packets[" << _buffer_packet_i << "][data_length]=" << packet->data_length << "&";
	//if (packet->data_type != 0) {
	//	_aux_buffer << "packets[" << _buffer_packet_i << "][data_type]=" << packet->data_type << "&";
	//	if (packet->data != NULL && packet->data_length > 0) {
	//		_aux_buffer << "packets[" << _buffer_packet_i << "][data]=";
	//		_aux_buffer.rdbuf()->sputn(reinterpret_cast<char*>(packet->data), sizeof(int) * packet->data_length);
	//		_aux_buffer << "&";
	//	}
	//}
	//
	//++_buffer_packet_i;
}
// switches aux buffer with main buffer. Then the main gets sent and aux can be written with new data
void Network::switchBuffers() {
	_buffer_packet_i = 0;
	_buffer.str(_aux_buffer.str());
	_aux_buffer.str("");
}
void Network::sendBuffer() {
	// prepare request
	sf::Http::Request request;
	request.setMethod(sf::Http::Request::Post);
	//request.setUri("/basepro/send.php");
	request.setUri("/asfl/test/send.php");
	request.setHttpVersion(1, 1); // HTTP 1.1
	request.setField("Content-Type", "application/x-www-form-urlencoded");

	request.setBody(_buffer.str());
	_buffer.str("");
	sf::Http::Response response = _http.sendRequest(request);
	
	//BASEPro-packet
	//BASEPro-packet-count
	// packet sent back?
	if (response.getField("basepro-packet").compare("data/plain") == 0) {
		// if packets are locked, just ignore the replies
		while (arePacketsLocked() && !_stop_sending);
		// lock packets
		//////getCore()->logMessage("LOCK PACKETS\n");
		lockPackets();

		std::string buf = response.getField("basepro-packet-count");
		int packet_count = atoi(buf.c_str());
		//getCore()->logMessage("Received back %d packets\n", packet_count);
		
		std::istringstream reply(response.getBody());
		std::string line;

		line = response.getBody().c_str();
		getCore()->logMessage("BODY: %s\n\n", line.c_str());
		while (packet_count) {
			// skip to packet header
			while (line.compare("__LOOK_AT_THE_SKY__") != 0 && !reply.eof()) {
				getline(reply, line);
			}
			// check for end of stream
			if (reply.eof()) break;
			
			// read first line
			getline(reply, line);

			// create packet object
			NetworkData *packet = new NetworkData;

			// start reading
			getCore()->logMessage("timestamp: %s", line.c_str());
			//packet->timestamp = atoi(line.c_str());
			packet->timestamp = stof(line);

			getline(reply, line);
			getCore()->logMessage("receiver_type: %s", line.c_str());
			packet->receiver_type = atoi(line.c_str());

			getline(reply, line);
			getCore()->logMessage("receiver_id: %s", line.c_str());
			packet->receiver_id = atoi(line.c_str());

			getline(reply, line);
			getCore()->logMessage("data_type: %s", line.c_str());
			packet->data_type = atoi(line.c_str());

			getline(reply, line);	//data_length
			packet->data_length = atoi(line.c_str());
			getCore()->logMessage("data_length: %d", packet->data_length);

			string buf;
			getline(reply, buf);
			getCore()->logMessage("buf: %s", buf.c_str());

			buf = base64_decode(buf);
			packet->data = new unsigned char[buf.length()];
			memcpy(packet->data, buf.c_str(), buf.length());
			//reply.read((char*)packet->data, packet->data_length);
			//packet->data = base64_decode(packet->data);
			getCore()->logMessage("data: %s", (char*)packet->data);

			// add packet to list
			getCore()->logMessage("ADDING PACKETS\n");
			////getCore()->logMessage("GOT PACKET(%d, %d) data_type: %d\n", packet->receiver_type, packet->receiver_id, packet->data_type);
			// add to list
			if (_packets == NULL) {
				_packets = packet;
				_packets->prev = NULL;
				_packets->next = NULL;
			} else {
				packet->prev = NULL;
				packet->next = _packets;
				_packets->prev = packet;
				_packets = packet;
			}

			--packet_count;
		}
		// unlock
		//////getCore()->logMessage("UNLOCK PACKETS\n");
		unlockPackets();
	}
	
}

void Network::sendPacket(NetworkData *packet) {
	// prepare request
	sf::Http::Request request;
	request.setMethod(sf::Http::Request::Post);
	request.setUri("/basepro/send.php");
	//request.setUri("/asfl/test/send.php");
	request.setHttpVersion(1, 1); // HTTP 1.1
	request.setField("Content-Type", "application/x-www-form-urlencoded");

	// create body
	std::ostringstream body;
	body << "timestamp=" << packet->timestamp << "&";
	body << "rec_type=" << packet->receiver_type << "&";
	body << "rec_id=" << packet->receiver_id << "&";
	body << "data_length=" << packet->data_length << "&";
	if (packet->data_type != 0) {
		body << "data_type=" << packet->data_type << "&";
		if (packet->data != NULL && packet->data_length > 0) {
			body << "data=";
			body.rdbuf()->sputn(reinterpret_cast<char*>(packet->data), sizeof(int) * packet->data_length);
			body << "&";
		}
	}

	request.setBody(body.str());
	sf::Http::Response response = _http.sendRequest(request);
	
	//BASEPro-packet
	//BASEPro-packet-count
	// packet sent back?
	if (response.getField("basepro-packet").compare("data/plain") == 0) {
		// if packets are locked, just ignore the replies
		while (arePacketsLocked() && !_stop_sending);
		// lock packets
		////getCore()->logMessage("LOCK PACKETS");
		lockPackets();

		std::string buf = response.getField("basepro-packet-count");
		int packet_count = atoi(buf.c_str());
		////getCore()->logMessage("Received back %d packets", packet_count);
		
		std::istringstream reply(response.getBody());
		std::string line;
		//getline(reply, line);	// ignore first line (???)
		////getCore()->logMessage("Line0: %s", line.c_str());
		line = response.getBody().c_str();
		getline(reply, line);		// read first line. in next iterations, first line will be read by the end of cycle
		while (packet_count) {
			NetworkData *packet = new NetworkData;

			////getCore()->logMessage("timestamp: %s", line.c_str());
			//packet->timestamp = atoi(line.c_str());
			packet->timestamp = stof(line);

			getline(reply, line);
			////getCore()->logMessage("receiver_type: %s", line.c_str());
			packet->receiver_type = atoi(line.c_str());

			getline(reply, line);
			////getCore()->logMessage("receiver_id: %s", line.c_str());
			packet->receiver_id = atoi(line.c_str());

			getline(reply, line);
			////getCore()->logMessage("data_type: %s", line.c_str());
			packet->data_type = atoi(line.c_str());

			getline(reply, line);	//data_length
			packet->data_length = atoi(line.c_str());
			////getCore()->logMessage("data_length: %d", packet->data_length);

			packet->data = new unsigned char[packet->data_length];
			reply.read((char*)packet->data, packet->data_length);
			////getCore()->logMessage("data: %s", (char*)packet->data);

			// skip new lines at the end of data
			//getCore()->logMessage("");
			getline(reply, line);
			while(packet_count > 1 && line.empty() && !reply.eof()) {
				getline(reply, line);
				////getCore()->logMessage("LOOP");
			}

			// add packet to list
			////getCore()->logMessage("ADDING PACKETS");
			//getCore()->logMessage("GOT PACKET(%d, %d) data_type: %d", packet->receiver_type, packet->receiver_id, packet->data_type);
			// add to list
			if (_packets == NULL) {
				_packets = packet;
				_packets->prev = NULL;
				_packets->next = NULL;
			} else {
				packet->prev = NULL;
				packet->next = _packets;
				_packets->prev = packet;
				_packets = packet;
			}

			--packet_count;
		}
		// unlock
		////getCore()->logMessage("UNLOCK PACKETS");
		unlockPackets();
	}
	
}