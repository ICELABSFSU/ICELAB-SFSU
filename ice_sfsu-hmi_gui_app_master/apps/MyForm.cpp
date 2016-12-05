#include "MyForm.h"

#define nbr_ptn 16


using namespace System;
using namespace System::Windows::Forms;
using namespace System::Windows::Forms::DataVisualization::Charting;


void Myo_app::MyForm::inputAxesInit()
{
	inputAxes = new std::map<std::string, int>();
	int i = ct.getNumberOfClasses();	//separate from indexing region of gestures
	inputAxes->emplace("Roll", i++);
	inputAxes->emplace("Pitch", i++);
	inputAxes->emplace("Yaw", i++);
	inputAxes->emplace("AccelX", i++);
	inputAxes->emplace("AccelY", i++);
	inputAxes->emplace("AccelZ", i++);
	inputAxes->emplace("Mean MAV", i++);
	inputAxes->emplace("Mean Wave", i++);
	inputAxes->emplace("Mean Zero Cross", i++);
	inputAxes->emplace("Mean Turns", i++);
}

void Myo_app::MyForm::inputAxesFill()
{
	comboBoxInputAxis->Items->Clear();
	for (auto it = inputAxes->begin(); it != inputAxes->end(); it++)
	{
		String^ name = gcnew String(it->first.c_str());
		comboBoxInputAxis->Items->Add(name);
	}
	comboBoxInputAxis->SelectedIndex = 0;
}

void Myo_app::MyForm::outputAxesInit()
{
	outputAxes = new std::map<std::string, int>();
	outputAxes->emplace("X", HID_USAGE_X);
	outputAxes->emplace("Y", HID_USAGE_Y);
	outputAxes->emplace("Z", HID_USAGE_Z);
	outputAxes->emplace("RX", HID_USAGE_RX);
	outputAxes->emplace("RY", HID_USAGE_RY);
	outputAxes->emplace("RZ", HID_USAGE_RZ);
	outputAxes->emplace("SL0", HID_USAGE_SL0);
	outputAxes->emplace("SL1", HID_USAGE_SL1);
}

void Myo_app::MyForm::outputAxesFill()
{
	comboBoxOutputAxis->Items->Clear();
	for (auto it = outputAxes->begin(); it != outputAxes->end(); it++)
	{
		String^ name = gcnew String(it->first.c_str());
		comboBoxOutputAxis->Items->Add(name);
	}
	comboBoxOutputAxis->SelectedIndex = 0;
}

void Myo_app::MyForm::updateComboBoxGestureOutput()
{
	comboBoxOutputGestureSelect->Items->Clear();
	if (ct.getNumberOfClasses())
	{
		std::vector<std::pair<int, std::string>> classesVector = ct.getAllClasses();
		for (std::pair<int, std::string> pair : classesVector)
		{
			comboBoxOutputGestureSelect->Items->Add(L"" + pair.first + L" " + gcnew String(pair.second.c_str()));
		}
		comboBoxOutputGestureSelect->SelectedIndex = 0;
	}
}

void Myo_app::MyForm::updateComboBoxNextGesture()
{
	//fill combobox
	comboBoxNextGesture->Items->Clear();
	comboBoxNextGesture->Items->Add(L"" + (-1) + L" null");

	std::vector<std::pair<int, std::string>> classesVector = ct.getAllClasses();
	for (std::pair<int, std::string> pair : classesVector)
	{
		comboBoxNextGesture->Items->Add(L"" + pair.first + L" " + gcnew String(pair.second.c_str()));
	}
	comboBoxNextGesture->SelectedIndex = 0;
}

void Myo_app::MyForm::runMyo()
{
	while (md.myo) {
		hub->runOnce(1);
	}
}

void Myo_app::MyForm::playback()
{
	do
	{
		reader.seek(0);
		while (currentSource == DataSource::file && reader.sendNextData()) {
			Thread::Sleep(5);
		}
	} while (currentSource == DataSource::file && checkBoxPlaybackRepeat->Checked);
	currentSource = DataSource::none;
}

void Myo_app::MyForm::startStreamingMyo()
{
	//enable streaming EMG
	md.myo->setStreamEmg(myo::Myo::streamEmgEnabled);
	//connect MyoData
	fc.reset();
	md.addConsumer(fc);
	fc.addSwingFinder(sf);
	fc.addMMAVPkFinder(mpf);
	fc.addSSorTrans(sst);
	//update ui
	labelMyo1StreamingStatus->Text = L"Streaming EMG";
	myo1Streaming = true;
	buttonMyo1Stream->Text = L"Disable";
	buttonRecord->Enabled = true;
	toolStripStatusLabelDataSource->Text = L"Streaming from Myo";
	toolStripProgressBarDataSourceStatus->Style = ProgressBarStyle::Marquee;
	toolStripStatusLabelDataSourceStatus->Text = L"-";
	toolStripStatusLabelStatus->Text = L"Streaming started.";

	md.myo->vibrate(myo::Myo::vibrationShort);
	currentSource = DataSource::myo;
}

void Myo_app::MyForm::stopStreamingMyo()
{
	//disconnect MyoData
	md.removeConsumer(fc);
	fc.reset();
	//disable streaming EMG
	md.myo->setStreamEmg(myo::Myo::streamEmgDisabled);
	//update ui
	labelMyo1StreamingStatus->Text = L"Not streaming EMG";
	myo1Streaming = false;
	buttonMyo1Stream->Text = L"Enable";
	buttonRecord->Enabled = false;
	toolStripStatusLabelDataSource->Text = L"No data source";
	toolStripProgressBarDataSourceStatus->Style = ProgressBarStyle::Continuous;
	toolStripStatusLabelStatus->Text = L"Streaming stopped.";

	currentSource = DataSource::none;
}

void Myo_app::MyForm::startFilePlayback()
{
	//connect reader
	fc.reset();
	reader.addConsumer(fc);
	//update ui
	buttonPlaybackLoad->Enabled = false;
	buttonPlaybackPlay->Text = L"Stop";
	progressBarPlayback->Maximum = reader.size();
	toolStripStatusLabelDataSource->Text = L"File playback";
	toolStripProgressBarDataSourceStatus->Maximum = reader.size();

	currentSource = DataSource::file;
	fileThread = gcnew Thread(gcnew ThreadStart(
		this, &MyForm::playback));
	fileThread->Start();
}

void Myo_app::MyForm::stopFilePlayback()
{
	//disconnect reader
	currentSource = DataSource::none;
	fileThread->Join();	//wait for playback thread to stop
	reader.removeConsumer(fc);
	fc.reset();
	//update ui
	buttonPlaybackLoad->Enabled = true;
	buttonPlaybackPlay->Text = L"Play File";
	toolStripStatusLabelDataSource->Text = L"No data source";
}

void Myo_app::MyForm::centerMyo(MyoData & md)
{
	//store conjugate of centered orientation
	//relative orientation can be obtained from multipling by current orientation
	myo::Quaternion<float> center = md.orientation().conjugate();
	myoInitOrientation[0] = center.x();
	myoInitOrientation[1] = center.y();
	myoInitOrientation[2] = center.z();
	myoInitOrientation[3] = center.w();
}

void Myo_app::MyForm::init_EMG_charts(void)
{
	//location of charts
	/*
	this->chart1->Location = System::Drawing::Point(336, 195);
	this->chart1->Size = System::Drawing::Size(200, 200);
	this->chart3->Location = System::Drawing::Point(336, 451);
	this->chart3->Size = System::Drawing::Size(200, 200);
	this->chart2->Location = System::Drawing::Point(623, 195);
	this->chart2->Size = System::Drawing::Size(200, 200);
	*/

	for (int i(0); i < 2; i++)
	{
		chartEMG[i]->Legends->Clear();
		chartEMG[i]->Series->Clear();
	}

	for (int i(0); i < 2; i++)
	{
		chartEMG[i]->ChartAreas->Add("area");
	}

	chart3->ChartAreas->Add("area");

	chart1->ChartAreas["area"]->AxisY->Title = L"Zero %";
	chart2->ChartAreas["area"]->AxisY->Title = L"Turn %";

	for (int i(0); i < 2; i++)
	{
		chartEMG[i]->ChartAreas["area"]->AxisX->Minimum = 0;
		chartEMG[i]->ChartAreas["area"]->AxisX->Maximum = 8;
		chartEMG[i]->ChartAreas["area"]->AxisX->Interval = 1;
		chartEMG[i]->ChartAreas["area"]->AxisY->Minimum = 0;
		chartEMG[i]->ChartAreas["area"]->AxisY->Maximum = 100;
		chartEMG[i]->ChartAreas["area"]->AxisY->Interval = 10;

		chartEMG[i]->Series->Add("radar1");
		chartEMG[i]->Series["radar1"]->ChartType = DataVisualization::Charting::SeriesChartType::Radar;
		chartEMG[i]->Series["radar1"]->BorderWidth = thickness;
		chartEMG[i]->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::True;
		chartEMG[i]->Series["radar1"]->Color = Color::FromArgb(120, 0, 100, 255);

		chartEMG[i]->Series->Add("radar2");
		chartEMG[i]->Series["radar2"]->ChartType = DataVisualization::Charting::SeriesChartType::Radar;
		chartEMG[i]->Series["radar2"]->BorderWidth = thickness;
		chartEMG[i]->Series["radar2"]->Color = Color::FromArgb(120, 255, 0, 0);

		chartEMG[i]->ChartAreas["area"]->InnerPlotPosition->Auto = false;
		chartEMG[i]->ChartAreas["area"]->InnerPlotPosition->Height = 100;
		chartEMG[i]->ChartAreas["area"]->InnerPlotPosition->Width = 100;
		chartEMG[i]->ChartAreas["area"]->InnerPlotPosition->X = 0;
		chartEMG[i]->ChartAreas["area"]->InnerPlotPosition->Y = 0;
	}

	/// bar graph




	chart3->ChartAreas["area"]->InnerPlotPosition->Auto = false;
	chart3->ChartAreas["area"]->InnerPlotPosition->Height = 90;
	chart3->ChartAreas["area"]->InnerPlotPosition->Width = 100;
	chart3->ChartAreas["area"]->InnerPlotPosition->X = 7;
	chart3->ChartAreas["area"]->InnerPlotPosition->Y = 3;

	//	chart3->ChartAreas["area"]->AxisY->Title = L"";

	chart3->ChartAreas["area"]->AxisX->Minimum = 0;
	chart3->ChartAreas["area"]->AxisX->Maximum = 3;
	chart3->ChartAreas["area"]->AxisX->Interval = 1;
	chart3->ChartAreas["area"]->AxisY->Minimum = 0;
	chart3->ChartAreas["area"]->AxisY->Maximum = 70;
	chart3->ChartAreas["area"]->AxisY->Interval = 5;
	chart3->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::False; //db flase


	chart3->Series->Add("bar1");
	chart3->Series["bar1"]->ChartType = DataVisualization::Charting::SeriesChartType::Column;
	chart3->Series["bar1"]->BorderWidth = thickness;

	chart3->Series->Add("bar2");
	chart3->Series["bar2"]->ChartType = DataVisualization::Charting::SeriesChartType::Column;
	chart3->Series["bar2"]->BorderWidth = thickness;
	chart3->Series["bar2"]->Color = Color::FromArgb(125, 51, 255, 0);

	chart3->Series->Add("bar3");
	chart3->Series["bar3"]->ChartType = DataVisualization::Charting::SeriesChartType::Column;
	chart3->Series["bar3"]->BorderWidth = thickness;
	chart3->Series["bar3"]->Color = Color::FromArgb(125, 255, 204, 0);

	chart3->Series->Add("bar4");
	chart3->Series["bar4"]->ChartType = DataVisualization::Charting::SeriesChartType::Column;
	chart3->Series["bar4"]->BorderWidth = thickness;
	chart3->Series["bar4"]->Color = Color::FromArgb(125, 255, 0, 0);

	chart3->Legends->Add("Average MAV");
	chart3->Series["bar1"]->LegendText = "Average WAVE";
	chart3->Series["bar1"]->IsVisibleInLegend = true;

	chart3->Legends->Add("Average WAVE");
	chart3->Series["bar2"]->LegendText = "Average MAV";
	chart3->Series["bar2"]->IsVisibleInLegend = true;

	chart3->Legends->Add("Average Zero");
	chart3->Series["bar3"]->LegendText = "Average Zero";
	chart3->Series["bar3"]->IsVisibleInLegend = true;

	chart3->Legends->Add("Average Turn");
	chart3->Series["bar4"]->LegendText = "Average Turn";
	chart3->Series["bar4"]->IsVisibleInLegend = true;

}

//Used on DisplaEMG() so that the display of the EMG data on the graph does not go beyond 100
float scaling(float inValue) {
	float outValue = inValue;
	if (outValue > 100) { outValue = 100; }
	return outValue;
}

void Myo_app::MyForm::Display_EMG()
{
	chart3->Series["bar1"]->Points->Clear();
	chart3->Series["bar2"]->Points->Clear();
	chart3->Series["bar3"]->Points->Clear();
	chart3->Series["bar4"]->Points->Clear();

	for (int a(0); a < 2; a++) {
		chartEMG[a]->Series["radar1"]->Points->Clear();
		chartEMG[a]->Series["radar2"]->Points->Clear();
	}

	// Average Bars
	chart3->Series["bar1"]->Points->AddXY(1, fc.getFeatureAverage(0));
	chart3->Series["bar2"]->Points->AddXY(2, fc.getFeatureAverage(1));
	chart3->Series["bar3"]->Points->AddXY(1, fc.getFeatureAverage(2));
	chart3->Series["bar4"]->Points->AddXY(2, fc.getFeatureAverage(3));

	for (int a(0); a < 8; a++)
	{
		float cbBox7 = scaling(fc.getFeature(comboBox7->SelectedIndex, a));
		float cbBox9 = scaling(fc.getFeature(comboBox9->SelectedIndex, a));
		float cbBox6 = scaling(fc.getFeature(comboBox6->SelectedIndex, a));
		float cbBox8 = scaling(fc.getFeature(comboBox8->SelectedIndex, a));

		chartEMG[1]->Series["radar1"]->Points->AddXY(a, cbBox7);
		chartEMG[1]->Series["radar2"]->Points->AddXY(a, cbBox9);
		chartEMG[0]->Series["radar1"]->Points->AddXY(a, cbBox6);
		chartEMG[0]->Series["radar2"]->Points->AddXY(a, cbBox8);
		
		/*if (!checkBox7->Checked) {
			chartEMG[0]->Series["radar1"]->Points->AddXY(a, fc.getFeature(comboBox6->SelectedIndex, a));
			chartEMG[0]->Series["radar2"]->Points->AddXY(a, fc.getFeature(comboBox8->SelectedIndex, a));
		}
		/*
		else
		{
			comboBox6->SelectedIndex = 0;
			comboBox8->SelectedIndex = 1;

			double normFeatAvg[4];
			for (int i = 0; i < 2; i++) {
				normFeatAvg[i] = fc.getFeature(i, a) / fc.getFeatureAverage(i);
				if (normFeatAvg[i] > 5 && fc.getFeatureAverage(1) > deviceMyo.collector.threshold) {
					normFeatAvg[i] = 5;
				}
				if (normFeatAvg[i] >= 5 && fc.getFeatureAverage(1) <= deviceMyo.collector.threshold) {
					normFeatAvg[i] = 1;
				}
			}
			chartEMG[0]->Series["radar1"]->Points->AddXY(a, normFeatAvg[comboBox6->SelectedIndex]);
			chartEMG[0]->Series["radar2"]->Points->AddXY(a, normFeatAvg[comboBox8->SelectedIndex]);
		}*/
	}
}


void Myo_app::MyForm::Relabel_EMG_chart_area(Chart^ chart) {
	//updating titles, can be optimized
	if (comboBox6->SelectedIndex == 0) {
		chartEMG[0]->ChartAreas["area"]->AxisY->Title = L"WAVE";
	}
	else if (comboBox6->SelectedIndex == 1) {
		chartEMG[0]->ChartAreas["area"]->AxisY->Title = L"MAV";
	}
	else if (comboBox6->SelectedIndex == 2) {
		chartEMG[0]->ChartAreas["area"]->AxisY->Title = L"Zero %";
	}
	else if (comboBox6->SelectedIndex == 3) {
		chartEMG[0]->ChartAreas["area"]->AxisY->Title = L"Turn %";
	}
	else if (comboBox6->SelectedIndex == 4) {
		chartEMG[0]->ChartAreas["area"]->AxisY->Title = L"MAV/MMAV";
	}



	if (comboBox8->SelectedIndex == 0) {
		chartEMG[0]->ChartAreas["area"]->AxisX->Title = L"WAVE";
	}
	else if (comboBox8->SelectedIndex == 1) {
		chartEMG[0]->ChartAreas["area"]->AxisX->Title = L"MAV";
	}
	else if (comboBox8->SelectedIndex == 2) {
		chartEMG[0]->ChartAreas["area"]->AxisX->Title = L"Zero %";
	}
	else if (comboBox8->SelectedIndex == 3) {
		chartEMG[0]->ChartAreas["area"]->AxisX->Title = L"Turn %";
	}
	else if (comboBox8->SelectedIndex == 4) {
		chartEMG[0]->ChartAreas["area"]->AxisX->Title = L"MAV/MMAV";
	}



	if (comboBox7->SelectedIndex == 0) {
		chartEMG[1]->ChartAreas["area"]->AxisY->Title = L"WAVE";
	}
	else if (comboBox7->SelectedIndex == 1) {
		chartEMG[1]->ChartAreas["area"]->AxisY->Title = L"MAV";
	}
	else if (comboBox7->SelectedIndex == 2) {
		chartEMG[1]->ChartAreas["area"]->AxisY->Title = L"Zero %";
	}
	else if (comboBox7->SelectedIndex == 3) {
		chartEMG[1]->ChartAreas["area"]->AxisY->Title = L"Turn %";
	}
	else if (comboBox7->SelectedIndex == 4) {
		chartEMG[1]->ChartAreas["area"]->AxisY->Title = L"MAV/MMAV";
	}



	if (comboBox9->SelectedIndex == 0) {
		chartEMG[1]->ChartAreas["area"]->AxisX->Title = L" WAVE";
	}
	else if (comboBox9->SelectedIndex == 1) {
		chartEMG[1]->ChartAreas["area"]->AxisX->Title = L" MAV";
	}
	else if (comboBox9->SelectedIndex == 2) {
		chartEMG[1]->ChartAreas["area"]->AxisX->Title = L" Zero %";
	}
	else if (comboBox9->SelectedIndex == 3) {
		chartEMG[1]->ChartAreas["area"]->AxisX->Title = L" Turn %";
	}
	else if (comboBox9->SelectedIndex == 4) {
		chartEMG[1]->ChartAreas["area"]->AxisX->Title = L"MAV/MMAV";
	}

	//////
	/*
	if (!checkBox7->Checked) {//normalized
		chart->ChartAreas["area"]->AxisX->Minimum = 0;
		chart->ChartAreas["area"]->AxisX->Maximum = 8;
		chart->ChartAreas["area"]->AxisX->Interval = 1;
		chart->ChartAreas["area"]->AxisY->Minimum = 0;
		chart->ChartAreas["area"]->AxisY->Maximum = 100;
		chart->ChartAreas["area"]->AxisY->Interval = 10;
		chart->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::True;
	}
	else { 
		chart->ChartAreas["area"]->AxisX->Minimum = 0;
		chart->ChartAreas["area"]->AxisX->Maximum = 8;
		chart->ChartAreas["area"]->AxisX->Interval = 1;
		chart->ChartAreas["area"]->AxisY->Minimum = 0;
		chart->ChartAreas["area"]->AxisY->Maximum = 5;
		chart->ChartAreas["area"]->AxisY->Interval = 1;
		chart->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::True;
	}*/
};

void Myo_app::MyForm::playbackEMG(std::ifstream& file) {
	/*std::string emgLine;
	std::string emgTemp;

	for (int samp = 0; samp < 4; samp++) {
		std::getline(file, emgLine);

		if (!file.good()) {

			if (checkBox6->Checked == true) {
				file.close();
				file.open(stdOpenEMGFile);
				std::getline(file, emgTemp);
			}
			else {
				//stop upon loop ending
				// db
				openEMGFile.close();
				//
				break;
			}

		}
		if (playbackLoop) {
			currentNumberLinesEmgPlayback++;
		}
		textBox1->Text = "" + currentNumberLinesEmgPlayback; //db
		std::stringstream emgIss(emgLine);

		for (int col = 0; col < 9; col++) {
			std::string emgVal;
			std::getline(emgIss, emgVal, ',');

			if (!emgIss.good()) {
				//break;
			}

			std::stringstream converter(emgVal);

			if (col != 0) {
				converter >> deviceMyo.collector.emgSamples[col - 1][deviceMyo.collector.iteration_emg][0];
			}

		}

		deviceMyo.collector.iteration_emg++;
		deviceMyo.collector.iteration_emg &= 127;


	}*/
}

void Myo_app::MyForm::playbackIMU(std::ifstream& file) {

	/*for (int samp = 0; samp < 1; samp++) { // #RF : one iteration for loop used for breaking out of main timer loop to stop automatic looping if loop flag is not checked, 

		std::string imuLine;
		std::string imuTemp;

		std::getline(file, imuLine);

		// not exactly sure what this if statement is doing
		if (!file.good()) {
			//if loop box checked, loop, else stop loop
			if (checkBox6->Checked == true) {
				file.close();
				file.open(stdOpenIMUFile);
				std::getline(file, imuTemp);
			}
			else {
				//stop upon loop ending db
				openIMUFile.close();

				break;
			}
		}

		++currentNumberLinesImuPlayback;

		std::stringstream imuIss(imuLine);

		for (int col = 0; col < 10; col++) {

			std::string imuVal;
			std::getline(imuIss, imuVal, ',');
			if (!imuIss.good()) {
				//break;
			}
			std::stringstream converter(imuVal);
			if (col != 0) {
				converter >> deviceMyo.collector.IMUSamples[col - 1][deviceMyo.collector.iteration_imu][0];
			}

		}

		for (int col = 0; col < 9; col++) {
			std::string imuVal;
			std::getline(imuIss, imuVal, ',');
			if (!imuIss.good()) {
				//break;
			}
			std::stringstream converter(imuVal);

			converter >> deviceMyo.collector.IMUSamples[col][deviceMyo.collector.iteration_imu][1];

		}

	}*/

}

// mmav

void Myo_app::MyForm::init_mmav_chart(void)
{
	chart4->ChartAreas->Add("area");
	chart4->ChartAreas["area"]->AxisX->Minimum = 0;
	chart4->ChartAreas["area"]->AxisX->Maximum = 1000;
	chart4->ChartAreas["area"]->AxisX->Interval = 1;
	chart4->ChartAreas["area"]->AxisY->Minimum = 0;
	chart4->ChartAreas["area"]->AxisY->Maximum = 100;
	chart4->ChartAreas["area"]->AxisY->Interval = 10;
	chart4->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::False;
	chart4->ChartAreas["area"]->AxisY->Title = L"EMG sensors";

	chart4->ChartAreas["area"]->InnerPlotPosition->Auto = false;
	chart4->ChartAreas["area"]->InnerPlotPosition->Height = 100;
	chart4->ChartAreas["area"]->InnerPlotPosition->Width = 100;
	chart4->ChartAreas["area"]->InnerPlotPosition->X = 4;
	chart4->ChartAreas["area"]->InnerPlotPosition->Y = 2;

	for (int a(0); a < 8; a++)
	{
		chart4->Series->Add("sensor" + a + "");
		chart4->Series["sensor" + a + ""]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
		chart4->Series["sensor" + a + ""]->BorderWidth = thickness;
	}

	chart4->Series->Add("threshold");
	chart4->Series["threshold"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chart4->Series["threshold"]->BorderWidth = 2;
	chart4->Series["threshold"]->Color = Color::HotPink;

	chart4->Series->Add("mmav");
	chart4->Series["mmav"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chart4->Series["mmav"]->BorderWidth = 5;
	chart4->Series["mmav"]->Color = Color::Black;

	chart4->Series["sensor0"]->Color = Color::Green;
	chart4->Series["sensor1"]->Color = Color::Red;
	chart4->Series["sensor2"]->Color = Color::Blue;
	chart4->Series["sensor3"]->Color = Color::DarkViolet;
	chart4->Series["sensor4"]->Color = Color::DarkSlateGray;
	chart4->Series["sensor5"]->Color = Color::DarkOrange;
	chart4->Series["sensor6"]->Color = Color::DarkCyan;
	chart4->Series["sensor7"]->Color = Color::Black;
}

void Myo_app::MyForm::init_or_chart(void)
{
	chartOrPitch->ChartAreas->Add("area");
	chartOrPitch->ChartAreas["area"]->AxisX->Minimum = 0;
	chartOrPitch->ChartAreas["area"]->AxisX->Maximum = 500;
	chartOrPitch->ChartAreas["area"]->AxisX->Interval = 1;
	chartOrPitch->ChartAreas["area"]->AxisY->Minimum = -7;
	chartOrPitch->ChartAreas["area"]->AxisY->Maximum = 7;
	chartOrPitch->ChartAreas["area"]->AxisY->Interval = 1;
	chartOrPitch->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::False;
	chartOrPitch->ChartAreas["area"]->AxisY->Title = L"Orientation (Pitch)";

	chartOrPitch->ChartAreas["area"]->InnerPlotPosition->Auto = false;
	chartOrPitch->ChartAreas["area"]->InnerPlotPosition->Height = 100;
	chartOrPitch->ChartAreas["area"]->InnerPlotPosition->Width = 100;
	chartOrPitch->ChartAreas["area"]->InnerPlotPosition->X = 4;
	chartOrPitch->ChartAreas["area"]->InnerPlotPosition->Y = 2;

	chartOrPitch->Series->Add("average");
	chartOrPitch->Series["average"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartOrPitch->Series["average"]->BorderWidth = 2;
	chartOrPitch->Series["average"]->Color = Color::HotPink;

	chartOrPitch->Series->Add("orPitch");
	chartOrPitch->Series["orPitch"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartOrPitch->Series["orPitch"]->BorderWidth = 2;
	chartOrPitch->Series["orPitch"]->Color = Color::Black;
}

void Myo_app::MyForm::init_d2mmav_chart(void)
{
	chartD2MMAV->ChartAreas->Add("area");
	chartD2MMAV->ChartAreas["area"]->AxisX->Minimum = 0;
	chartD2MMAV->ChartAreas["area"]->AxisX->Maximum = 1000;
	chartD2MMAV->ChartAreas["area"]->AxisX->Interval = 1;
	chartD2MMAV->ChartAreas["area"]->AxisY->Minimum = 0;
	chartD2MMAV->ChartAreas["area"]->AxisY->Maximum = 100;
	chartD2MMAV->ChartAreas["area"]->AxisY->Interval = 10;
	chartD2MMAV->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::False;
	chartD2MMAV->ChartAreas["area"]->AxisY->Title = L"Delta 2 MMAV";

	chartD2MMAV->ChartAreas["area"]->InnerPlotPosition->Auto = false;
	chartD2MMAV->ChartAreas["area"]->InnerPlotPosition->Height = 100;
	chartD2MMAV->ChartAreas["area"]->InnerPlotPosition->Width = 100;
	chartD2MMAV->ChartAreas["area"]->InnerPlotPosition->X = 4;
	chartD2MMAV->ChartAreas["area"]->InnerPlotPosition->Y = 2;

	chartD2MMAV->Series->Add("threshold");
	chartD2MMAV->Series["threshold"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartD2MMAV->Series["threshold"]->BorderWidth = 2;
	chartD2MMAV->Series["threshold"]->Color = Color::HotPink;

	chartD2MMAV->Series->Add("mmav");
	chartD2MMAV->Series["mmav"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartD2MMAV->Series["mmav"]->BorderWidth = 1;
	chartD2MMAV->Series["mmav"]->Color = Color::Black;
}

void Myo_app::MyForm::init_windowedSTD_chart(void)
{
	chartSTD->ChartAreas->Add("area");
	chartSTD->ChartAreas["area"]->AxisX->Minimum = 0;
	chartSTD->ChartAreas["area"]->AxisX->Maximum = 1000;
	chartSTD->ChartAreas["area"]->AxisX->Interval = 1;
	chartSTD->ChartAreas["area"]->AxisY->Minimum = -5;
	chartSTD->ChartAreas["area"]->AxisY->Maximum = 50;
	chartSTD->ChartAreas["area"]->AxisY->Interval = 5;
	chartSTD->ChartAreas["area"]->AxisX->Enabled = DataVisualization::Charting::AxisEnabled::False;
	chartSTD->ChartAreas["area"]->AxisY->Title = L"Standard Deviation";

	chartSTD->ChartAreas["area"]->InnerPlotPosition->Auto = false;
	chartSTD->ChartAreas["area"]->InnerPlotPosition->Height = 100;
	chartSTD->ChartAreas["area"]->InnerPlotPosition->Width = 100;
	chartSTD->ChartAreas["area"]->InnerPlotPosition->X = 4;
	chartSTD->ChartAreas["area"]->InnerPlotPosition->Y = 2;

	chartSTD->Series->Add("threshold");
	chartSTD->Series["threshold"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartSTD->Series["threshold"]->BorderWidth = 2;
	chartSTD->Series["threshold"]->Color = Color::HotPink;

	chartSTD->Series->Add("std");
	chartSTD->Series["std"]->ChartType = DataVisualization::Charting::SeriesChartType::FastLine;
	chartSTD->Series["std"]->BorderWidth = 2;
	chartSTD->Series["std"]->Color = Color::Black;
}

void Myo_app::MyForm::mmav_display_line()
{
	//increment chart x axis
	chart4->ChartAreas["area"]->AxisX->Minimum++;
	chart4->ChartAreas["area"]->AxisX->Maximum++;

	//remove oldeset element
	//if more points in series than being shown, npoints > max - min + 1
	if (chart4->Series["threshold"]->Points->Count > (chart4->ChartAreas["area"]->AxisX->Maximum - chart4->ChartAreas["area"]->AxisX->Minimum + 1))
	{
		chart4->Series["threshold"]->Points->RemoveAt(0);
		chart4->Series["mmav"]->Points->RemoveAt(0);
	}

	//add to series from minimum up to and including maximum
	if (md.myo)
	{
		chart4->Series["threshold"]->Points->AddXY(chart4->ChartAreas["area"]->AxisX->Maximum + 1, (int)numericUpDown1->Value);
		chart4->Series["mmav"]->Points->AddXY(chart4->ChartAreas["area"]->AxisX->Maximum + 1, fc.getFeatureAverage(0));
	}
}

void Myo_app::MyForm::orPitch_display_line()
{
	//increment chart x axis
	chartOrPitch->ChartAreas["area"]->AxisX->Minimum++;
	chartOrPitch->ChartAreas["area"]->AxisX->Maximum++;

	//remove oldeset element
	//if more points in series than being shown, npoints > max - min + 1
	if (chartOrPitch->Series["average"]->Points->Count > (chartOrPitch->ChartAreas["area"]->AxisX->Maximum - chartOrPitch->ChartAreas["area"]->AxisX->Minimum + 1))
	{
		chartOrPitch->Series["average"]->Points->RemoveAt(0);
		chartOrPitch->Series["orPitch"]->Points->RemoveAt(0);
	}

	//add to series from minimum up to and including maximum
	if (md.myo)
	{
		chartOrPitch->Series["average"]->Points->AddXY(chartOrPitch->ChartAreas["area"]->AxisX->Maximum + 1, sf.getAverage());
		chartOrPitch->Series["orPitch"]->Points->AddXY(chartOrPitch->ChartAreas["area"]->AxisX->Maximum + 1, sf.getOrPitch());
	}
}

void Myo_app::MyForm::d2mmav_display_line()
{
	//increment chart x axis
	chartD2MMAV->ChartAreas["area"]->AxisX->Minimum++;
	chartD2MMAV->ChartAreas["area"]->AxisX->Maximum++;

	//remove oldeset element
	//if more points in series than being shown, npoints > max - min + 1
	if (chartD2MMAV->Series["threshold"]->Points->Count > (chartD2MMAV->ChartAreas["area"]->AxisX->Maximum - chartD2MMAV->ChartAreas["area"]->AxisX->Minimum + 1))
	{
		chartD2MMAV->Series["threshold"]->Points->RemoveAt(0);
		chartD2MMAV->Series["mmav"]->Points->RemoveAt(0);
	}

	//add to series from minimum up to and including maximum
	if (md.myo)
	{
		chartD2MMAV->Series["threshold"]->Points->AddXY(chartD2MMAV->ChartAreas["area"]->AxisX->Maximum + 1, (int)numericUpDownMMAVThresh->Value);
		chartD2MMAV->Series["mmav"]->Points->AddXY(chartD2MMAV->ChartAreas["area"]->AxisX->Maximum + 1, fc.getFeatureAverage(0));
	}
}

void Myo_app::MyForm::std_display_line()
{
	//increment chart x axis
	chartSTD->ChartAreas["area"]->AxisX->Minimum++;
	chartSTD->ChartAreas["area"]->AxisX->Maximum++;

	//remove oldeset element
	//if more points in series than being shown, npoints > max - min + 1
	if (chartSTD->Series["threshold"]->Points->Count > (chartSTD->ChartAreas["area"]->AxisX->Maximum - chartSTD->ChartAreas["area"]->AxisX->Minimum + 1))
	{
		chartSTD->Series["threshold"]->Points->RemoveAt(0);
		chartSTD->Series["std"]->Points->RemoveAt(0);
	}

	//add to series from minimum up to and including maximum
	if (md.myo)
	{
		chartSTD->Series["threshold"]->Points->AddXY(chartSTD->ChartAreas["area"]->AxisX->Maximum + 1, sst.getThresh());
		chartSTD->Series["std"]->Points->AddXY(chartSTD->ChartAreas["area"]->AxisX->Maximum + 1, sst.getWindowedSTD());
	}
}

void Myo_app::MyForm::printEMG() {
	if (currentSource == DataSource::file)
	{
		DataVector dvec = reader.getData(reader.getCurrentIndex());
		textBox54->Text = "" + dvec.data[0];
		textBox51->Text = "" + dvec.data[1];
		textBox52->Text = "" + dvec.data[2];
		textBox49->Text = "" + dvec.data[3];
		textBox53->Text = "" + dvec.data[4];
		textBox15->Text = "" + dvec.data[5];
		textBox50->Text = "" + dvec.data[6];
		textBox48->Text = "" + dvec.data[7];
	}
	else
	{
		textBox54->Text = "" + md.emgData(0);
		textBox51->Text = "" + md.emgData(1);
		textBox52->Text = "" + md.emgData(2);
		textBox49->Text = "" + md.emgData(3);
		textBox53->Text = "" + md.emgData(4);
		textBox15->Text = "" + md.emgData(5);
		textBox50->Text = "" + md.emgData(6);
		textBox48->Text = "" + md.emgData(7);
	}
}

void Myo_app::MyForm::printIMUText() {
	//Myo 1
	myo::Quaternion<float> orientation = md.orientation();
	//multiply, noncommutative
	orientation = (myo::Quaternion<float>(myoInitOrientation[0], myoInitOrientation[1], myoInitOrientation[2], myoInitOrientation[3])) * orientation;
	myo::Vector3<float> acceleration = md.acceleration();
	myo::Vector3<float> gyroscope = md.gyroscope();
	//ori
	textBoxOrientRoll->Text = "" + MyoData::roll(orientation) * 180 / Math::PI;
	textBoxOrientPitch->Text = "" + MyoData::pitch(orientation) * 180 / Math::PI;
	textBoxOrientYaw->Text = "" + MyoData::yaw(orientation) * 180 / Math::PI;
	//accel
	textBoxAccelX->Text = "" + acceleration.x();
	textBoxAccelY->Text = "" + acceleration.y();
	textBoxAccelZ->Text = "" + acceleration.z();
	//gyro
	textBoxGyroX->Text = "" + gyroscope.x();
	textBoxGyroY->Text = "" + gyroscope.y();
	textBoxGyroZ->Text = "" + gyroscope.z();
}

void Myo_app::MyForm::drawIMUDisplays()
{
	myo::Quaternion<float> orientation = md.orientation();
	//multiply, noncommutative
	orientation = (myo::Quaternion<float>(myoInitOrientation[0], myoInitOrientation[1], myoInitOrientation[2], myoInitOrientation[3])) * orientation;
	myo::Vector3<float> acceleration = md.acceleration();
	myo::Vector3<float> gyroscope = md.gyroscope();
	InertialNavDisplay::drawAttitude(pictureBoxAttitude, MyoData::pitch(orientation), -MyoData::roll(orientation));
	InertialNavDisplay::drawHeading(pictureBoxHeading, -MyoData::yaw(orientation));
	InertialNavDisplay::drawAccelerationXY(pictureBoxAccelXY, -acceleration.y(), -acceleration.z());
	InertialNavDisplay::drawAccelerationZVert(pictureBoxAccelZ, acceleration.x());
}

void Myo_app::MyForm::updateStatus() {
	progressBarMyo1Battery->Value = Myo_app::md.getBatteryLevel();

	//if (myoStreaming) {
	//	textBox33->Text = "Myo";
	//}
	//else if (playbackLoop) {
	//	textBox33->Text = "Text File";
	//}
	//else {
	//	textBox33->Text = "No Data Source";
	//}

	//// status
	//if (endtimer1) {
	//	textBox34->Text = "Paused";
	//}
	//else if (!endtimer1 && myoStreaming) {
	//	textBox34->Text = "Streaming";
	//}
	//else if (checkBox6->Checked && playbackLoop) {
	//	textBox34->Text = "Looping";
	//}
	//else if (playbackLoop && !checkBox6->Checked) {
	//	textBox34->Text = "Playing";
	//}

	//// error
	//if (playbackLoop && myoStreaming) {
	//	textBox33->Text = "ERROR";
	//}
}

//becca functions
void Myo_app::MyForm::lda_classify_train()
{
	/*
	insert becca's code here

	format:

	comment
	syntax
	max index

	access the training data in the array:
	test20.collector2.LDATrainer[maxClasses][maxWindowNumber][maxFeatures]
	LDATrainer[30][1024][64]

	access number of samples per class in training array:
	test20.collector2.numWC[test20.collector2.currentTotalClasses]
	numWC[currentTotalClasses] - changes depending how mnay gestures you add

	access current live data in array:
	test20.collector2.FeatEMG[FtNm][numSensors]
	FeatEMG[4][8]

	*/

	// save gesture names
}

void Myo_app::MyForm::lda_classify_predict() {
	// becca lda predict function

	/*float FeatureVectorLive[maxFeatures];

	//creating current feature input file to fead the predictor
	//LDAPredictInputFileName = folderBrowserDialog2->SelectedPath + "\\input.txt";
	//stdLDAPredictInputFileName = context.marshal_as<std::string>(LDAPredictInputFileName);
	//fileLDAPredictInput.open(stdLDAPredictInputFileName);

	int currentFeatNum = 0;

	for (int i = 0; i < FtNm; i++) {

		for (int j = 0; j < numSensors; j++) {

			//FeatureVectorLive[currentFeatNum] = deviceMyo.collector.FeatEMG[i][j];

			currentFeatNum++;
			// Sensor1FeaT1, Sens2Feat1, SEns3Feat1, ... // Sensor1Feat2, Sens2Feat2..etc
		}

	}

	int x = 0;

	x = lda.predict(FeatureVectorLive);
	String^ DispString = gcnew String(gestureNames[x].c_str());
	textBoxClassifierDecision->Text = DispString;

	*/
}

void Myo_app::MyForm::checkPipeClientAcknowledge(unsigned char ack) {
	switch (ack) {
	case 0xB0:
		// short vibrate
		if (md.myo)
			md.myo->vibrate(myo::Myo::vibrationShort);
		break;
	case 0xB1:
		// medium vibrate
		if (md.myo)
			md.myo->vibrate(myo::Myo::vibrationMedium);
		break;
	case 0xB2:
		// long vibrate
		if (md.myo)
			md.myo->vibrate(myo::Myo::vibrationLong);
		break;
	default:
		break;
	}
}

void Myo_app::MyForm::updatePipe() {

	/*if (pipelineControlScheme == 0) {
		nps->changeBit(cl->getLatestPrediction(), 1); // Set the bit for the latest prediction
	}
	else {
		//if (cl->getLatestPrediction() != prevPrediction)
			//nps->changeBit(cl->getLatestPrediction(), 1); // Set the bit for the latest prediction
	}*/
	nps->sendValue(cl->getLatestPrediction(), fc.getFeatureAverage(0));
	labelPipeClientAcknowledge->Text = L"" + nps->getAcknowledge();
	checkPipeClientAcknowledge(nps->getAcknowledge());

	nps->flushByte(); // reset byte to 0x00
	prevPrediction = cl->getLatestPrediction(); // Update the previous prediction from the classifier
}

[STAThread]
int main(array<String^>^ args)
{
	bool mutexOwned;
	System::Threading::Mutex mutex(true, "HMI_GUI_APP", mutexOwned);
	if (!mutexOwned)
	{
		MessageBox::Show("An instance of the program is already running.",
			"Error", MessageBoxButtons::OK,
			MessageBoxIcon::Error);
		return 0;
	}

	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Myo_app::MyForm form;
	Application::Run(%form);

	mutex.ReleaseMutex();

	return 0;
}

