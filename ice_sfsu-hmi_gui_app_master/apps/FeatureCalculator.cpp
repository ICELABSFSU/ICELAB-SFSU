#include "FeatureCalculator.h"
#include <cmath>
#include <map>


#define Sigma 5
#define threshold Sigma


void FeatureCalculator::featCalc()
{
	char signLast;
	char slopLast;
	int j, k;
	double Delta_2;
	float sMAV[8] = {0};
	float MMAV = 0;

	lock();
	//for each sensor calculate features
	for (int sensor(0); sensor < nSensors; sensor++)
	{
		for (int f = 0; f < nFeatures; f++) { FeatEMG[f][sensor] = 0; }

		k = (firstCall + bufsize - 1) % bufsize;;	//one before window start   // (41 - 40 + 1 = 2) - 1
		j = (k + bufsize - 1) % bufsize;	//        two before ws(firstCall)  // 0

		signLast = 0;
		slopLast = 0;

		Delta_2 = samplebuf[k].data[sensor] - samplebuf[j].data[sensor];

		if (Delta_2 > threshold) { slopLast += 4; }
		if (Delta_2 < -threshold) { slopLast += 8; }

		if (samplebuf[j].data[sensor] > threshold) { signLast = 4; }
		if (samplebuf[j].data[sensor] < -threshold) { signLast = 8; }

		for (int i(0); i < (winsize); i++) //-2
		{
			j = k;                 //prev     //1 - 40
			k = (j + 1) % bufsize; //current  //2 - 41

			Delta_2 = samplebuf[k].data[sensor] - samplebuf[j].data[sensor];

			if (samplebuf[k].data[sensor] > threshold) { signLast += 1; }
			if (samplebuf[k].data[sensor] < -threshold) { signLast += 2; }

			if (Delta_2 > threshold) { slopLast += 1; }
			if (Delta_2 < -threshold) { slopLast += 2; }

			if (signLast == 9 || signLast == 6) { FeatEMG[2][sensor]++; }
			if (slopLast == 9 || slopLast == 6) { FeatEMG[3][sensor]++; }

			signLast = (signLast << 2) & 15;
			slopLast = (slopLast << 2) & 15;

			//FeatEMG[4][sensor] += pow(samplebuf[k].data[sensor],2);
			FeatEMG[0][sensor] += abs(samplebuf[k].data[sensor]);
			FeatEMG[1][sensor] += (float)abs(Delta_2);

		}

		
		FeatEMG[0][sensor] = FeatEMG[0][sensor] / winsize;//MAV of One sensor
		FeatEMG[1][sensor] = FeatEMG[1][sensor] / winsize;
		FeatEMG[2][sensor] = FeatEMG[2][sensor] * 100 / winsize;
		FeatEMG[3][sensor] = FeatEMG[3][sensor] * 100 / winsize;
	
		sMAV[sensor] = FeatEMG[0][sensor];
		MMAV += FeatEMG[0][sensor];

		if (sensor == (nSensors-1)) {
			for (int l = 0; l < nSensors; l++) {
				FeatEMG[4][l] = (sMAV[l] / (MMAV/8)) * 25; //Scaling otherwise the value is so small
														   //it does not display on the EMG graph
			}
		}

		//FeatEMG[4][sensor] = FeatEMG[4][sensor] / winsize;
		//FeatEMG[4][sensor] = pow(FeatEMG[4][sensor], 0.5);
		
	}

	unlock();
}

// Feature calculator for IMU, gives average  of last k-values for roll, pitch, yaw
// where k = (size of EMG buffer window) / 4
// (1 piece of IMU data for every 4 pieces of EMG data)
void FeatureCalculator::featCalcIMU()
{
	lock();
	int i = 0;
	float sum;
	for (int ft = 0; ft < nIMUFeatures; ft++) {
		for (int d = 0; d < nDimensions; d++) {
			i = (imuibuf + bufsize - (winsize / 4)) % bufsize;
			sum = 0;
			while (i != imuibuf) {
				sum += imusamplebuf[i].data[d];
				i = (i + bufsize + 1) % bufsize;
			}
			FeatIMU[ft][d] = sum / (winsize / 4);
		}
	}
	unlock();
}

void FeatureCalculator::findMajorityFlag()
{
	std::map<int, int> freq;

	lock();
	//count flag freq
	int index = firstCall;
	for (int i = 0; i < winsize; i++)
	{
		index = index % bufsize;
		freq[samplebuf[index].flag] += 1;
		index++;
	}
	unlock();

	//find most freq
	//ties results in the lower integer flag
	std::map<int, int>::iterator maxIt = freq.begin();
	for (std::map<int, int>::iterator it = ++freq.begin(); it != freq.end(); it++)
	{
		//if max count is less than current count
		if (maxIt->second < it->second)
		{
			maxIt = it;
		}
	}

	//set flag
	flag = maxIt->first;
}

DataVector FeatureCalculator::buildDataVector()
{
	// Count total EMG features to send
	int emgct = 0;
	for (int i = 0; i < FtNm; i++)
	{
		for (int j = 0; j < numSensors; j++)
		{
			if (checkEMGGrid[i][j] == true) emgct++;
		}
	}

	// Count total IMU features to send
	int imuct = 0;
	if (orientationOn) imuct += 3;
	if (gyroOn) imuct += 3;
	if (accelOn) imuct += 3;

	float* temp = new float[emgct + imuct]();
	lock();
	int n = 0;

	for (int i = 0; i < nFeatures; i++)
	{
		//group features per sensor
		for (int j = 0; j < nSensors; j++)
		{
			if (checkEMGGrid[i][j] == true)
			{
				temp[n] = FeatEMG[i][j];
				n++;
			}
		}
	}

	for (int k = 0; k < nIMUFeatures; k++)
	{
		if (orientationOn) {
			for (int or = 0; or < 3; or ++)
			{
				temp[n] = FeatIMU[k][or];
				n++;
			}
		}
		if (gyroOn) {
			for (int gy = 3; gy < 6; gy++)
			{
				temp[n] = FeatIMU[k][gy];
				n++;
			}
		}
		if (accelOn) {
			for (int acc = 6; acc < 9; acc++)
			{
				temp[n] = FeatIMU[k][acc];
				n++;
			}
		}
	}
	unlock();
	DataVector dvec(flag, (emgct + imuct), temp);
	delete[] temp;
	return dvec;
}

DataVector FeatureCalculator::buildSFDataVector()
{
	float* temp = &FeatIMU[0][1];
	DataVector SFVec(flag, 1, temp);
	SFVec.imuOrEmg = 1;
	return SFVec;
}

DataVector FeatureCalculator::buildMMAVVector()
{
	float MMAV = getFeatureAverage(0);
	float* temp = &MMAV;
	DataVector SFVec(flag, 1, temp);
	SFVec.imuOrEmg = 1;
	return SFVec;
}

DataVector FeatureCalculator::buildSSorTransVector()
{
	float* temp = new float[nSensors];
	lock();
	for (int ch = 0; ch < nSensors; ch++) {
		temp[ch] = FeatEMG[4][ch];
	}
	unlock();
	DataVector SSTVec(flag, nSensors, temp);
	SSTVec.imuOrEmg = 0;
	delete[] temp;
	return SSTVec;
}

void FeatureCalculator::lock()
{
	waitForLock();
	locked = true;
}

void FeatureCalculator::unlock()
{
	locked = false;
}

void FeatureCalculator::waitForLock() const
{
	while (locked);
}

FeatureCalculator::FeatureCalculator()
	: samplebuf(nullptr), imusamplebuf(nullptr), SwingFinder(nullptr), MMAVPkFinder(nullptr), SSorTrans(nullptr)
{
	orientationOn = gyroOn = accelOn = false;
	samplebuf = new DataVector[bufsize];
	imusamplebuf = new DataVector[bufsize];
	FeatEMG = new float*[nFeatures];
	FeatIMU = new float*[nFeatures];
	for (int ft = 0; ft < nFeatures; ft++)
	{
		for (int sen = 0; sen < nSensors; sen++)
		{
			checkEMGGrid[ft][sen] = true;
		}
	}
	for (int i = 0; i < nFeatures; i++)
	{
		FeatEMG[i] = new float[nSensors]();
	}
	for (int j = 0; j < nIMUFeatures; j++)
	{
		FeatIMU[j] = new float[nDimensions]();
	}
}

FeatureCalculator::~FeatureCalculator()
{
	lock();
	delete[] samplebuf;
	delete[] imusamplebuf;
	for (int i = 0; i < nFeatures; i++)
	{
		delete[] FeatEMG[i];
	}
	for (int j = 0; j < nIMUFeatures; j++)
	{
		delete[] FeatIMU[j];
	}
	delete[] FeatEMG;
	delete[] FeatIMU;
}

void FeatureCalculator::acceptData(const DataVector & dvec)
{
	//push new dvec into circular buffer
	//if !imuOrEmg, dvec is EMG data, put into samplebuf
	//else dvec is IMU data, put into imusamplebuf
	if (!(dvec.imuOrEmg))
	{
		lock();
		samplebuf[ibuf] = dvec;
		unlock();

		//process window if next window is reached
		if (ibuf == winnext)
		{
			lastCall = winnext; //current ibuf 
			firstCall = (lastCall - winsize + bufsize + 1) % bufsize;

			findMajorityFlag();
			featCalc();
			featCalcIMU();
			DataVector features(buildDataVector());
			//copy timestamp of this last DataVector in the window
			features.timestamp = dvec.timestamp;
			notifyAll(features);
			if (SwingFinder != nullptr) {
				DataVector SFVec(buildSFDataVector());
				SFVec.timestamp = dvec.timestamp;
				notifySwingFinder(SFVec);
			}
			if (MMAVPkFinder != nullptr) {
				DataVector MMAVVec(buildMMAVVector());
				MMAVVec.timestamp = dvec.timestamp;
				notifyMMAVPkFinder(MMAVVec);
			}
			if (SSorTrans != nullptr) {
				DataVector SSTVec(buildSSorTransVector());
				SSTVec.timestamp = dvec.timestamp;
				notifySSorTrans(SSTVec);
			}
			winnext = (winnext + winincr) % bufsize;
		}
		ibuf = ++ibuf % (bufsize); //-1
	}
	else
	{
		imuibuf = ++imuibuf % (bufsize);
		lock();
		imusamplebuf[imuibuf] = dvec;
		unlock();
	}
}

void FeatureCalculator::reset()
{
	ibuf = 0;
	imuibuf = 0;
	winnext = winsize + 1;
}

float FeatureCalculator::getFeature(int featureIndex, int sensor) const
{
	if (featureIndex >= nFeatures || featureIndex < 0) return 0.0;
	if (sensor >= nSensors || sensor < 0) return 0.0;
	waitForLock();
	return FeatEMG[featureIndex][sensor];
}

float FeatureCalculator::getFeatureAverage(int featureIndex) const
{
	if (featureIndex >= nFeatures || featureIndex < 0) return 0.0;
	float sum = 0.0;
	waitForLock();
	for (int sensor = 0; sensor < nSensors; sensor++)
	{
		sum += FeatEMG[featureIndex][sensor];
	}
	return sum / nSensors;
}

void FeatureCalculator::addFeatToChannel(const int & ft, const int & ch)
{
	if (checkEMGGrid[ft][ch] != true) {
		checkEMGGrid[ft][ch] = true;
	}
}

void FeatureCalculator::remFeatFromChannel(const int & ft, const int & ch)
{
	if (checkEMGGrid[ft][ch] == true) {
		checkEMGGrid[ft][ch] = false;
	}
}

void FeatureCalculator::setOrienOnOff(bool onoff)
{
	if (orientationOn != onoff) {
		orientationOn = onoff;
	}
}

bool FeatureCalculator::getOrienOnOff() const
{
	return orientationOn;
}

void FeatureCalculator::setGyroOnOff(bool onoff)
{
	if (gyroOn != onoff) {
		gyroOn = onoff;
	}
}

bool FeatureCalculator::getGyroOnOff() const
{
	return gyroOn;
}

void FeatureCalculator::setAccelOnOff(bool onoff)
{
	if (accelOn != onoff) {
		accelOn = onoff;
	}
}

bool FeatureCalculator::getAccelOnOff() const
{
	return accelOn;
}

void FeatureCalculator::addSwingFinder(DataAgent & newConsumer)
{
	SwingFinder = &newConsumer;
}

void FeatureCalculator::remSwingFinder()
{
	SwingFinder = nullptr;
}

void FeatureCalculator::addMMAVPkFinder(DataAgent & newConsumer)
{
	MMAVPkFinder = &newConsumer;
}

void FeatureCalculator::remMMAVPkFinder()
{
	MMAVPkFinder = nullptr;
}

void FeatureCalculator::addSSorTrans(DataAgent & newConsumer)
{
	SSorTrans = &newConsumer;
}

void FeatureCalculator::remSSorTrans()
{
	SSorTrans = nullptr;
}

void FeatureCalculator::notifySwingFinder(const DataVector & dvec) const
{
	SwingFinder->acceptData(dvec);
}

void FeatureCalculator::notifyMMAVPkFinder(const DataVector & dvec) const
{
	MMAVPkFinder->acceptData(dvec);
}

void FeatureCalculator::notifySSorTrans(const DataVector & dvec) const
{
	SSorTrans->acceptData(dvec);
}

int FeatureCalculator::getWindowSize() const
{
	return winsize;
}

void FeatureCalculator::setWindowSize(int newWinsize)
{
	winsize = newWinsize;
	//if winsize is larger than buffer
	//if winsize is considerably smaller than buffer
	if (winsize + 10 > bufsize)
	{
		bufsize = winsize + 10;
		lock();
		delete[] samplebuf;
		samplebuf = new DataVector[bufsize];
		unlock();
	}

	reset();
}

int FeatureCalculator::getWindowIncrement() const
{
	return winincr;
}

void FeatureCalculator::setWindowIncrement(int newWinincr)
{
	if (winincr + 10 > bufsize)
	{
		bufsize = winincr + 10;
		lock();
		delete[] samplebuf;
		samplebuf = new DataVector[bufsize];
		unlock();
	}

	winincr = newWinincr;
}

std::vector<DataVector> FeatureCalculator::calculateFeatures(const std::vector<DataVector> & data, int winsize, int winincr)
{
	std::vector<DataVector> windows;
	setWindowSize(winsize);
	setWindowIncrement(winincr);
	reset();

	//run through data
	for (size_t i = 0; i < data.size(); i++)
	{
		//push new dvec into circular buffer
		lock();
		samplebuf[ibuf] = data[i];
		unlock();

		//process window if next window is reached
		if (ibuf == winnext)
		{
			lastCall = winnext;
			firstCall = (lastCall - winsize + bufsize + 1) % bufsize;

			findMajorityFlag();
			featCalc();
			DataVector features(buildDataVector());
			//copy timestamp of this last DataVector in the window
			features.timestamp = data[i].timestamp;
			windows.push_back(features);

			winnext = (winnext + winincr) % bufsize;
		}

		ibuf = ++ibuf & (bufsize - 1);
	}

	return windows;
}

std::vector<DataVector> FeatureCalculator::calculateFeaturesNoMix(const std::vector<DataVector> & data, int winsize, int winincr)
{
	std::vector<DataVector> windows;
	setWindowSize(winsize);
	setWindowIncrement(winincr);
	reset();

	//initialize flag
	int currentClass;
	if (data.size())
	{
		currentClass = data[0].flag;
	}

	//run through data
	for (size_t i = 0; i < data.size(); i++)
	{
		//reset buffer if new class
		if (currentClass != data[i].flag)
		{
			reset();
			currentClass = data[i].flag;
		}

		//push new dvec into circular buffer
		lock();
		samplebuf[ibuf] = data[i];
		unlock();

		//process window if next window is reached
		if (ibuf == winnext)
		{
			lastCall = winnext;
			firstCall = (lastCall - winsize + bufsize + 1) % bufsize;

			findMajorityFlag();
			featCalc();
			DataVector features(buildDataVector());
			//copy timestamp of this last DataVector in the window
			features.timestamp = data[i].timestamp;
			windows.push_back(features);

			winnext = (winnext + winincr) % bufsize;
		}

		ibuf = ++ibuf & (bufsize - 1);
	}

	return windows;
}

void SwingFinder::calcAverage()
{
	if (elementsInbuf < valuesInAverage) {
		average = (sum / (float)elementsInbuf);
	}
	else {
		average = (sum / (float)valuesInAverage);
	}
}

SwingFinder::SwingFinder()
{
	SFBuf = new float[bufsize];
	ibuf = 0;
	elementsInbuf = 0;
	valuesInAverage = 20;
	prev = 0;
	curr = 0;
	sum = 0;
	average = 0;
	sfthresh = 5 / 170.0f;
}

SwingFinder::~SwingFinder()
{
	delete[] SFBuf;
}

void SwingFinder::acceptData(const DataVector & dvec)
{
	prev = curr;
	SFBuf[ibuf] = *(dvec.data);
	if (elementsInbuf < bufsize) elementsInbuf++;
	sum += *(dvec.data);
	if (elementsInbuf > valuesInAverage) {
		sum -= SFBuf[(ibuf + bufsize - valuesInAverage) % bufsize];
	}
	calcAverage();
	if (*(dvec.data) > (average + sfthresh)) {
		curr = 1;
	}
	else if (*(dvec.data) < (average - sfthresh)) {
		curr = -1;
	}
	else curr = 0;
	ibuf = ++ibuf % bufsize;
}

float SwingFinder::getOrPitch() const
{
	if (elementsInbuf > 0) {
		return SFBuf[(ibuf + bufsize - 1) % bufsize];
	}
	else return 0.0f;
}

float SwingFinder::getAverage() const
{
	return average;
}

void SwingFinder::setThresh(const float & newThresh)
{
	sfthresh = newThresh;
}

void SwingFinder::setValuesInAvg(const int & newValuesInAvg)
{
	valuesInAverage = newValuesInAvg;
}

bool SwingFinder::hasSwOccured() const
{
	if ((prev == 1 && curr == -1) || (prev == -1 && curr == 1)) {
		return true;
	}
	else return false;
}

MMAVPkFinder::MMAVPkFinder()
{
	MMAVPkBuf = new float[bufsize];
	ibuf = 0;
	prevAboveThresh = false;
	currAboveThresh = false;
}

MMAVPkFinder::~MMAVPkFinder()
{
	delete[] MMAVPkBuf;
}

void MMAVPkFinder::acceptData(const DataVector & dvec)
{
	MMAVPkBuf[ibuf] = *(dvec.data);
	prevAboveThresh = currAboveThresh;
	if (*(dvec.data) > MMAVThresh) {
		currAboveThresh = true;
	}
	else currAboveThresh = false;
	ibuf = ++ibuf % bufsize;
}

void MMAVPkFinder::setThresh(const float& newThresh)
{
	MMAVThresh = newThresh;
}

bool MMAVPkFinder::hasPkOccured() const
{
	if (prevAboveThresh && !currAboveThresh) {
		return true;
	}
	else return false;
}

float SSorTransition::stDeviation()
{
	// For each channel calculate the window average
	float* average = new float[numSensors];
	for (int ch = 0; ch < numSensors; ch++) {
		average[ch] = 0;
		for (int i = 0; i < winsize; i++) {
			average[ch] += mavBuffer[(first + i) % bufsize].data[ch];
		}
		average[ch] /= winsize;
	}
	// For each channel, find squared distance from channel average. Add to sum.
	float sum = 0;
	for (int ch = 0; ch < numSensors; ch++) {
		for (int i = 0; i < winsize; i++) {
			sum += ((mavBuffer[(first + i) % bufsize].data[ch]) - (average[ch])) *
				((mavBuffer[(first + i) % bufsize].data[ch]) - (average[ch]));
		}
	}
	delete[] average;
	return std::sqrt((sum / winsize));
}

void SSorTransition::reset()
{
	ibuf = 0;
	first = 0;
}

void SSorTransition::lock()
{
	waitForLock();
	locked = true;
}

void SSorTransition::unlock()
{
	locked = false;
}

void SSorTransition::waitForLock() const
{
	while (locked);
}

SSorTransition::SSorTransition()
{
	ibuf = 0;
	wait = false;
	prevAboveThresh = false;
	currAboveThresh = false;
	mavBuffer = new DataVector[bufsize];
}

SSorTransition::~SSorTransition()
{
	delete[] mavBuffer;
}

void SSorTransition::acceptData(const DataVector & dvec)
{
	//push new dvec into circular buffer
	lock();
	mavBuffer[ibuf] = dvec;
	unlock();

	//process window if next window is reached
	if (ibuf == winnext)
	{
		first = (winnext - winsize + 1 + bufsize) % bufsize;
		prevAboveThresh = currAboveThresh;
		STD = stDeviation();
		if (STD > SSTThresh) {
			currAboveThresh = true;
		}
		else currAboveThresh = false;
		winnext = (winnext + winincr) % bufsize;
	}
	ibuf = ++ibuf % (bufsize); //-1
}

float SSorTransition::getWindowedSTD() const
{
	waitForLock();
	return STD;
}

float SSorTransition::getThresh() const
{
	return SSTThresh;
}

void SSorTransition::setWait(bool &newWait)
{
	wait = newWait;
}

void SSorTransition::setThresh(const float & newThresh)
{
	SSTThresh = newThresh;
}

void SSorTransition::setWinsize(const int & newWinsize)
{
	winsize = newWinsize;
	reset();
}

bool SSorTransition::hasEnteredStedayState()
{
	waitForLock();
	if (prevAboveThresh && !currAboveThresh && !wait) {
		return true;
	}
	else return false;
}