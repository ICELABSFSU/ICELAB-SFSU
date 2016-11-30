#pragma once
#include "DataAgent.h"
#include <vector>
#include <map>

/**
ClassifyTrainer
when active, stores incoming windows (a vector of features) to buffer
passes data to classifier in train()
*/
class ClassifyTrainer :
	public DataAgent
{
private:
	//size of databuf should always be equal to size of classNames

	//vector of class data
	std::map<int, std::vector<DataVector>> databuf;
	//(bimap) map key index, value name
	std::map<int, std::string> classNames;
	std::map<int, std::string> classPicturePath; //for picture file paths
	//(bimap) map key name, value index
	std::map<std::string, int> classIndex;

	int trainingActive = 0;
	int sampleCount = 0;
	int sampleSize = 0;
public:
	ClassifyTrainer();
	virtual ~ClassifyTrainer();

	// Inherited via DataAgent
	virtual void acceptData(const DataVector & dvec) override;

	//add another dataset into current dataset
	void addDataset(const std::vector<DataVector> & data);
	//start collecting data, adds to current dataset
	//continues collection until stopCollect is called
	void startCollect();
	//start data collection, adds to current dataset
	//automatically stops after collecting sampleSize samples
	void startCollect(int sampleSize);
	//stops data collection
	void stopCollect();
	//returns zero if collection is inactive, non-zero if collecting
	int isActive() const;
	//returns number of samples collected in the current collection set
	int getSampleCount() const;
	//returns number of samples for given class
	int getClassSampleCount(int classIndex) const;

	//add new class name
	//returns the assigned index of new class
	int addClassName(std::string& name);
	//rename a class
	void renameClass(int index, std::string& newName);
	// JTY Add picture filepath to classPicturePath
	void selectPicture(int index, std::string& path);
	//delete a class
	void removeClass(int index);
	//delete a class's sampling data
	void deleteClassData(int classIndex);
	//returns class name given its index
	const std::string getClassName(int index) const;
	/// JTY returns System::String^ filepath of classification[index]
	System::String^ getClassPicturePath(int index) const;
	//returns class index given its name
	int getClassIndex(std::string& name) const;
	//returns number of classes in class name list
	int getNumberOfClasses() const;
	std::vector<std::pair<int, std::string>> getAllClasses() const;

	const std::vector<DataVector> data() const;
	
	//delete all data
	void clearData();
	//delete all data and classes
	void clearAll();

	//void train(LDA& lda);
};

