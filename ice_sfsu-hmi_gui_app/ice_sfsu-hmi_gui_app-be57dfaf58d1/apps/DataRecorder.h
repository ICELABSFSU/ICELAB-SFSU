#pragma once
#include "DataAgent.h"
#include <fstream>

/**
Data Recorder
writes DataVector to file when attached as a consumer to
another DataAgent.
*/
class DataRecorder :
	public DataAgent
{
protected:
	std::string filepath;
	std::ofstream ofile;
	int writing;
public:
	DataRecorder();
	DataRecorder(std::string filepath);
	virtual ~DataRecorder();

	// Inherited via DataAgent
	virtual void acceptData(const DataVector & dvec) override;

	//opens file for writing
	int openFile(std::string filepath);

	//writes incoming DataVector to file
	void startWrite();
	//opens file for writing and writes incoming DataVector to file
	void startWrite(std::string filepath);
	//halts write and closes file
	void stopWrite();
};


/**
AccuracyRecorder
Accepts data from a normal DataAgent and a 'zero-data' DataAgent
such as a classifier that will only output a decision flag.
Writes normal DataAgent DataVector with a decision flag inserted
in the second column.
*/
class AccuracyRecorder :
	public DataRecorder
{
private:
	DataVector decision;
	DataVector data;
public:
	AccuracyRecorder();
	AccuracyRecorder(std::string filepath);
	virtual ~AccuracyRecorder();

	// Inherited via DataRecorder
	virtual void acceptData(const DataVector & dvec) override;
};
