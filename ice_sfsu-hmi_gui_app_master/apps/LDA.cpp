#include "LDA.h"
#include <fstream>
#include <string>
#include <sstream>


LDA::LDA()
{
}

LDA::~LDA()
{
	free(Wg);
	free(Cg);
	free(xmean);
	free(xstd);
}

void LDA::acceptData(const DataVector & dvec)
{
	latestPrediction = predict(dvec);
	//build data vector, zero-length data array
	//only information is classification (flag)
	notifyAll(DataVector(latestPrediction, 0, 0, dvec.timestamp));
}

void LDA::train(const std::vector<DataVector>& data)
{
	int *TrainClass;
	float *Feature_matrix;
	int feature_matrix_idx;

	free(Wg);
	free(Cg);
	free(xmean);
	free(xstd);
	featureDim	= data[0].length;
	//organize feature data
	std::vector<std::vector<DataVector>> classData;
	//place dvec in the correct class set
	for (DataVector dvec : data)
	{
		//make space for new flag
		if (dvec.flag >= classData.size())
		{
			classData.push_back(std::vector<DataVector>());
		}
		classData[dvec.flag].push_back(dvec);	//push window into class
	}

	int winPerClass = classData[0].size();
	//find length of shortest vector
	for (size_t i = 1; i < classData.size(); i++)
	{
		if (classData[i].size() < winPerClass)
		{
			winPerClass = classData[i].size();
		}
	}
	nClasses = classData.size();

	TrainClass = (int*)malloc(nClasses*winPerClass*sizeof(int));     //class label for the training dataset
	Feature_matrix = (float*)malloc(nClasses*winPerClass*featureDim*sizeof(float));
	Wg = (float*)malloc(featureDim*nClasses*sizeof(float));
	Cg = (float*)malloc(1 * nClasses*sizeof(float));
	xmean = (float*)malloc(1 * featureDim*sizeof(float));
	xstd = (float*)malloc(1 * featureDim*sizeof(float));

	//convert to proper C arrays
	//copy Feature_matrix and TrainClass
	feature_matrix_idx = 0;
	for (std::vector<DataVector> cl : classData)
	{
		for (size_t i = 0; i < winPerClass; i++)
		{
			TrainClass[feature_matrix_idx] = cl[i].flag;
			for (int j = 0; j < cl[i].length; j++)
			{
				Feature_matrix[ feature_matrix_idx*cl[i].length + j ] = cl[i].data[j];
			}
			feature_matrix_idx++;
		}
	}

	feature_normalization(Feature_matrix, xmean, xstd, feature_matrix_idx, featureDim);
	LDA_train(Feature_matrix, TrainClass, Wg, Cg, featureDim, winPerClass, nClasses);

	free(TrainClass);
	free(Feature_matrix);

	ready = true;
}

bool LDA::modelReady() const
{
	return ready;
}

int LDA::predict(const DataVector & dvec) const
{
	float *Feature_test = (float*)malloc(featureDim*sizeof(float));
	float *tmp = (float*)malloc(1 * nClasses*sizeof(float));
	float *tmp1 = (float*)malloc(1 * nClasses*sizeof(float));
	float maxdata = -9999.0;
	int test_decision;

	//convert dvec to float array
	for (size_t i = 0; i < featureDim && i < dvec.length; i++)
	{
		Feature_test[i] = dvec.data[i];
	}

	feature_normalization_apply(Feature_test, xmean, xstd, featureDim);

	mulAB(Feature_test, Wg, tmp, 1, featureDim, nClasses);
	addition(Cg, tmp, tmp1, 1, nClasses);

	for (int j = 0; j < nClasses; j++) {
		if (tmp1[j] > maxdata)
		{
			maxdata = tmp1[j];
			test_decision = j;
		}
	}

	free(Feature_test);
	free(tmp);
	free(tmp1);
	return test_decision;
}

int LDA::saveModel(std::string filename) const
{
	//ensure a model exists
	if (!ready)
	{
		return -1;
	}

	std::ofstream ofile(filename, std::ios::ios_base::out | std::ios::ios_base::trunc);
	if (!ofile.is_open())	//failed
	{
		ofile.close();
		return 1;
	}
	
	//write Wg
	for (size_t i = 0; i < featureDim * nClasses; i++)
	{
		ofile << Wg[i] << " ";
	}
	ofile << std::endl;

	//write Cg
	for (size_t i = 0; i < nClasses; i++)
	{
		ofile << Cg[i] << " ";
	}
	ofile << std::endl;

	//write xmean
	for (size_t i = 0; i < featureDim; i++)
	{
		ofile << xmean[i] << " ";
	}
	ofile << std::endl;

	//write xstd
	for (size_t i = 0; i < featureDim; i++)
	{
		ofile << xstd[i] << " ";
	}
	ofile << std::endl;

	return 0;
}

void LDA::loadModel(std::string filename)
{
	std::ifstream ifile(filename);
	if (!ifile.is_open())	//failed
	{
		ifile.close();
		return;
	}

	std::vector<float> vWg;		//f x c
	std::vector<float> vCg;		//c
	std::vector<float> vxmean;	//f
	std::vector<float> vxstd;	//f

	std::string line;

	//read Wg
	std::getline(ifile, line);
	parseFloat(line, vWg);

	//read Cg
	std::getline(ifile, line);
	parseFloat(line, vCg);

	//read xmean
	std::getline(ifile, line);
	parseFloat(line, vxmean);

	//read xstd
	std::getline(ifile, line);
	parseFloat(line, vxstd);

	ifile.close();

	//check dimensions
	int tmpnFeat = vxmean.size();
	int tmpnClass = vCg.size();
	if ((tmpnFeat == 0) ||
		(tmpnClass == 0) ||
		(vWg.size() != tmpnClass * tmpnFeat) ||
		(vxstd.size() != tmpnFeat))
	{
		return;
	}

	//copy
	free(Wg);
	free(Cg);
	free(xmean);
	free(xstd);

	featureDim = tmpnFeat;
	nClasses = tmpnClass;

	Wg = (float*)malloc(featureDim*nClasses*sizeof(float));
	Cg = (float*)malloc(1 * nClasses*sizeof(float));
	xmean = (float*)malloc(1 * featureDim*sizeof(float));
	xstd = (float*)malloc(1 * featureDim*sizeof(float));

	//write Wg
	for (size_t i = 0; i < featureDim * nClasses; i++)
	{
		Wg[i] = vWg[i];
	}

	//write Cg
	for (size_t i = 0; i < nClasses; i++)
	{
		Cg[i] = vCg[i];
	}

	//write xmean
	for (size_t i = 0; i < featureDim; i++)
	{
		xmean[i] = vxmean[i];
	}

	//write xstd
	for (size_t i = 0; i < featureDim; i++)
	{
		xstd[i] = vxstd[i];
	}

	ready = true;
}

size_t LDA::parseFloat(const std::string& str, std::vector<float>& list)
{
	float temp;
	std::stringstream ss;
	list.clear();
	ss << str;
	while (1)
	{
		ss >> temp;
		if (!ss.good()) break;
		list.push_back(temp);
	}
	return list.size();
}

