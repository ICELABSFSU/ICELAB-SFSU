#include "SVMAgent.h"

SVMAgent::SVMAgent()
{
	//from svm-train.c
	// default values
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0.1;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	setKernelLinear();
}

SVMAgent::~SVMAgent()
{
	svm_free_and_destroy_model(&model);
	svm_destroy_param(&param);
	delete[] prob.y;
	for (int i = 0; i < prob.l; i++)
	delete[] prob.x[i];
	delete[] prob.x;
}

void SVMAgent::acceptData(const DataVector & dvec)
{
	latestPrediction = (int)svm_predict(model, convertDataVector(dvec));
	//build data vector, zero-length data array
	//only information is classification (flag)
	notifyAll(DataVector(latestPrediction, 0, 0, dvec.timestamp));
}

void SVMAgent::train(const std::vector<DataVector> & data)
{
	//prepare svm_problem
	const int nWindows = data.size();
	const int nFeatures = data[0].length;
	prob.l = nWindows;
	prob.y = new double[nWindows];
	prob.x = new svm_node*[nWindows];
	for (int i = 0; i < nWindows; i++)
	{
		prob.y[i] = data[i].flag;
		prob.x[i] = convertDataVector(data[i]);
	}

	//prepare svm_parameter
	//if gamma has not been set
	if (param.gamma == 0)
	{
		param.gamma = 1.0 / (float)nFeatures;
	}

	if (svm_check_parameter(&prob, &param) != NULL)
	{
		return;
	}

	model = svm_train(&prob, &param);
}

bool SVMAgent::modelReady() const
{
	int ready = false;
	if (model != nullptr && model->nr_class > 0)
	ready = true;
	return ready;
}


std::pair<int, int> SVMAgent::testAccuracy(const std::vector<DataVector>& data) const
{
	int correct = 0;
	int nClass = 0;
	int total = data.size();
	for (int i = 0; i < total; i++)
	{
		if (data[i].flag > nClass)
		{
			nClass = data[i].flag;
		}
	}
	for (int i = 0; i < total; i++)
	{
		if (data[i].flag == predict(data[i]))
		{
			correct++;
		}
	}
	nClass++;
	return std::pair<int, int>(nClass, total);
}

int SVMAgent::predict(const DataVector & dvec) const
{
	return (int)svm_predict(model, convertDataVector(dvec));
}

void SVMAgent::setType(int svm_type)
{
	param.svm_type = svm_type;
}

void SVMAgent::setKernelLinear()
{
	param.kernel_type = LINEAR;
}

void SVMAgent::setKernelPoly(int degree, double gamma, double coef0)
{
	param.kernel_type = POLY;
	param.degree = degree;
	param.gamma = gamma;
	param.coef0 = coef0;
}

void SVMAgent::setKernelRBF(double gamma)
{
	param.kernel_type = RBF;
	param.gamma = gamma;

}

void SVMAgent::setKernelSigmoid(double gamma, double coef0)
{
	param.kernel_type = SIGMOID;
	param.gamma = gamma;
	param.coef0 = coef0;
}

int SVMAgent::saveModel(std::string filename) const
{
	return svm_save_model(filename.c_str(), model);
}

void SVMAgent::loadModel(std::string filename)
{
	model = svm_load_model(filename.c_str());
}

svm_node * SVMAgent::convertDataVector(const DataVector & dvec)
{
	svm_node* x = new svm_node[dvec.length + 1];
	for (int f = 0; f < dvec.length; f++)
	{
		x[f].index = f;
		x[f].value = dvec.data[f];
	}
	x[dvec.length].index = -1;	//end of vector
	return x;
}

