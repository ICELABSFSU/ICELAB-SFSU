#pragma once
#include "Classifier.h"
#include "EMG_PR.h"

class LDA :
	public Classifier
{
private:
	int featureDim = 1;
	int nClasses;

	float *Wg;
	float *Cg;
	float *xmean;
	float *xstd;
	bool ready = false;
public:
	LDA();
	virtual ~LDA();

	// Inherited via Classifier
	virtual void acceptData(const DataVector & dvec) override;
	//creates svm model based on given data
	//automatically trims all class datasets to the shortest class dataset
	virtual void train(const std::vector<DataVector>& data) override;
	virtual bool modelReady() const override;
	virtual int predict(const DataVector & dvec) const override;
	virtual int saveModel(std::string filename) const override;
	virtual void loadModel(std::string filename) override;

	static size_t parseFloat(const std::string& str, std::vector<float>& list);
};
