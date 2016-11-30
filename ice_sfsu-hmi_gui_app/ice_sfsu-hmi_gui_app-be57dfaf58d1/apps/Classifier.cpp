#include "Classifier.h"


Classifier::Classifier()
{
}


Classifier::~Classifier()
{
}

int Classifier::getLatestPrediction() const
{
	return latestPrediction;
}

std::pair<int, int> Classifier::testAccuracy(const std::vector<DataVector> & data) const
{
	int correct = 0;
	int total = data.size();
	for (DataVector dvec : data)
	{
		if (dvec.flag == predict(dvec))
		{
			correct++;
		}
	}
	return std::pair<int, int>(correct, total);
}

float * Classifier::crossAccuracy(const std::vector<DataVector>& data, int nClass, int parts)
{
	int correct = 0;
	int total = 0;
	float* cM = new float[nClass * nClass + 1];
	int** confMatrix = new int*[nClass];
	for (int i = 0; i < nClass; i++) {
		confMatrix[i] = new int[nClass];
		for (int j = 0; j < nClass; j++) {
			confMatrix[i][j] = 0;
		}
	}

	std::vector <std::vector <DataVector>> separateData;

	for (int classIndex = 0; classIndex < nClass; classIndex++) {
		std::vector<DataVector> temp;
		for (unsigned i = 0; i < data.size(); i++) {
			if (classIndex == data[i].flag) {
				temp.push_back(data[i]);
			}
		}
		separateData.push_back(temp);
	}

	std::vector <DataVector> auxData;
	for (int i = 0; i < separateData.size(); i++) {
		for (int j = 0; j < separateData[i].size(); j++) {
			auxData.push_back(separateData[i][j]);
		}
	}

	std::vector<int> numElementGroup; //number of elements in a group for a class
	for (int i = 0; i < separateData.size(); i++) {
		numElementGroup.push_back(int(separateData[i].size() / parts));
	}

	std::vector< DataVector > auxTrain;
	for (int j = 0; j < parts; j++) {
		for (int k = 0; k < nClass; k++) {
			int element = numElementGroup[k];
			int index = ((j + 1)*element) - 1;
			for (int l = 0; l < separateData[k].size(); l++) {
				if ((l >= j*element) && (l < ((j + 1) * element))) {
					auxTrain.push_back(separateData[k][l]);
				}
				else {
					auxData.push_back(separateData[k][l]);
				}
			}
				//			for (int l = j*element; l < ((j + 1) * element); l++) {
				//				auxTrain.push_back(separateData[k][l]); //this works with the above conditions
				//				auxData.erase(auxData.begin() + ((k + 1)*index--));
				//			}
		}
		
		train(auxTrain);
		
		for (int m = 0; m < auxData.size(); m++) {
			total++;
			int pred = predict(auxData[m]);
			confMatrix[(int)auxData[m].flag][(int)pred]++;
			if (auxData[m].flag == pred) {
				correct++;
			}
		}

		auxTrain.clear();

		while (auxData.size() != 0) {//for (int n = 0; n < auxData.size(); n++) {
			auxData.erase(auxData.begin());
		}
			
			//		for (int i = 0; i < separateData.size(); i++) {
			//			for (int k = 0; k < separateData[i].size(); k++) {
			//				auxData.push_back(separateData[i][k]);
			//			}
			//		}
	}
	cM[0] = (float)correct / (float)total;
	for (int i = 0; i < nClass; i++) { //i known, j pred
		for (int j = 0; j < nClass; j++) {
			cM[1 + j + i*nClass] = 100 * nClass * (float)confMatrix[i][j] / total;
		}
	}
	return cM;
}