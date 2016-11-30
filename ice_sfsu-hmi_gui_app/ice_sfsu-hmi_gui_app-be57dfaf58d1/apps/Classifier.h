#pragma once
#include "DataAgent.h"

/**
Classifier
Abstract class that classifies incoming data and outputs a special DataVector
with zero length data array and the decision stored in the flag member.
*/
class Classifier :
	public DataAgent
{
protected:
	int latestPrediction;
public:
	Classifier();
	virtual ~Classifier();

	//creates svm model based on given data
	virtual void train(const std::vector<DataVector> & data) = 0;
	//Returns 1 if model is ready for classification, otherwise returns 0
	virtual bool modelReady() const = 0;
	//returns predicted class of given DataVector
	virtual int predict(const DataVector & dvec) const = 0;

	virtual int saveModel(std::string filename) const = 0;
	virtual void loadModel(std::string filename) = 0;

	//returns classification of last received DataVector
	int getLatestPrediction() const;
	//returns number of correct classifications (first) and total vectors tested (second)
	virtual std::pair<int, int> testAccuracy(const std::vector<DataVector> & data) const;
	
	//For the LDA use one MAV/MMAV less
	float * crossAccuracy(const std::vector<DataVector>& data, int nClass, int parts);
};

