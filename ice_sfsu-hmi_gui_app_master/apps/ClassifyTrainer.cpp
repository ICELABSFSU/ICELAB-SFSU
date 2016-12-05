#include "ClassifyTrainer.h"
#include <string>


ClassifyTrainer::ClassifyTrainer()
{
}


ClassifyTrainer::~ClassifyTrainer()
{
}

void ClassifyTrainer::acceptData(const DataVector & dvec)
{
	if (trainingActive && dvec.flag >= 0)
	{
		if (databuf.find(dvec.flag) == databuf.end())
		{
			//assign index
			std::string newClass = std::string("__class") + std::to_string(dvec.flag);
			classNames.emplace(dvec.flag, newClass);
			classIndex.emplace(newClass, dvec.flag);

			//initialize space in data map
			databuf.emplace(dvec.flag, std::vector<DataVector>());
		}
		databuf.at(dvec.flag).push_back(DataVector(dvec));
		notifyAll(dvec);

		sampleCount++;
		if (sampleCount == sampleSize)
		{
			trainingActive = 0;
		}
	}
}

void ClassifyTrainer::addDataset(const std::vector<DataVector>& data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i].flag >= 0)
		{
			if (databuf.find(data[i].flag) == databuf.end())
			{
				//assign index
				std::string newClass = std::string("__class") + std::to_string(data[i].flag);
				classNames.emplace(data[i].flag, newClass);
				classIndex.emplace(newClass, data[i].flag);

				//initialize space in data map
				databuf.emplace(data[i].flag, std::vector<DataVector>());
			}
			databuf.at(data[i].flag).push_back(DataVector(data[i]));
		}
	}
}

void ClassifyTrainer::startCollect()
{
	trainingActive = 1;
	sampleSize = -1;
}

void ClassifyTrainer::startCollect(int sampleSize)
{
	this->sampleSize = sampleSize;
	sampleCount = 0;
	trainingActive = 1;
}

void ClassifyTrainer::stopCollect()
{
	trainingActive = 0;
}

int ClassifyTrainer::isActive() const
{
	return trainingActive;
}

int ClassifyTrainer::getSampleCount() const
{
	return sampleCount;
}

int ClassifyTrainer::getClassSampleCount(int classIndex) const
{
	return databuf.at(classIndex).size();
}

int ClassifyTrainer::addClassName(std::string& name)
{
	//if name is unique
	if (classIndex.find(name) == classIndex.end())
	{
		//assign an unused index
		for (int i = 0; i < 100; i++)
		{
			if (classNames.find(i) == classNames.end())
			{
				std::string newClass(name);
				classNames.emplace(i, newClass);
				classIndex.emplace(newClass, i);

				//initialize space in data map
				databuf.emplace(i, std::vector<DataVector>());
				break;
			}
		}
	}
	return classIndex.at(name);
}

void ClassifyTrainer::renameClass(int index, std::string & newName)
{
	std::string oldName = classNames.at(index);
	// replace index-name map
	classNames.at(index) = newName;
	// remove old name and emplace new name in name-index map
	classIndex.erase(oldName);
	classIndex.emplace(newName, index);
}

// JTY Add picture filepath to classPicturePath
void ClassifyTrainer::selectPicture(int index, std::string& path)
{
	if (classPicturePath.find(index) != classPicturePath.end()) {
		classPicturePath.erase(index);
	}
	classPicturePath.emplace(index, path);
}



void ClassifyTrainer::removeClass(int index)
{
	//remove from name map
	classIndex.erase(classNames[index]);
	classNames.erase(index);
	// JTY remove classification[index] image filepath
	classPicturePath.erase(index);

	//remove from data map
	databuf.erase(index);
}

const std::string ClassifyTrainer::getClassName(int index) const
{
	std::string name;
	if (classNames.find(index) != classNames.end())
	{
		name = classNames.at(index);
	}
	else
	{
		name = "no name!";
	}
	return name;
}

// JTY returns System::String^ filepath of classification at index
System::String^ ClassifyTrainer::getClassPicturePath(int index) const
{
	if (classPicturePath.find(index) == classPicturePath.end()) {
		return "";
	}
	else {
		System::String^ path = gcnew System::String(classPicturePath.at(index).c_str());
		return path;
	}
}



int ClassifyTrainer::getClassIndex(std::string& name) const
{
	if (classIndex.find(name) == classIndex.end())
		return -1;
	return  classIndex.at(name);
}

int ClassifyTrainer::getNumberOfClasses() const
{
	return classNames.size();
}

std::vector<std::pair<int, std::string>> ClassifyTrainer::getAllClasses() const
{
	std::vector<std::pair<int, std::string>> classList;
	for (auto it = classNames.begin(); it != classNames.end(); it++)
	{
		classList.push_back(std::pair<int, std::string>(it->first, it->second));
	}
	return classList;
}

const std::vector<DataVector> ClassifyTrainer::data() const
{
	std::vector<DataVector> dataout;
	int i = 0;
	//iterate through classes
	for (auto it = databuf.begin(); it != databuf.end(); it++)
	{
		for (int w = 0; w < it->second.size(); w++)
		{
			dataout.push_back(DataVector(it->second[w]));
			i++;
		}
	}
	return dataout;
}

void ClassifyTrainer::deleteClassData(int classIndex)
{
	databuf.at(classIndex).clear();
}

void ClassifyTrainer::clearData()
{
	for (auto it = databuf.begin(); it != databuf.end(); it++)
	{
		//clear DataVector vector
		it->second.clear();
	}
}

void ClassifyTrainer::clearAll()
{
	databuf.clear();
	classNames.clear();
	classIndex.clear();
	// JTY clear classPicturePath
	classPicturePath.clear();

}

/*void ClassifyTrainer::train(LDA & lda)
{
	const int nWindow = databuf.size();	//total number of windows (all classes)
	if (nWindow == 0) return;		//error empty dataset
	const int fdim = databuf[0].length;	//assuming all windows have same number of features
	std::map<int, int> classWin;		//key class, value number of windows

	//count windows per class
	for (int w = 0; w < nWindow; w++)
	{
		classWin.emplace(databuf[w].flag, 0);	//inserts only if key is unique
		classWin[databuf[w].flag]++;
	}
	const int nClass = classWin.size();	//total number of classes

	float *** classData = new float** [nClass];
	for (int c = 0; c < nClass; c++)
	{
		classData[c] = new float* [classWin[c]];
		for (int w = 0; w < classWin[c]; w++)
		{
			classData[c][w] = new float[fdim]();
		}
	}

	//organize windows by class and place in float matrix
	int* Wspecific = new int[nClass]();
	int c;
	for (int w = 0; w < nWindow; w++)
	{
		//add to appropriate class
		c = databuf[w].flag;
		for (int f = 0; f < fdim; f++)
		{
			classData[c][Wspecific[c]][f] = databuf[w].data[f];
		}
		Wspecific[c]++;
	}



	//pass to LDA
	lda.train(classData, Wspecific, fdim, nClass);

	System::Diagnostics::Debug::Write(lda.getTrainAccuracy());
	System::Diagnostics::Debug::Write(" ");
	System::Diagnostics::Debug::Write(lda.getTestAccuracy());


	delete[] Wspecific;
	for (int c = 0; c < nClass; c++)
	{
		for (int w = 0; w < Wspecific[c]; w++)
		{
			delete[] classData[c][w];
		}
		delete[] classData[c];
	}
	delete[] classData;
}*/
