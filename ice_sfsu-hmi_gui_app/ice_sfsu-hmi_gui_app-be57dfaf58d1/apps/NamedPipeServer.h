#pragma once
#include <windows.h>
#using <System.Core.dll>

#define PIPE_NAME "ICEClassifier"

/**
NamedPipeServer
Periodically on its own thread sends value to pipe
where value is set externally
*/
public ref class NamedPipeServer
{
private:
	array<System::Threading::Thread^>^ servers;
	static const int BUFFER_SIZE = 512;
	static const int PIPE_SLEEP_INTERVAL = 50;
	static const int MAX_PIPE_INSTANCES = 4;
	static const unsigned char ACKNOWLEDGE = 0xB0;
	void ServerThread();
	int value = 0;
	int mag = 0;
	bool active = false;	//to communicate status to child threads
	unsigned char ack = 0x00;
	unsigned char bitArray = 0x00;
	unsigned char prevBitArray = 0x00;
public:
	NamedPipeServer();
	virtual ~NamedPipeServer();

	// Start pipe server on new thread
	void start();
	// Closes pipe
	void stop();
	// Set gesture value to be written to pipe
	void sendValue(int value, int mag);
	// Send the most recent acknowledgement from the client
	unsigned char getAcknowledge();
	// Change individual bits in the array
	void changeBit(char, bool);
	// Returns the value of a bit
	bool checkBit(int);
	// Flush the byte
	void flushByte();
};

