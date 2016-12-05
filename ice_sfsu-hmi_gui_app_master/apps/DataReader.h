#pragma once
#include "DataAgent.h"
#include <fstream>

/**
DataReader
reads and outputs DataVectors from file
*/
class DataReader :
	public DataAgent
{
private:
	std::vector<DataVector> dataset;
	size_t datasetIndex;
public:
	DataReader();
	virtual ~DataReader();

	// Inherited via DataAgent
	// Does nothing
	virtual void acceptData(const DataVector & dvec) override;

	//loads data from file for sending
	//returns number of DataVectors in loaded dataset
	size_t load(std::string filepath, const int dataLength);
	//sends next DataVector, returns false when dataset end is reached
	bool sendNextData();
	//set current DataVector to specified index, returns new index
	size_t seek(size_t index);
	DataVector getData(size_t index) const;
	size_t size() const;
	size_t getCurrentIndex() const;
	//sends all DataVectors to any consumers, blocks until finished
	void sendAll();

	//reads entire file and returns data a single set of DataVectors
	static std::vector<DataVector> readFile(std::string filepath, const int dataLength);
};

