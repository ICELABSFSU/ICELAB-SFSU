#include "DataRecorder.h"


DataRecorder::DataRecorder()
	: filepath(""), writing(0) 
{
}

DataRecorder::DataRecorder(std::string filepath)
	: filepath(filepath), writing(0)
{
	openFile(filepath);
}

DataRecorder::~DataRecorder()
{
	ofile.close();
}

void DataRecorder::acceptData(const DataVector & dvec)
{
	if(writing)
		// JTY << overload currently outputs only emg data to file
		// Later, sort IMU to different txt file
		ofile << dvec;
}

int DataRecorder::openFile(std::string filepath) {
	ofile.open(filepath.c_str(), std::ios::ios_base::out | std::ios::ios_base::trunc);
	if (!ofile.is_open()) {
		writing = 0;
	}
	else {
		writing = 1;
	}
	return writing;
}

void DataRecorder::startWrite() {
	writing = 1;
}

void DataRecorder::startWrite(std::string filepath) {
	openFile(filepath);
}

void DataRecorder::stopWrite() {
	writing = 0;
	ofile.flush();
	ofile.close();
	filepath = "";
}


AccuracyRecorder::AccuracyRecorder()
	: DataRecorder()
{
}

AccuracyRecorder::AccuracyRecorder(std::string filepath)
	: DataRecorder(filepath)
{
}

AccuracyRecorder::~AccuracyRecorder()
{
}

void AccuracyRecorder::acceptData(const DataVector & dvec)
{
	if (writing)
	{
		//should data and decision in pairs, order not known
		if (dvec.length == 0)	//decision from classifier, no data
		{
			this->decision = dvec;
		}
		else
		{
			//save dvec data
			this->data = dvec;
		}

		if (decision.timestamp != this->data.timestamp)
		{
			//ofile << "timestamp mismatch" << std::endl;
		}
		else
		{
			//print flag, decision, and data
			ofile << decision.flag << '\t';
			ofile << this->data;
		}
	}
}
