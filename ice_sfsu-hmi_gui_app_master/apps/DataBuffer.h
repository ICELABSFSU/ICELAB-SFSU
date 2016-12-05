#pragma once
#include "DataAgent.h"
#include "DataRecorder.h"
#include <vector>
#include <fstream>

/**
DataBuffer
Buffer for data holding and printing to file
*/
class DataBuffer :
	public DataAgent
{
private:
	std::vector<DataVector> databuf;
public:
	DataBuffer();
	virtual ~DataBuffer();

	// Inherited via DataRecorder
	virtual void acceptData(const DataVector & dvec) override;

	// Add another dataset into current dataset
	void addDataset(const std::vector<DataVector> & data);
	// Delete data
	void clear();
	// Remove all data with given flag
	void clearClassData(int flag);
	// Return stored data
	std::vector<DataVector> data() const;
	void printData(std::string filepath);
};

