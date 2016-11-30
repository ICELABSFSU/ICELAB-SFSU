#pragma once
#include <cmath>
#include "DataAgent.h"

#define	FtNm	5// 4 to 5
#define IMUFtNm 1
#define	numSensors	8
#define numDimensions 9


/**
FeatureCalculator
processes a set of incoming DataVectors and outputs a FeatureVector
incoming DataVectors are stored in a circular buffer whose parameters can be changed
*/
class FeatureCalculator :
	public DataAgent
{
private:
	int flag;
	int nFeatures = FtNm;
	int nIMUFeatures = IMUFtNm;
	int nSensors = numSensors;
	int nDimensions = numDimensions;
	bool orientationOn;
	bool gyroOn;
	bool accelOn;
	bool checkEMGGrid[FtNm][numSensors];
	DataVector* samplebuf;
	DataVector* imusamplebuf;
	int ibuf = 0;
	int imuibuf = 0;
	int bufsize = 128;

	DataAgent* SwingFinder;
	DataAgent* MMAVPkFinder;
	DataAgent* SSorTrans;

	int winsize = 40;	//window size
	int winincr = 8;	//separation length between windows
	int winnext = winsize + 1;	//winsize + 2 samples until first feature

	int lastCall;
	int firstCall;

	float** FeatEMG;
	float** FeatIMU;

	void featCalc();
	void featCalcIMU();
	//sets flag from majority in current window
	void findMajorityFlag();
	DataVector buildDataVector();
	DataVector buildSFDataVector();
	DataVector buildMMAVVector();
	DataVector buildSSorTransVector();

	bool locked = false;	//mutex lock for samplebuf and FeatEMG
	void lock();
	void unlock();
	void waitForLock() const;

public:
	FeatureCalculator();
	virtual ~FeatureCalculator();

	// Inherited via DataAgent
	virtual void acceptData(const DataVector & dvec) override;

	// Resets data buffer
	void reset();

	// Returns current feature on specified sensor
	// and returns zero if either index is invalid
	float getFeature(int featureIndex, int sensor) const;
	// Returns current average of feature across all sensors
	// and returns zero if index is invalid
	float getFeatureAverage(int featureIndex) const;

	void addFeatToChannel(const int& ft, const int& ch);
	void remFeatFromChannel(const int& ft, const int& ch);
	void setOrienOnOff(bool);
	bool getOrienOnOff() const;
	void setGyroOnOff(bool);
	bool getGyroOnOff() const;
	void setAccelOnOff(bool);
	bool getAccelOnOff() const;

	void addSwingFinder(DataAgent& newConsumer);
	void remSwingFinder();
	void addMMAVPkFinder(DataAgent& newConsumer);
	void remMMAVPkFinder();
	void addSSorTrans(DataAgent& newConsumer);
	void remSSorTrans();

	void notifySwingFinder(const DataVector& dvec) const;
	void notifyMMAVPkFinder(const DataVector& dvec) const;
	void notifySSorTrans(const DataVector& dvec) const;

	int getWindowSize() const;
	// Resize sampling window, resets data buffer
	void setWindowSize(int newWinsize);
	int getWindowIncrement() const;
	void setWindowIncrement(int newWinincr);

	// Returns set of feature windows from given data set
	std::vector<DataVector> calculateFeatures(const std::vector<DataVector> & data, int winsize, int winincr);
	// Returns set of feature windows from given data set
	// Reads the file as a concatenated set of individual class data, not as a time-continuous data set
	std::vector<DataVector> calculateFeaturesNoMix(const std::vector<DataVector> & data, int winsize, int winincr);
};

class SwingFinder : public DataAgent {
private:
	float* SFBuf;
	int bufsize = 128;
	int ibuf;
	int elementsInbuf;
	int valuesInAverage;
	short prev; // -1 0 1
	short curr; // -1 0 1
	float sum;
	float average;
	float sfthresh;
	void calcAverage();
public:
	SwingFinder();
	virtual ~SwingFinder();
	virtual void acceptData(const DataVector & dvec) override;
	float getOrPitch() const;
	float getAverage() const;
	void setThresh(const float& newThresh);
	void setValuesInAvg(const int& newValuesInAvg);
	bool hasSwOccured() const;
};

// JTY In Progress
class MMAVPkFinder : public DataAgent {
private:
	float* MMAVPkBuf;
	bool prevAboveThresh;
	bool currAboveThresh;
	int bufsize = 128;
	int ibuf;
	float MMAVThresh = 13;
public:
	MMAVPkFinder();
	virtual ~MMAVPkFinder();
	virtual void acceptData(const DataVector & dvec) override;
	void setThresh(const float& newThresh);
	bool hasPkOccured() const;
};

class SSorTransition : public DataAgent {
private:
	DataVector* mavBuffer; // Buffer for mav of 8 channels
	float STD; // STD across 8 channels
	int bufsize = 128;
	int ibuf;
	int first;
	int last;
	int winsize = 5;
	int winincr = 1;
	int winnext = winsize - 1;
	float SSTThresh = 21;
	bool wait;
	bool prevAboveThresh;
	bool currAboveThresh;
	float stDeviation();
	void reset();

	bool locked = false;
	void lock();
	void unlock();
	void waitForLock() const;
public:
	SSorTransition();
	virtual ~SSorTransition();
	virtual void acceptData(const DataVector & dvec) override;
	float getWindowedSTD() const;
	float getThresh() const;
	void setWait(bool &newWait);
	void setThresh(const float& newThresh);
	void setWinsize(const int& newWinsize);
	bool hasEnteredStedayState();
};