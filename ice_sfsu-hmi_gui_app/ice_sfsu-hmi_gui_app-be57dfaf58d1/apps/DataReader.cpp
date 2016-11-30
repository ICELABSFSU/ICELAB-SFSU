#include "DataReader.h"

#define CHAR_BUF_SIZE	256



DataReader::DataReader()
{
}

DataReader::~DataReader()
{
}

void DataReader::acceptData(const DataVector & dvec)
{
	//do nothing
}

size_t DataReader::load(std::string filepath, const int dataLength)
{
	datasetIndex = 0;
	dataset = readFile(filepath, dataLength);
	return dataset.size();
}

bool DataReader::sendNextData()
{
	bool moreData = true;
	notifyAll(dataset.at(datasetIndex));
	datasetIndex++;
	if (datasetIndex >= dataset.size()) {
		//end of dataset reached
		moreData = false;
	}
	return moreData;
}

size_t DataReader::seek(size_t index)
{
	if (index >= 0 && index < dataset.size()) datasetIndex = index;
	return datasetIndex;
}

DataVector DataReader::getData(size_t index) const
{
	return dataset.at(index);
}

size_t DataReader::size() const
{
	return dataset.size();
}

size_t DataReader::getCurrentIndex() const
{
	return datasetIndex;
}

void DataReader::sendAll()
{
	if (dataset.size() > 0)
	{
		seek(0);
		while (sendNextData());
	}
}

std::vector<DataVector> DataReader::readFile(std::string filepath, const int dataLength)
{
	std::ifstream ifile;
	std::vector<DataVector> dataset;
	ifile.open(filepath.c_str(), std::ios::ios_base::in);
	if (!ifile.is_open())	//failed
	{
		ifile.close();
		return dataset;
	}

	char cbuf[CHAR_BUF_SIZE];
	char* cbufp = nullptr;
	int offset;
	int flag = 0;
	float* data = new float[dataLength];
	uint64_t timestamp;

	while (ifile.getline(cbuf, CHAR_BUF_SIZE))
	{
		//each line
		cbufp = cbuf;
		offset = 0;
		//read flag
		sscanf(cbufp, " %d%n", &flag, &offset);
		cbufp += offset;
		//read data
		for (int i = 0; i < dataLength; i++)
		{
			if (sscanf(cbufp, " %f%n", &data[i], &offset) != 1)
			{
				//default value if value was not found
				data[i] = 0.0;
			}
			cbufp += offset;
		}
		if (sscanf(cbufp, " %llu", &timestamp) != 1)
		{
			timestamp = 0;
		}

		//push DataVector
		dataset.push_back(DataVector(flag, dataLength, data, timestamp));
	}

	delete[] data;
	ifile.close();
	return dataset;
}
