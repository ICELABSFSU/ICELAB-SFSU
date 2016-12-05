#pragma once

#include <ostream>
#include <vector>

/**
DataVector
Container for associated data
*/
struct DataVector {
	bool imuOrEmg; // 1 IMU, 0 EMG
	int flag;
	int length; //number of samples
	float* data;
	uint64_t timestamp;	//number of microseconds since some event

	DataVector();
	DataVector(int flag, int length);
	DataVector(int flag, int length, float* data);
	DataVector(int flag, int length, float* data, uint64_t timestamp);
	DataVector(bool imuOrEmg, int flag, int length, float* data, uint64_t timestamp);
	DataVector(const DataVector& orig);
	DataVector& operator=(const DataVector& rhs);
	virtual ~DataVector();
	friend std::ostream& operator<<(std::ostream& os, const DataVector& dvec);
};

/**
DataAgent
Abstract class that operates on incoming data and outputs a DataVector
DataAgents can both generate events for and receive events from other DataAgents
*/
class DataAgent {
private:
	std::vector<DataAgent*> consumers;	//receives this object's data
	bool muted = false;
public:
	DataAgent();
	//Copy constructor
	//Constructed object will have identical consumers to original
	DataAgent(const DataAgent& orig);
	virtual ~DataAgent();

	//might revise to implement generic events and observers

	void addConsumer(DataAgent& newConsumer);
	void removeConsumer(DataAgent& newConsumer);
	//accept data from sender
	virtual void acceptData(const DataVector& dvec) = 0;
	//push data to consumer
	void notifyAll(const DataVector& dvec) const;
	//Disable output from this DataAgent, silences notifyAll
	void mute();
	//Enable output from this DataAgent
	void unmute();
};

