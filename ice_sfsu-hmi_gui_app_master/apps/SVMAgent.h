#pragma once
#include "Classifier.h"
#include "svm.h"

/*
SVMAgent
accepts DataVectors and classifies data based on svm model
model is created by passing training data to train
*/
class SVMAgent :
	public Classifier
{
private:
	svm_problem prob;
	svm_parameter param;
	svm_model* model;
public:
	SVMAgent();
	virtual ~SVMAgent();

	// Inherited via DataAgent
	virtual void acceptData(const DataVector & dvec) override;

	//creates svm model based on given data
	void train(const std::vector<DataVector> & data);
	//Returns 1 if model is ready for classification, otherwise returns 0
	bool modelReady() const;
	//returns number of correct classifications (first) and total vectors tested (second)
	std::pair<int, int> testAccuracy(const std::vector<DataVector>& data) const;

	//returns predicted class of given DataVector
	int predict(const DataVector & dvec) const;

	int saveModel(std::string filename) const;
	void loadModel(std::string filename);

	//svm parameters

	void setType(int svm_type);
	void setKernelLinear();
	void setKernelPoly(int degree, double gamma, double coef0);
	void setKernelRBF(double gamma);
	void setKernelSigmoid(double gamma, double coef0);

	static svm_node* convertDataVector(const DataVector & dvec);
};

