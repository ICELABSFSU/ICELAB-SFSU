#include "NamedPipeServer.h"

void NamedPipeServer::ServerThread()
{
	array<unsigned char>^ response = gcnew array<unsigned char>(80);
	System::IO::Pipes::NamedPipeServerStream^ pipeServer;

	//server connection loop
	do
	{
		pipeServer = gcnew System::IO::Pipes::NamedPipeServerStream(
			PIPE_NAME,
			System::IO::Pipes::PipeDirection::InOut,
			MAX_PIPE_INSTANCES);
		//wait for connection
		pipeServer->WaitForConnection();
		while (active)
		{
			//send-receive loop
			try
			{
				array<unsigned char>^ pack = { (unsigned char)value , (unsigned char)mag , };
				pipeServer->Write(pack, 0, pack->Length);
				pipeServer->Read(response, 0, 80);

				//break if error
				if ((response[0] & 0xF0) != ACKNOWLEDGE)
				{
					//lost connection
					break;
				}
				//checkAcknowledge(response[0]);
				ack = response[0];

				System::Threading::Thread::Sleep(PIPE_SLEEP_INTERVAL);
			}
			catch (...)
			{
				break;
			}
			ack = response[0];
		}
		ack = response[0];
		//reaches here if there was an error or pipe was closed
		pipeServer->Close();
	} while (active); //don't try to connect again if deactivated
}

NamedPipeServer::NamedPipeServer()
{
	servers = gcnew array<System::Threading::Thread^>(MAX_PIPE_INSTANCES);
}

NamedPipeServer::~NamedPipeServer()
{
	stop();
}

void NamedPipeServer::start()
{
	active = true;
	for (int i = 0; i < servers->Length; i++)
	{
		servers[i] = gcnew System::Threading::Thread(
			gcnew System::Threading::ThreadStart(
				this, &NamedPipeServer::ServerThread));
		servers[i]->Start();
	}
}

void NamedPipeServer::stop()
{
	active = false;
	System::IO::Pipes::NamedPipeClientStream^ client;
	for (int i = 0; i < servers->Length; i++)
	{
		//force server thread to return from WaitForConnection
		client = gcnew System::IO::Pipes::NamedPipeClientStream(PIPE_NAME);
		try
		{
			client->Connect(50);
		}
		catch (System::TimeoutException^)
		{
		}
		catch (System::IO::IOException^)
		{
		}
		client->Close();
	}
	for (int i = 0; i < servers->Length; i++)
	{
		servers[i]->Join();
	}
}

void NamedPipeServer::sendValue(int value, int mag)
{
	this->value = value;
	this->mag = mag;
}

unsigned char NamedPipeServer::getAcknowledge()
{
	return ack;
}

void NamedPipeServer::changeBit(char bit, bool state) {
	if (state)
		bitArray |= 1 << bit;
	else
		bitArray &= ~(1 << bit);
}

bool NamedPipeServer::checkBit(int bit) {
	return (bitArray >> bit) & 1;
}

void NamedPipeServer::flushByte() {
	prevBitArray = bitArray;
	bitArray = 0x00;
}
















