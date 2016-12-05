#include "DataBuffer.h"



DataBuffer::DataBuffer()
{
}


DataBuffer::~DataBuffer()
{
}

void DataBuffer::acceptData(const DataVector & dvec)
{
	databuf.push_back(DataVector(dvec));
}

void DataBuffer::addDataset(const std::vector<DataVector>& data)
{
	for (DataVector dvec : data)
	{
		databuf.push_back(DataVector(dvec));
	}
}

void DataBuffer::clear()
{
	databuf.clear();
}

void DataBuffer::clearClassData(int flag)
{
	std::vector<DataVector>::iterator it = databuf.begin();
	while (it != databuf.end()) {
		if (it->flag == flag)
		{
			it = databuf.erase(it);
		}
		else
		{
			it++;
		}
	}
}

std::vector<DataVector> DataBuffer::data() const
{
	return databuf;
}

void DataBuffer::printData(std::string filepath)
{
	DataRecorder dr;
	this->addConsumer(dr);

	dr.startWrite(filepath);
	for (int i = 0; i < databuf.size(); i++)
	{
		this->notifyAll(databuf[i]);
	}
	dr.stopWrite();
	this->removeConsumer(dr);
}
