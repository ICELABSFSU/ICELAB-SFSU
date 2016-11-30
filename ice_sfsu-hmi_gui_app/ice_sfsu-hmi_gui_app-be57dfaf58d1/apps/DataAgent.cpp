#include "DataAgent.h"
#include <algorithm>


DataVector::DataVector()
	: imuOrEmg(0), flag(0), length(0), data(nullptr), timestamp(0)
{
}

DataVector::DataVector(int flag, int length)
	: imuOrEmg(0), flag(flag), length(length), data(nullptr), timestamp(0)
{
	if (length > 0)
	{
		this->data = new float[length];
		for (int i = 0; i < length; i++)
		{
			data[i] = 0.0;
		}
	}
}

DataVector::DataVector(int flag, int length, float * data)
	:imuOrEmg(0), flag(flag), length(length), data(nullptr), timestamp(0)
{
	if (length > 0) {
		this->data = new float[length];
		for (int i = 0; i < length; i++) {
			this->data[i] = data[i];
		}
	}
}

DataVector::DataVector(int flag, int length, float * data, uint64_t timestamp)
	:imuOrEmg(0), flag(flag), length(length), data(nullptr), timestamp(timestamp)
{
	if (length > 0) {
		this->data = new float[length];
		for (int i = 0; i < length; i++) {
			this->data[i] = data[i];
		}
	}
}

DataVector::DataVector(bool imuOrEmg, int flag, int length, float * data, uint64_t timestamp)
	:imuOrEmg(imuOrEmg), flag(flag), length(length), data(nullptr), timestamp(timestamp)
{
	if (length > 0) {
		this->data = new float[length];
		for (int i = 0; i < length; i++) {
			this->data[i] = data[i];
		}
	}
}

DataVector::DataVector(const DataVector & orig)
	:imuOrEmg(orig.imuOrEmg), flag(orig.flag), length(orig.length), data(nullptr), timestamp(orig.timestamp)
{
	data = new float[length];
	for (int i = 0; i < length; i++) {
		data[i] = orig.data[i];
	}
}

DataVector & DataVector::operator=(const DataVector & rhs)
{
	imuOrEmg = rhs.imuOrEmg;
	flag = rhs.flag;
	length = rhs.length;
	timestamp = rhs.timestamp;
	if (data != nullptr) delete[] data;
	data = new float[length];
	for (int i = 0; i < length; i++) {
		data[i] = rhs.data[i];
	}
	return *this;
}

DataVector::~DataVector()
{
	delete[] data;
}

std::ostream& operator<<(std::ostream& os, const DataVector& dvec) {
	// JTY- Will only output EMG. Later, sort IMU to different txt file
	if (dvec.imuOrEmg == 0) {
		os << dvec.flag << '\t';
		for (int i = 0; i < dvec.length; i++) {
			os << dvec.data[i] << '\t';
		}
		os << dvec.timestamp;
		os << std::endl;
		return os;
	}
}





DataAgent::DataAgent()
{
}

DataAgent::DataAgent(const DataAgent & orig)
	: consumers(orig.consumers)
{
}

DataAgent::~DataAgent()
{
}

void DataAgent::addConsumer(DataAgent & newConsumer)
{
	if (std::find(consumers.begin(), consumers.end(), &newConsumer) == consumers.end())
	{
		consumers.push_back(&newConsumer);
	}
}

void DataAgent::removeConsumer(DataAgent & consumer)
{
	std::vector<DataAgent*>::iterator it = std::remove(consumers.begin(), consumers.end(), &consumer);
	if (it != consumers.end())
	{
		consumers.erase(it);
	}
}

void DataAgent::notifyAll(const DataVector & dvec) const
{
	if (muted)
	{
		//do nothing
	}
	else
	{
		const std::vector<DataAgent*> consumers = this->consumers;
		for (DataAgent* dp : consumers)
		{
			dp->acceptData(dvec);
		}
	}
}

void DataAgent::mute()
{
	muted = true;
}

void DataAgent::unmute()
{
	muted = false;
}
