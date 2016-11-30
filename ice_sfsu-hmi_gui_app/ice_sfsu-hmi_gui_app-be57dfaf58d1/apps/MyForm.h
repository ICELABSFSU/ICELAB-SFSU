#pragma once
#include "INCLUSIONS.h"
#include "DataAgent.h"
#include "FeatureCalculator.h"
#include "MyoData.h"
#include "DataRecorder.h"
#include "ClassifyTrainer.h"
#include "SVMAgent.h"
#include "LDA.h"
#include "InertialNavDisplay.h"
#include "GestureOutput.h"
#include "SetJoyOutputDialog.h"
#include "SetKeyOutputDialog.h"
#include "SetBitOutputDialog.h"
#include "DataBuffer.h"
#include "DataReader.h"
#include "NamedPipeServer.h"
#include <map>

#define input 1
#define imu 2
#define emg 3
#define feature 4 //4
#define classifier 5
#define output 6
#define thickness 4

namespace Myo_app{

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;
	using namespace System::Text;
	using namespace System::Windows::Forms::DataVisualization::Charting;
	using namespace System::Threading;


	MyoData md;
	DataRecorder dr;
	DataReader reader;
	FeatureCalculator fc;
	ClassifyTrainer ct;
	Classifier* cl;
	SVMAgent svm;
	LDA lda;
	GestureOutput go;
	DataBuffer dbuf;
	AccuracyRecorder acr;
	SwingFinder sf;
	MMAVPkFinder mpf;
	SSorTransition sst;

	int swingcount = 0;

	enum DataSource { none, myo, file };
	bool featSelectGridLoaded = false;


	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();


			chartEMG[0] = chart1;
			chartEMG[1] = chart2;
			// EMG tab
			init_EMG_charts();
			// classifier tab
			init_mmav_chart();
			init_or_chart();
			init_d2mmav_chart();
			init_windowedSTD_chart();

			inputAxesInit();
			inputAxesFill();
			outputAxesInit();
			outputAxesFill();
			updateComboBoxGestureOutput();
			updateComboBoxNextGesture();
			nextGestureDelay = gcnew System::Windows::Forms::Timer();
			nextGestureDelay->Tick += gcnew System::EventHandler(this, &MyForm::nextGestureDelay_Tick);
			nextTrainGestureDelay = gcnew System::Windows::Forms::Timer();
			nextTrainGestureDelay->Tick += gcnew System::EventHandler(this, &MyForm::nextTrainGestureDelay_Tick);

			//initial combobox selections
			this->comboBoxSVMKernel->SelectedIndex = 0;
			this->comboBoxClassifierType->SelectedIndex = 0;

			String^ defaultSaveDirectory =
				System::Environment::GetEnvironmentVariable("userprofile") + "\\Desktop";
			this->folderBrowserDialogRecording->SelectedPath = defaultSaveDirectory;
			this->folderBrowserDialogRecordDecisionData->SelectedPath = defaultSaveDirectory;
            
			nps = gcnew NamedPipeServer();
			timer1->Start();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (md.myo)
			{
				md.myo->setStreamEmg(myo::Myo::streamEmgDisabled);
				md.myo = nullptr;	//signal myoThread to close
				myoThread->Join();
			}
			if (hub)
			{
				delete hub;
				hub = nullptr;
			}
			if (classifyFileData)
			{
				delete classifyFileData;
				classifyFileData = nullptr;
			}
			if (inputAxes) {
				delete inputAxes;
				inputAxes = nullptr;
			}
			if (outputAxes) {
				delete outputAxes;
				outputAxes = nullptr;
			}
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::Timer^  RecordTimer;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialogPlayback;

	private: System::Windows::Forms::SaveFileDialog^  saveFileDialog1;



	private: System::Windows::Forms::TabPage^  tabPageInput;

	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelInput;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelInputCenter;

	private: System::Windows::Forms::GroupBox^  groupBoxInputMyo;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelInputMyo;
	private: System::Windows::Forms::GroupBox^  groupBoxMyo1;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelMyo1;
	private: System::Windows::Forms::Label^  labelMyo1ConnectionStatus;
	private: System::Windows::Forms::Label^  labelMyo1StreamingStatus;
	private: System::Windows::Forms::Button^  buttonMyo1Connect;
	private: System::Windows::Forms::Button^  buttonMyo1Stream;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelMyo1Controls;
	private: System::Windows::Forms::Button^  buttonMyo1Center;
	private: System::Windows::Forms::Button^  buttonMyo1Vibrate;
	private: System::Windows::Forms::Panel^  panelMyo1Battery;
	private: System::Windows::Forms::Label^  labelMyo1Battery;
	private: System::Windows::Forms::ProgressBar^  progressBarMyo1Battery;
	private: System::Windows::Forms::GroupBox^  groupBoxMyo2;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelMyo2;
	private: System::Windows::Forms::Label^  labelMyo2ConnectionStatus;
	private: System::Windows::Forms::Label^  labelMyo2StreamingStatus;
	private: System::Windows::Forms::Button^  buttonMyo2Connect;
	private: System::Windows::Forms::Button^  buttonMyo2Stream;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelMyo2Controls;
	private: System::Windows::Forms::Button^  buttonMyo2Center;
	private: System::Windows::Forms::Button^  buttonMyo2Vibrate;
	private: System::Windows::Forms::Panel^  panelMyo2Battery;
	private: System::Windows::Forms::Label^  labelMyo2Battery;
	private: System::Windows::Forms::ProgressBar^  progressBarMyo2Battery;

	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelInputData;
	private: System::Windows::Forms::GroupBox^  groupBoxPlayback;
	private: System::Windows::Forms::Button^  buttonPlaybackLoad;
	private: System::Windows::Forms::TextBox^  textBoxPlaybackFilename;
	private: System::Windows::Forms::ProgressBar^  progressBarPlayback;
	private: System::Windows::Forms::Label^  labelPlaybackTime;
	private: System::Windows::Forms::Button^  buttonPlaybackPlay;
	private: System::Windows::Forms::CheckBox^  checkBoxPlaybackRepeat;

	private: System::Windows::Forms::GroupBox^  groupBoxRecording;
	private: System::Windows::Forms::Label^  labelRecordingStatus;
	private: System::Windows::Forms::TextBox^  textBoxRecordingStatus;
	private: System::Windows::Forms::Button^  buttonRecord;
	private: System::Windows::Forms::Label^  labelRecordDataInclude;
	private: System::Windows::Forms::CheckBox^  checkBoxRecordDataIMU;
	private: System::Windows::Forms::CheckBox^  checkBoxRecordDataEMG;
	private: System::Windows::Forms::Label^  labelRecordingFilename;
	private: System::Windows::Forms::TextBox^  textBoxRecordingFilename;
	private: System::Windows::Forms::Label^  labelRecordingTimestamp;
	private: System::Windows::Forms::Button^  buttonRecordingDialog;
	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialogRecording;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownRecordingFlag;
	private: System::Windows::Forms::Button^  buttonRecordingFlag;


	private: System::Windows::Forms::TabPage^  tabPage3;


	private: System::Windows::Forms::DataVisualization::Charting::Chart^  chart1;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^  chart2;
	private: System::Windows::Forms::GroupBox^  groupBox11;
	private: System::Windows::Forms::TextBox^  textBox54;
	private: System::Windows::Forms::Label^  label148;
	private: System::Windows::Forms::TextBox^  textBox53;
	private: System::Windows::Forms::TextBox^  textBox52;
	private: System::Windows::Forms::TextBox^  textBox15;
	private: System::Windows::Forms::Label^  label147;
	private: System::Windows::Forms::TextBox^  textBox51;
	private: System::Windows::Forms::TextBox^  textBox50;
	private: System::Windows::Forms::Label^  label137;
	private: System::Windows::Forms::Label^  label146;
	private: System::Windows::Forms::Label^  label138;
	private: System::Windows::Forms::Label^  label144;
	private: System::Windows::Forms::Label^  label141;
	private: System::Windows::Forms::TextBox^  textBox49;
	private: System::Windows::Forms::Label^  label142;
	private: System::Windows::Forms::TextBox^  textBox48;
	private: System::Windows::Forms::Label^  label78;
	private: System::Windows::Forms::Label^  label27;
	private: System::Windows::Forms::TabControl^  tabControl1;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^  chart4;
	private: System::Windows::Forms::CheckBox^  checkBox5;
	private: System::Windows::Forms::Label^  label40;
	private: System::Windows::Forms::NumericUpDown^  numericUpDown1;
	private: System::Windows::Forms::GroupBox^  groupBox17;
	private: System::Windows::Forms::NumericUpDown^  numericUpDown3;
	private: System::Windows::Forms::NumericUpDown^  numericUpDown2;
	private: System::Windows::Forms::ComboBox^  comboBox6;
	private: System::Windows::Forms::ComboBox^  comboBox7;
	private: System::Windows::Forms::ComboBox^  comboBox8;
	private: System::Windows::Forms::ComboBox^  comboBox9;










	private: System::Windows::Forms::Label^  label67;
	private: System::Windows::Forms::Label^  label65;
	private: System::Windows::Forms::Label^  label66;
	private: System::Windows::Forms::Label^  label54;
	private: System::Windows::Forms::DataVisualization::Charting::Chart^  chart3;
	//private: System::Windows::Forms::CheckBox^  checkBox7;
	private: System::Windows::Forms::Label^  label68;
	private: System::Windows::Forms::NumericUpDown^  numericUpDown5;
	private: System::Windows::Forms::TextBox^  textBox14;
	private: System::Windows::Forms::TextBox^  textBox30;
	private: System::Windows::Forms::TextBox^  textBox29;
	private: System::Windows::Forms::Label^  label72;
	private: System::Windows::Forms::Label^  label71;
	private: System::Windows::Forms::Label^  label70;
	private: System::Windows::Forms::Label^  label75;
	private: System::Windows::Forms::TextBox^  textBox35;
	private: System::Windows::Forms::Label^  label76;
	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialog2;
	private: System::Windows::Forms::NumericUpDown^  numericUpDown4;
	private: System::Windows::Forms::Label^  label88;
	private: System::Windows::Forms::Button^  button34;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog3;



	private: System::Windows::Forms::TabPage^  tabPageIMU;

	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelIMU;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelIMUCenter;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelIMUText;
	private: System::Windows::Forms::GroupBox^  groupBoxIMUGyroscope;
	private: System::Windows::Forms::TextBox^  textBoxGyroX;
	private: System::Windows::Forms::TextBox^  textBoxGyroY;
	private: System::Windows::Forms::TextBox^  textBoxGyroZ;
	private: System::Windows::Forms::Label^  labelIMUGyX;
	private: System::Windows::Forms::Label^  labelIMUGyY;
	private: System::Windows::Forms::Label^  labelIMUGyZ;
	private: System::Windows::Forms::Label^  labelIMUGyXUnit;
	private: System::Windows::Forms::Label^  labelIMUGyYUnit;
	private: System::Windows::Forms::Label^  labelIMUGyZUnit;
	private: System::Windows::Forms::GroupBox^  groupBoxIMUAccelerometer;
	private: System::Windows::Forms::TextBox^  textBoxAccelX;
	private: System::Windows::Forms::TextBox^  textBoxAccelY;
	private: System::Windows::Forms::TextBox^  textBoxAccelZ;
	private: System::Windows::Forms::Label^  labelIMUAcX;
	private: System::Windows::Forms::Label^  labelIMUAcY;
	private: System::Windows::Forms::Label^  labelIMUAcZ;
	private: System::Windows::Forms::Label^  labelIMUAcXUnit;
	private: System::Windows::Forms::Label^  labelIMUAcYUnit;
	private: System::Windows::Forms::Label^  labelIMUAcZUnit;
	private: System::Windows::Forms::GroupBox^  groupBoxIMUOrientation;
	private: System::Windows::Forms::TextBox^  textBoxOrientRoll;
	private: System::Windows::Forms::TextBox^  textBoxOrientPitch;
	private: System::Windows::Forms::TextBox^  textBoxOrientYaw;
	private: System::Windows::Forms::Label^  labelIMURoll;
	private: System::Windows::Forms::Label^  labelIMUPitch;
	private: System::Windows::Forms::Label^  labelIMUYaw;
	private: System::Windows::Forms::Label^  labelIMURollUnit;
	private: System::Windows::Forms::Label^  labelIMUPitchUnit;
	private: System::Windows::Forms::Label^  labelIMUYawUnit;
	private: System::Windows::Forms::Panel^  panelAttitudeHeading;
	private: System::Windows::Forms::PictureBox^  pictureBoxHeading;
	private: System::Windows::Forms::PictureBox^  pictureBoxAttitude;
	private: System::Windows::Forms::Panel^  panelAccelerationVector;
	private: System::Windows::Forms::Label^  labelAccelPlaneX;
	private: System::Windows::Forms::Label^  labelAccelPlaneY;
	private: System::Windows::Forms::PictureBox^  pictureBoxAccelXY;
	private: System::Windows::Forms::Label^  labelAccelVert;
	private: System::Windows::Forms::PictureBox^  pictureBoxAccelZ;



	private: System::Windows::Forms::TabPage^  tabPageClassifier;

	private: System::Windows::Forms::GroupBox^  groupBoxGesture;
	private: System::Windows::Forms::DataGridView^  gestureList;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Index;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Gesture;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Samples;
	private: System::Windows::Forms::Button^  buttonAddGesture;
	private: System::Windows::Forms::Button^  buttonTrainGesture;
	private: System::Windows::Forms::Button^  buttonClearSamples;
	private: System::Windows::Forms::Button^  buttonDeleteAll;
	private: System::Windows::Forms::Button^  buttonSaveFeatures;
	private: System::Windows::Forms::Button^  buttonImportData;
	private: System::Windows::Forms::Button^  buttonSaveData;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialogImportData;
	private: System::Windows::Forms::SaveFileDialog^  saveFileDialogSaveData;

	private: System::Windows::Forms::GroupBox^  groupBoxTrain;
	private: System::Windows::Forms::Label^  labelClassifierType;
	private: System::Windows::Forms::Label^  labelSVMKernel;
	private: System::Windows::Forms::ComboBox^  comboBoxClassifierType;
	private: System::Windows::Forms::ComboBox^  comboBoxSVMKernel;
	private: System::Windows::Forms::Label^  labelSVMParamDegree;
	private: System::Windows::Forms::Label^  labelSVMParamGamma;
	private: System::Windows::Forms::Label^  labelSVMParamCoef0;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownSVMParamDegree;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownSVMParamGamma;
	private: System::Windows::Forms::NumericUpDown^  numericUpDownSVMParamCoef0;
	private: System::Windows::Forms::Button^  buttonCreateModel;
	private: System::Windows::Forms::Button^  buttonSaveModel;
	private: System::Windows::Forms::Button^  buttonLoadModel;
	private: System::Windows::Forms::SaveFileDialog^  saveFileDialogModel;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialogModel;
	private: System::Windows::Forms::Label^  labelTrainingAccuracy;
	private: System::Windows::Forms::TextBox^  textBoxTrainingAccuracy;
	private: System::Windows::Forms::DataGridView^  confusionMatrix;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  dataGridViewTextBoxColumn1;

	private: System::Windows::Forms::GroupBox^  groupBoxClassify;
	private: System::Windows::Forms::TextBox^  textBoxRecordDecisionDataFilename;
	private: System::Windows::Forms::Label^  labelRecordDecisionDataTimestamp;
	private: System::Windows::Forms::Button^  buttonRecordDecisionDataDialog;
	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialogRecordDecisionData;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelClassifySource;
	private: System::Windows::Forms::RadioButton^  radioButtonClassifyLive;
	private: System::Windows::Forms::RadioButton^  radioButtonClassifyFile;
	private: System::Windows::Forms::GroupBox^  groupBoxClassifyFile;
	private: System::Windows::Forms::Panel^  panelClassifyFile;
	private: System::Windows::Forms::Button^  buttonClassifyLoadData;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialogClassifyFile;
	private: System::Windows::Forms::Button^  buttonClassifyTestFile;
	private: System::Windows::Forms::RadioButton^  radioButtonClassifyData;
	private: System::Windows::Forms::RadioButton^  radioButtonClassifyFeatures;
	private: System::Windows::Forms::TextBox^  textBoxClassifyFile;
	private: System::Windows::Forms::GroupBox^  groupBoxClassifyLive;
	private: System::Windows::Forms::CheckBox^  checkBoxRecordDecisionData;
	private: System::Windows::Forms::Button^  buttonStartClassifier;
	private: System::Windows::Forms::TextBox^  textBoxClassifierDecision;

	private: System::Windows::Forms::GroupBox^  groupBoxRealtimeTesting;
	private: System::Windows::Forms::Label^  labelNextGesture;
	private: System::Windows::Forms::ComboBox^  comboBoxNextGesture;
	private: System::Windows::Forms::Button^  buttonSetNowNextGesture;
	private: System::Windows::Forms::Button^  buttonSetDelayNextGesture;
	private: System::Windows::Forms::TextBox^  textBoxNextGestureWarning;
	private: System::Windows::Forms::ProgressBar^  progressBarNextGestureWarning;
	private: System::Windows::Forms::Label^  labelCurrentFlag;
	private: System::Windows::Forms::TextBox^  textBoxCurrentFlag;



	private: System::Windows::Forms::TabPage^  tabPageOutput;

	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanelOutput;
	private: System::Windows::Forms::FlowLayoutPanel^  flowLayoutPanelOutput;

	private: System::Windows::Forms::GroupBox^  groupBoxButtonsKeys;
	private: System::Windows::Forms::Label^  labelOutputGestureSelect;
	private: System::Windows::Forms::ComboBox^  comboBoxOutputGestureSelect;
	private: System::Windows::Forms::Button^  buttonKeyAssignment;
	private: System::Windows::Forms::Button^  buttonJoystickAssignment;
	private: System::Windows::Forms::Button^  buttonClearAssignment;

	private: System::Windows::Forms::GroupBox^  groupBoxAxes;
	private: System::Windows::Forms::Label^  labelInputAxis;
	private: System::Windows::Forms::Label^  labelOutputAxis;
	private: System::Windows::Forms::Label^  labelInputAxisUpper;
	private: System::Windows::Forms::Label^  labelInputAxisLower;
	private: System::Windows::Forms::ComboBox^  comboBoxInputAxis;
	private: System::Windows::Forms::ComboBox^  comboBoxOutputAxis;
	private: System::Windows::Forms::TextBox^  textBoxInputAxisUpper;
	private: System::Windows::Forms::TextBox^  textBoxInputAxisLower;
	private: System::Windows::Forms::CheckBox^  checkBoxOutputAxisInvert;
	private: System::Windows::Forms::Button^  buttonBindAxis;
	private: System::Windows::Forms::Button^  buttonClearAxis;

	private: System::Windows::Forms::Panel^  panelSettings;
	private: System::Windows::Forms::Button^  buttonOutputClearAll;
	private: System::Windows::Forms::Button^  buttonOuputLoad;
	private: System::Windows::Forms::Button^  buttonOutputSave;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialogOutput;
	private: System::Windows::Forms::SaveFileDialog^  saveFileDialogOutput;

	private: System::Windows::Forms::GroupBox^  groupBoxOutput;
	private: System::Windows::Forms::Label^  labelMyoMouse;
	private: System::Windows::Forms::Label^  labelMyoKeyboard;
	private: System::Windows::Forms::Label^  labelMyoJoystick;
	private: System::Windows::Forms::CheckBox^  checkBoxMyoMouse;
	private: System::Windows::Forms::CheckBox^  checkBoxMyoKeyboard;
	private: System::Windows::Forms::CheckBox^  checkBoxMyoJoystick;



	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabelDataSource;
	private: System::Windows::Forms::ToolStripProgressBar^  toolStripProgressBarDataSourceStatus;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabelDataSourceStatus;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabelSeparator;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusLabelStatus;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::IContainer^  components;


		myo::Hub* hub;


		int tabSelected;

		time_t currentTime;
		struct tm *localTime;

		DateTime timeRecordStart;

		System::Windows::Forms::Timer^ nextGestureDelay;
		System::Windows::Forms::Timer^ nextTrainGestureDelay;

		Thread^ myoThread;
private: System::Windows::Forms::Button^  buttonSaveGestures;
private: System::Windows::Forms::Button^  buttonLoadGestures;
		 System::Windows::Forms::PictureBox^  pictureBoxGesture;
private: System::Windows::Forms::GroupBox^  groupBox1;
private: System::Windows::Forms::CheckBox^  checkBoxAutoRun;
private: System::Windows::Forms::ProgressBar^  progressBarTrainNextGesture;
private: System::Windows::Forms::TextBox^  textBoxAutoRunSeconds;

private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::TextBox^  textBoxTrainNextGestureWarning;
private: System::Windows::Forms::TabPage^  tabPage4;
private: System::Windows::Forms::GroupBox^  groupBoxSSorTrans;
private: System::Windows::Forms::Label^  label11;
private: System::Windows::Forms::NumericUpDown^  numericUpDownSSorTransWinsize;




private: System::Windows::Forms::Label^  label10;
private: System::Windows::Forms::NumericUpDown^  numericUpDownSSorTransThresh;
private: System::Windows::Forms::DataVisualization::Charting::Chart^  chartSTD;
private: System::Windows::Forms::GroupBox^  groupBox3;


private: System::Windows::Forms::Label^  label7;

private: System::Windows::Forms::NumericUpDown^  numericUpDownMMAVThresh;

private: System::Windows::Forms::DataVisualization::Charting::Chart^  chartD2MMAV;
private: System::Windows::Forms::GroupBox^  groupBoxOrPitch;
private: System::Windows::Forms::Label^  label5;
private: System::Windows::Forms::Label^  label4;
private: System::Windows::Forms::Label^  label3;
private: System::Windows::Forms::NumericUpDown^  numericUpDownSCValuesInAvg;
private: System::Windows::Forms::NumericUpDown^  numericUpDownSCThresh;
private: System::Windows::Forms::TextBox^  textBoxSwingCount;
private: System::Windows::Forms::DataVisualization::Charting::Chart^  chartOrPitch;
private: System::Windows::Forms::GroupBox^  groupBox2;
private: System::Windows::Forms::CheckBox^  checkBoxAcc;
private: System::Windows::Forms::CheckBox^  checkBoxGyro;
private: System::Windows::Forms::CheckBox^  checkBoxOrientation;
private: System::Windows::Forms::GroupBox^  groupBoxFeatureSelect;
private: System::Windows::Forms::Label^  label2;
private: System::Windows::Forms::DataGridView^  dataGridFeatureSelect;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col0;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col1;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col2;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col3;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col4;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col5;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col6;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col7;
private: System::Windows::Forms::DataGridViewCheckBoxColumn^  col8;
private: System::Windows::Forms::Label^  label21;
private: System::Windows::Forms::RadioButton^  radioButtonTransition;
private: System::Windows::Forms::RadioButton^  radioButtonMMAVPk;


private: System::Windows::Forms::Label^  labelPipeClientAcknowledge;
private: System::Windows::Forms::Button^  buttonBitAssignment;
private: System::Windows::Forms::Button^  buttonLoadVR;
private: System::Windows::Forms::GroupBox^  groupBoxPipelineControlOptions;
private: System::Windows::Forms::RadioButton^  radioButtonContinuous;
private: System::Windows::Forms::RadioButton^  radioButtonOnTransitionAndPeak;
private: System::Windows::Forms::TextBox^  textBoxAli;
private: System::Windows::Forms::Button^  buttonAli;











		 Thread^ fileThread;
		void runMyo();
		void playback();
		int currentSource = DataSource::none;
		void startStreamingMyo();
		void stopStreamingMyo();
		void startFilePlayback();
		void stopFilePlayback();

		bool endtimer1 = false;

		int rrate = 50; // default refresh rate of hub.run

		bool classify = false;
		bool training = false;
		bool recordingRawData = false;

		bool myo1Streaming = false;

		short indexOfPreviousClass = -1;

		bool readyToTrainGesture = true;
		bool cancelAutoTrain = false;

		void checkPipeClientAcknowledge(unsigned char ack);
		void updatePipe();
		int pipelineControlSceme = 0; // 0: continuous (default), 1: on transition

		InertialNavDisplay navdisplay;
		//conjugate quaternion of centered orientation
		array<float>^ myoInitOrientation = gcnew array<float>(4);

		//float acc = 0;

		void centerMyo(MyoData & md);
		std::map<std::string, int>* inputAxes;
		void inputAxesInit();
		void inputAxesFill();
		std::map<std::string, int>* outputAxes;
		void outputAxesInit();
		void outputAxesFill();
		void updateComboBoxGestureOutput();
		void updateComboBoxNextGesture();

		inline std::string timestamp() {
			time_t now = time(0);
			struct tm * tm = localtime(&now);
			char timestamp[80] = { 0 };
			sprintf_s(timestamp, 80, ".%02d-%02d-%02d.%02d%02d.%02d",
				tm->tm_year - 100, tm->tm_mon + 1, tm->tm_mday,
				tm->tm_hour, tm->tm_min, tm->tm_sec);
			return std::string(timestamp);
		}

		NamedPipeServer^ nps;
		
		std::vector<DataVector>* classifyFileData;

		void init_EMG_charts();

		// testing purposes - graph on classifier tab
		void init_mmav_chart(void);
		void init_or_chart(void);
		void init_d2mmav_chart(void);
		void init_windowedSTD_chart(void);

		void Display_EMG(); //EMG Display

		void playbackEMG(std::ifstream& openEMGFile);
		void playbackIMU(std::ifstream& openIMUFile);

		void Relabel_EMG_chart_area(Chart^ chart);

		void mmav_display_line();
		void orPitch_display_line();
		void d2mmav_display_line();
		void std_display_line();

		void printEMG();

		void printIMUText();
		void drawIMUDisplays();

		void updateStatus();

		//classifier functions

		//becca functions
		void lda_classify_train();

		void lda_classify_predict();

		array<Chart^, 1> ^ chartEMG = gcnew array<Chart^, 1>(4); //db for charts

		msclr::interop::marshal_context context;//used for converting to strings

		String^ PictureDirectory;
		String^ recordingFileName;

		String^ model_name;
		String^ model_A_name;
		String^ model_B_name;
		String^ training_data_name;
		String^ LDAPredictInputFileName;

#pragma region Windows Form Designer generated code
		//Every gui element attribute list, properties
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->openFileDialogPlayback = (gcnew System::Windows::Forms::OpenFileDialog());
			this->saveFileDialog1 = (gcnew System::Windows::Forms::SaveFileDialog());
			this->folderBrowserDialogRecording = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->RecordTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->tabPageOutput = (gcnew System::Windows::Forms::TabPage());
			this->tableLayoutPanelOutput = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->flowLayoutPanelOutput = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->groupBoxButtonsKeys = (gcnew System::Windows::Forms::GroupBox());
			this->groupBoxPipelineControlOptions = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonContinuous = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonOnTransitionAndPeak = (gcnew System::Windows::Forms::RadioButton());
			this->buttonLoadVR = (gcnew System::Windows::Forms::Button());
			this->buttonBitAssignment = (gcnew System::Windows::Forms::Button());
			this->labelPipeClientAcknowledge = (gcnew System::Windows::Forms::Label());
			this->labelOutputGestureSelect = (gcnew System::Windows::Forms::Label());
			this->comboBoxOutputGestureSelect = (gcnew System::Windows::Forms::ComboBox());
			this->buttonClearAssignment = (gcnew System::Windows::Forms::Button());
			this->buttonJoystickAssignment = (gcnew System::Windows::Forms::Button());
			this->buttonKeyAssignment = (gcnew System::Windows::Forms::Button());
			this->groupBoxAxes = (gcnew System::Windows::Forms::GroupBox());
			this->buttonClearAxis = (gcnew System::Windows::Forms::Button());
			this->checkBoxOutputAxisInvert = (gcnew System::Windows::Forms::CheckBox());
			this->textBoxInputAxisUpper = (gcnew System::Windows::Forms::TextBox());
			this->textBoxInputAxisLower = (gcnew System::Windows::Forms::TextBox());
			this->labelInputAxisUpper = (gcnew System::Windows::Forms::Label());
			this->labelInputAxisLower = (gcnew System::Windows::Forms::Label());
			this->labelOutputAxis = (gcnew System::Windows::Forms::Label());
			this->comboBoxOutputAxis = (gcnew System::Windows::Forms::ComboBox());
			this->buttonBindAxis = (gcnew System::Windows::Forms::Button());
			this->labelInputAxis = (gcnew System::Windows::Forms::Label());
			this->comboBoxInputAxis = (gcnew System::Windows::Forms::ComboBox());
			this->panelSettings = (gcnew System::Windows::Forms::Panel());
			this->buttonOutputSave = (gcnew System::Windows::Forms::Button());
			this->buttonOuputLoad = (gcnew System::Windows::Forms::Button());
			this->buttonOutputClearAll = (gcnew System::Windows::Forms::Button());
			this->groupBoxOutput = (gcnew System::Windows::Forms::GroupBox());
			this->checkBoxMyoJoystick = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxMyoKeyboard = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxMyoMouse = (gcnew System::Windows::Forms::CheckBox());
			this->labelMyoKeyboard = (gcnew System::Windows::Forms::Label());
			this->labelMyoMouse = (gcnew System::Windows::Forms::Label());
			this->labelMyoJoystick = (gcnew System::Windows::Forms::Label());
			this->tabPageClassifier = (gcnew System::Windows::Forms::TabPage());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->textBoxTrainNextGestureWarning = (gcnew System::Windows::Forms::TextBox());
			this->progressBarTrainNextGesture = (gcnew System::Windows::Forms::ProgressBar());
			this->pictureBoxGesture = (gcnew System::Windows::Forms::PictureBox());
			this->groupBoxClassify = (gcnew System::Windows::Forms::GroupBox());
			this->groupBoxClassifyLive = (gcnew System::Windows::Forms::GroupBox());
			this->buttonStartClassifier = (gcnew System::Windows::Forms::Button());
			this->textBoxClassifierDecision = (gcnew System::Windows::Forms::TextBox());
			this->checkBoxRecordDecisionData = (gcnew System::Windows::Forms::CheckBox());
			this->groupBoxClassifyFile = (gcnew System::Windows::Forms::GroupBox());
			this->panelClassifyFile = (gcnew System::Windows::Forms::Panel());
			this->textBoxClassifyFile = (gcnew System::Windows::Forms::TextBox());
			this->buttonClassifyLoadData = (gcnew System::Windows::Forms::Button());
			this->buttonClassifyTestFile = (gcnew System::Windows::Forms::Button());
			this->radioButtonClassifyData = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonClassifyFeatures = (gcnew System::Windows::Forms::RadioButton());
			this->flowLayoutPanelClassifySource = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->radioButtonClassifyLive = (gcnew System::Windows::Forms::RadioButton());
			this->radioButtonClassifyFile = (gcnew System::Windows::Forms::RadioButton());
			this->buttonRecordDecisionDataDialog = (gcnew System::Windows::Forms::Button());
			this->textBoxRecordDecisionDataFilename = (gcnew System::Windows::Forms::TextBox());
			this->labelRecordDecisionDataTimestamp = (gcnew System::Windows::Forms::Label());
			this->groupBoxTrain = (gcnew System::Windows::Forms::GroupBox());
			this->confusionMatrix = (gcnew System::Windows::Forms::DataGridView());
			this->dataGridViewTextBoxColumn1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->numericUpDownSVMParamCoef0 = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDownSVMParamGamma = (gcnew System::Windows::Forms::NumericUpDown());
			this->labelSVMParamCoef0 = (gcnew System::Windows::Forms::Label());
			this->labelSVMParamGamma = (gcnew System::Windows::Forms::Label());
			this->labelSVMParamDegree = (gcnew System::Windows::Forms::Label());
			this->numericUpDownSVMParamDegree = (gcnew System::Windows::Forms::NumericUpDown());
			this->labelSVMKernel = (gcnew System::Windows::Forms::Label());
			this->labelClassifierType = (gcnew System::Windows::Forms::Label());
			this->comboBoxSVMKernel = (gcnew System::Windows::Forms::ComboBox());
			this->comboBoxClassifierType = (gcnew System::Windows::Forms::ComboBox());
			this->buttonSaveModel = (gcnew System::Windows::Forms::Button());
			this->buttonLoadModel = (gcnew System::Windows::Forms::Button());
			this->buttonCreateModel = (gcnew System::Windows::Forms::Button());
			this->labelTrainingAccuracy = (gcnew System::Windows::Forms::Label());
			this->textBoxTrainingAccuracy = (gcnew System::Windows::Forms::TextBox());
			this->groupBoxRealtimeTesting = (gcnew System::Windows::Forms::GroupBox());
			this->textBoxNextGestureWarning = (gcnew System::Windows::Forms::TextBox());
			this->textBoxCurrentFlag = (gcnew System::Windows::Forms::TextBox());
			this->buttonSetDelayNextGesture = (gcnew System::Windows::Forms::Button());
			this->comboBoxNextGesture = (gcnew System::Windows::Forms::ComboBox());
			this->progressBarNextGestureWarning = (gcnew System::Windows::Forms::ProgressBar());
			this->buttonSetNowNextGesture = (gcnew System::Windows::Forms::Button());
			this->labelCurrentFlag = (gcnew System::Windows::Forms::Label());
			this->labelNextGesture = (gcnew System::Windows::Forms::Label());
			this->groupBoxGesture = (gcnew System::Windows::Forms::GroupBox());
			this->textBoxAutoRunSeconds = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->checkBoxAutoRun = (gcnew System::Windows::Forms::CheckBox());
			this->buttonSaveGestures = (gcnew System::Windows::Forms::Button());
			this->buttonLoadGestures = (gcnew System::Windows::Forms::Button());
			this->buttonSaveFeatures = (gcnew System::Windows::Forms::Button());
			this->buttonImportData = (gcnew System::Windows::Forms::Button());
			this->buttonClearSamples = (gcnew System::Windows::Forms::Button());
			this->gestureList = (gcnew System::Windows::Forms::DataGridView());
			this->Index = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Gesture = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Samples = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->buttonSaveData = (gcnew System::Windows::Forms::Button());
			this->button34 = (gcnew System::Windows::Forms::Button());
			this->numericUpDown4 = (gcnew System::Windows::Forms::NumericUpDown());
			this->label75 = (gcnew System::Windows::Forms::Label());
			this->textBox35 = (gcnew System::Windows::Forms::TextBox());
			this->textBox30 = (gcnew System::Windows::Forms::TextBox());
			this->textBox29 = (gcnew System::Windows::Forms::TextBox());
			this->label88 = (gcnew System::Windows::Forms::Label());
			this->label72 = (gcnew System::Windows::Forms::Label());
			this->label71 = (gcnew System::Windows::Forms::Label());
			this->label70 = (gcnew System::Windows::Forms::Label());
			this->buttonDeleteAll = (gcnew System::Windows::Forms::Button());
			this->buttonAddGesture = (gcnew System::Windows::Forms::Button());
			this->buttonTrainGesture = (gcnew System::Windows::Forms::Button());
			this->textBox14 = (gcnew System::Windows::Forms::TextBox());
			this->groupBox17 = (gcnew System::Windows::Forms::GroupBox());
			this->label76 = (gcnew System::Windows::Forms::Label());
			this->checkBox5 = (gcnew System::Windows::Forms::CheckBox());
			this->label40 = (gcnew System::Windows::Forms::Label());
			this->chart4 = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->numericUpDown1 = (gcnew System::Windows::Forms::NumericUpDown());
			this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
			this->label68 = (gcnew System::Windows::Forms::Label());
			this->numericUpDown5 = (gcnew System::Windows::Forms::NumericUpDown());
			this->chart3 = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->label67 = (gcnew System::Windows::Forms::Label());
			this->label65 = (gcnew System::Windows::Forms::Label());
			this->label66 = (gcnew System::Windows::Forms::Label());
			this->label54 = (gcnew System::Windows::Forms::Label());
			this->comboBox7 = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox9 = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox8 = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox6 = (gcnew System::Windows::Forms::ComboBox());
			this->numericUpDown3 = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDown2 = (gcnew System::Windows::Forms::NumericUpDown());
			this->chart2 = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->chart1 = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->groupBox11 = (gcnew System::Windows::Forms::GroupBox());
			this->textBox54 = (gcnew System::Windows::Forms::TextBox());
			this->label148 = (gcnew System::Windows::Forms::Label());
			this->textBox53 = (gcnew System::Windows::Forms::TextBox());
			this->textBox52 = (gcnew System::Windows::Forms::TextBox());
			this->textBox15 = (gcnew System::Windows::Forms::TextBox());
			this->label147 = (gcnew System::Windows::Forms::Label());
			this->textBox51 = (gcnew System::Windows::Forms::TextBox());
			this->textBox50 = (gcnew System::Windows::Forms::TextBox());
			this->label137 = (gcnew System::Windows::Forms::Label());
			this->label146 = (gcnew System::Windows::Forms::Label());
			this->label138 = (gcnew System::Windows::Forms::Label());
			this->label144 = (gcnew System::Windows::Forms::Label());
			this->label141 = (gcnew System::Windows::Forms::Label());
			this->textBox49 = (gcnew System::Windows::Forms::TextBox());
			this->label142 = (gcnew System::Windows::Forms::Label());
			this->textBox48 = (gcnew System::Windows::Forms::TextBox());
			this->label78 = (gcnew System::Windows::Forms::Label());
			this->label27 = (gcnew System::Windows::Forms::Label());
			this->tabPageIMU = (gcnew System::Windows::Forms::TabPage());
			this->tableLayoutPanelIMU = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->tableLayoutPanelIMUCenter = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->tableLayoutPanelIMUText = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->groupBoxIMUGyroscope = (gcnew System::Windows::Forms::GroupBox());
			this->labelIMUGyZUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUGyYUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUGyXUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUGyZ = (gcnew System::Windows::Forms::Label());
			this->textBoxGyroX = (gcnew System::Windows::Forms::TextBox());
			this->textBoxGyroY = (gcnew System::Windows::Forms::TextBox());
			this->labelIMUGyX = (gcnew System::Windows::Forms::Label());
			this->labelIMUGyY = (gcnew System::Windows::Forms::Label());
			this->textBoxGyroZ = (gcnew System::Windows::Forms::TextBox());
			this->groupBoxIMUAccelerometer = (gcnew System::Windows::Forms::GroupBox());
			this->labelIMUAcZUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUAcYUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUAcXUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUAcZ = (gcnew System::Windows::Forms::Label());
			this->labelIMUAcX = (gcnew System::Windows::Forms::Label());
			this->textBoxAccelZ = (gcnew System::Windows::Forms::TextBox());
			this->textBoxAccelX = (gcnew System::Windows::Forms::TextBox());
			this->labelIMUAcY = (gcnew System::Windows::Forms::Label());
			this->textBoxAccelY = (gcnew System::Windows::Forms::TextBox());
			this->groupBoxIMUOrientation = (gcnew System::Windows::Forms::GroupBox());
			this->labelIMUYawUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUPitchUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMURollUnit = (gcnew System::Windows::Forms::Label());
			this->labelIMUYaw = (gcnew System::Windows::Forms::Label());
			this->textBoxOrientYaw = (gcnew System::Windows::Forms::TextBox());
			this->labelIMUPitch = (gcnew System::Windows::Forms::Label());
			this->textBoxOrientPitch = (gcnew System::Windows::Forms::TextBox());
			this->labelIMURoll = (gcnew System::Windows::Forms::Label());
			this->textBoxOrientRoll = (gcnew System::Windows::Forms::TextBox());
			this->panelAccelerationVector = (gcnew System::Windows::Forms::Panel());
			this->pictureBoxAccelXY = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBoxAccelZ = (gcnew System::Windows::Forms::PictureBox());
			this->labelAccelVert = (gcnew System::Windows::Forms::Label());
			this->labelAccelPlaneX = (gcnew System::Windows::Forms::Label());
			this->labelAccelPlaneY = (gcnew System::Windows::Forms::Label());
			this->panelAttitudeHeading = (gcnew System::Windows::Forms::Panel());
			this->pictureBoxHeading = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBoxAttitude = (gcnew System::Windows::Forms::PictureBox());
			this->tabPageInput = (gcnew System::Windows::Forms::TabPage());
			this->tableLayoutPanelInput = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->flowLayoutPanelInputCenter = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->groupBoxInputMyo = (gcnew System::Windows::Forms::GroupBox());
			this->tableLayoutPanelInputMyo = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->groupBoxMyo2 = (gcnew System::Windows::Forms::GroupBox());
			this->flowLayoutPanelMyo2 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->labelMyo2ConnectionStatus = (gcnew System::Windows::Forms::Label());
			this->buttonMyo2Connect = (gcnew System::Windows::Forms::Button());
			this->labelMyo2StreamingStatus = (gcnew System::Windows::Forms::Label());
			this->buttonMyo2Stream = (gcnew System::Windows::Forms::Button());
			this->tableLayoutPanelMyo2Controls = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->buttonMyo2Vibrate = (gcnew System::Windows::Forms::Button());
			this->buttonMyo2Center = (gcnew System::Windows::Forms::Button());
			this->panelMyo2Battery = (gcnew System::Windows::Forms::Panel());
			this->progressBarMyo2Battery = (gcnew System::Windows::Forms::ProgressBar());
			this->labelMyo2Battery = (gcnew System::Windows::Forms::Label());
			this->groupBoxMyo1 = (gcnew System::Windows::Forms::GroupBox());
			this->flowLayoutPanelMyo1 = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->labelMyo1ConnectionStatus = (gcnew System::Windows::Forms::Label());
			this->buttonMyo1Connect = (gcnew System::Windows::Forms::Button());
			this->labelMyo1StreamingStatus = (gcnew System::Windows::Forms::Label());
			this->buttonMyo1Stream = (gcnew System::Windows::Forms::Button());
			this->tableLayoutPanelMyo1Controls = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->buttonMyo1Vibrate = (gcnew System::Windows::Forms::Button());
			this->buttonMyo1Center = (gcnew System::Windows::Forms::Button());
			this->panelMyo1Battery = (gcnew System::Windows::Forms::Panel());
			this->progressBarMyo1Battery = (gcnew System::Windows::Forms::ProgressBar());
			this->labelMyo1Battery = (gcnew System::Windows::Forms::Label());
			this->flowLayoutPanelInputData = (gcnew System::Windows::Forms::FlowLayoutPanel());
			this->groupBoxPlayback = (gcnew System::Windows::Forms::GroupBox());
			this->labelPlaybackTime = (gcnew System::Windows::Forms::Label());
			this->progressBarPlayback = (gcnew System::Windows::Forms::ProgressBar());
			this->checkBoxPlaybackRepeat = (gcnew System::Windows::Forms::CheckBox());
			this->buttonPlaybackLoad = (gcnew System::Windows::Forms::Button());
			this->textBoxPlaybackFilename = (gcnew System::Windows::Forms::TextBox());
			this->buttonPlaybackPlay = (gcnew System::Windows::Forms::Button());
			this->groupBoxRecording = (gcnew System::Windows::Forms::GroupBox());
			this->labelRecordDataInclude = (gcnew System::Windows::Forms::Label());
			this->checkBoxRecordDataEMG = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxRecordDataIMU = (gcnew System::Windows::Forms::CheckBox());
			this->numericUpDownRecordingFlag = (gcnew System::Windows::Forms::NumericUpDown());
			this->labelRecordingTimestamp = (gcnew System::Windows::Forms::Label());
			this->buttonRecordingDialog = (gcnew System::Windows::Forms::Button());
			this->buttonRecordingFlag = (gcnew System::Windows::Forms::Button());
			this->labelRecordingFilename = (gcnew System::Windows::Forms::Label());
			this->textBoxRecordingFilename = (gcnew System::Windows::Forms::TextBox());
			this->buttonRecord = (gcnew System::Windows::Forms::Button());
			this->labelRecordingStatus = (gcnew System::Windows::Forms::Label());
			this->textBoxRecordingStatus = (gcnew System::Windows::Forms::TextBox());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage4 = (gcnew System::Windows::Forms::TabPage());
			this->groupBoxSSorTrans = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonTransition = (gcnew System::Windows::Forms::RadioButton());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownSSorTransWinsize = (gcnew System::Windows::Forms::NumericUpDown());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownSSorTransThresh = (gcnew System::Windows::Forms::NumericUpDown());
			this->chartSTD = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->radioButtonMMAVPk = (gcnew System::Windows::Forms::RadioButton());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownMMAVThresh = (gcnew System::Windows::Forms::NumericUpDown());
			this->chartD2MMAV = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->groupBoxOrPitch = (gcnew System::Windows::Forms::GroupBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->numericUpDownSCValuesInAvg = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDownSCThresh = (gcnew System::Windows::Forms::NumericUpDown());
			this->textBoxSwingCount = (gcnew System::Windows::Forms::TextBox());
			this->chartOrPitch = (gcnew System::Windows::Forms::DataVisualization::Charting::Chart());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->checkBoxAcc = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxGyro = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxOrientation = (gcnew System::Windows::Forms::CheckBox());
			this->groupBoxFeatureSelect = (gcnew System::Windows::Forms::GroupBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->dataGridFeatureSelect = (gcnew System::Windows::Forms::DataGridView());
			this->col0 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col1 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col2 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col3 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col4 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col5 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col6 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col7 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->col8 = (gcnew System::Windows::Forms::DataGridViewCheckBoxColumn());
			this->label21 = (gcnew System::Windows::Forms::Label());
			this->folderBrowserDialog2 = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->openFileDialog3 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->saveFileDialogModel = (gcnew System::Windows::Forms::SaveFileDialog());
			this->openFileDialogModel = (gcnew System::Windows::Forms::OpenFileDialog());
			this->openFileDialogOutput = (gcnew System::Windows::Forms::OpenFileDialog());
			this->saveFileDialogOutput = (gcnew System::Windows::Forms::SaveFileDialog());
			this->saveFileDialogSaveData = (gcnew System::Windows::Forms::SaveFileDialog());
			this->openFileDialogImportData = (gcnew System::Windows::Forms::OpenFileDialog());
			this->folderBrowserDialogRecordDecisionData = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->openFileDialogClassifyFile = (gcnew System::Windows::Forms::OpenFileDialog());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusLabelDataSource = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripProgressBarDataSourceStatus = (gcnew System::Windows::Forms::ToolStripProgressBar());
			this->toolStripStatusLabelDataSourceStatus = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabelSeparator = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusLabelStatus = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->buttonAli = (gcnew System::Windows::Forms::Button());
			this->textBoxAli = (gcnew System::Windows::Forms::TextBox());
			this->tabPageOutput->SuspendLayout();
			this->tableLayoutPanelOutput->SuspendLayout();
			this->flowLayoutPanelOutput->SuspendLayout();
			this->groupBoxButtonsKeys->SuspendLayout();
			this->groupBoxPipelineControlOptions->SuspendLayout();
			this->groupBoxAxes->SuspendLayout();
			this->panelSettings->SuspendLayout();
			this->groupBoxOutput->SuspendLayout();
			this->tabPageClassifier->SuspendLayout();
			this->groupBox1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxGesture))->BeginInit();
			this->groupBoxClassify->SuspendLayout();
			this->groupBoxClassifyLive->SuspendLayout();
			this->groupBoxClassifyFile->SuspendLayout();
			this->panelClassifyFile->SuspendLayout();
			this->flowLayoutPanelClassifySource->SuspendLayout();
			this->groupBoxTrain->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->confusionMatrix))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamCoef0))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamGamma))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamDegree))->BeginInit();
			this->groupBoxRealtimeTesting->SuspendLayout();
			this->groupBoxGesture->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->gestureList))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown4))->BeginInit();
			this->groupBox17->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart4))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown1))->BeginInit();
			this->tabPage3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown5))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart3))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown3))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart1))->BeginInit();
			this->groupBox11->SuspendLayout();
			this->tabPageIMU->SuspendLayout();
			this->tableLayoutPanelIMU->SuspendLayout();
			this->tableLayoutPanelIMUCenter->SuspendLayout();
			this->tableLayoutPanelIMUText->SuspendLayout();
			this->groupBoxIMUGyroscope->SuspendLayout();
			this->groupBoxIMUAccelerometer->SuspendLayout();
			this->groupBoxIMUOrientation->SuspendLayout();
			this->panelAccelerationVector->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAccelXY))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAccelZ))->BeginInit();
			this->panelAttitudeHeading->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxHeading))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAttitude))->BeginInit();
			this->tabPageInput->SuspendLayout();
			this->tableLayoutPanelInput->SuspendLayout();
			this->flowLayoutPanelInputCenter->SuspendLayout();
			this->groupBoxInputMyo->SuspendLayout();
			this->tableLayoutPanelInputMyo->SuspendLayout();
			this->groupBoxMyo2->SuspendLayout();
			this->flowLayoutPanelMyo2->SuspendLayout();
			this->tableLayoutPanelMyo2Controls->SuspendLayout();
			this->panelMyo2Battery->SuspendLayout();
			this->groupBoxMyo1->SuspendLayout();
			this->flowLayoutPanelMyo1->SuspendLayout();
			this->tableLayoutPanelMyo1Controls->SuspendLayout();
			this->panelMyo1Battery->SuspendLayout();
			this->flowLayoutPanelInputData->SuspendLayout();
			this->groupBoxPlayback->SuspendLayout();
			this->groupBoxRecording->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownRecordingFlag))->BeginInit();
			this->tabControl1->SuspendLayout();
			this->tabPage4->SuspendLayout();
			this->groupBoxSSorTrans->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSSorTransWinsize))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSSorTransThresh))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartSTD))->BeginInit();
			this->groupBox3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownMMAVThresh))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartD2MMAV))->BeginInit();
			this->groupBoxOrPitch->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSCValuesInAvg))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSCThresh))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartOrPitch))->BeginInit();
			this->groupBox2->SuspendLayout();
			this->groupBoxFeatureSelect->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridFeatureSelect))->BeginInit();
			this->statusStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// timer1
			// 
			this->timer1->Interval = 20;
			this->timer1->Tick += gcnew System::EventHandler(this, &MyForm::timer1_Tick);
			// 
			// openFileDialogPlayback
			// 
			this->openFileDialogPlayback->Filter = L"Text files (*.txt)|*.txt|All files (*.*)|*.*";
			this->openFileDialogPlayback->Title = L"Load EMG Data File";
			// 
			// RecordTimer
			// 
			this->RecordTimer->Interval = 1000;
			this->RecordTimer->Tick += gcnew System::EventHandler(this, &MyForm::RecordTimer_Tick);
			// 
			// tabPageOutput
			// 
			this->tabPageOutput->Controls->Add(this->tableLayoutPanelOutput);
			this->tabPageOutput->Location = System::Drawing::Point(4, 40);
			this->tabPageOutput->Name = L"tabPageOutput";
			this->tabPageOutput->Padding = System::Windows::Forms::Padding(3);
			this->tabPageOutput->Size = System::Drawing::Size(1256, 637);
			this->tabPageOutput->TabIndex = 8;
			this->tabPageOutput->Text = L"Output";
			this->tabPageOutput->UseVisualStyleBackColor = true;
			this->tabPageOutput->Enter += gcnew System::EventHandler(this, &MyForm::tabPageOutput_Enter);
			// 
			// tableLayoutPanelOutput
			// 
			this->tableLayoutPanelOutput->ColumnCount = 1;
			this->tableLayoutPanelOutput->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelOutput->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				20)));
			this->tableLayoutPanelOutput->Controls->Add(this->flowLayoutPanelOutput, 0, 0);
			this->tableLayoutPanelOutput->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanelOutput->Location = System::Drawing::Point(3, 3);
			this->tableLayoutPanelOutput->Name = L"tableLayoutPanelOutput";
			this->tableLayoutPanelOutput->RowCount = 1;
			this->tableLayoutPanelOutput->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelOutput->Size = System::Drawing::Size(1250, 631);
			this->tableLayoutPanelOutput->TabIndex = 4;
			// 
			// flowLayoutPanelOutput
			// 
			this->flowLayoutPanelOutput->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->flowLayoutPanelOutput->Controls->Add(this->groupBoxButtonsKeys);
			this->flowLayoutPanelOutput->Controls->Add(this->groupBoxAxes);
			this->flowLayoutPanelOutput->Controls->Add(this->panelSettings);
			this->flowLayoutPanelOutput->Controls->Add(this->groupBoxOutput);
			this->flowLayoutPanelOutput->FlowDirection = System::Windows::Forms::FlowDirection::TopDown;
			this->flowLayoutPanelOutput->Location = System::Drawing::Point(193, 28);
			this->flowLayoutPanelOutput->MinimumSize = System::Drawing::Size(843, 574);
			this->flowLayoutPanelOutput->Name = L"flowLayoutPanelOutput";
			this->flowLayoutPanelOutput->Size = System::Drawing::Size(863, 575);
			this->flowLayoutPanelOutput->TabIndex = 1;
			// 
			// groupBoxButtonsKeys
			// 
			this->groupBoxButtonsKeys->Controls->Add(this->groupBoxPipelineControlOptions);
			this->groupBoxButtonsKeys->Controls->Add(this->buttonLoadVR);
			this->groupBoxButtonsKeys->Controls->Add(this->buttonBitAssignment);
			this->groupBoxButtonsKeys->Controls->Add(this->labelPipeClientAcknowledge);
			this->groupBoxButtonsKeys->Controls->Add(this->labelOutputGestureSelect);
			this->groupBoxButtonsKeys->Controls->Add(this->comboBoxOutputGestureSelect);
			this->groupBoxButtonsKeys->Controls->Add(this->buttonClearAssignment);
			this->groupBoxButtonsKeys->Controls->Add(this->buttonJoystickAssignment);
			this->groupBoxButtonsKeys->Controls->Add(this->buttonKeyAssignment);
			this->groupBoxButtonsKeys->Location = System::Drawing::Point(10, 10);
			this->groupBoxButtonsKeys->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxButtonsKeys->Name = L"groupBoxButtonsKeys";
			this->groupBoxButtonsKeys->Size = System::Drawing::Size(528, 225);
			this->groupBoxButtonsKeys->TabIndex = 0;
			this->groupBoxButtonsKeys->TabStop = false;
			this->groupBoxButtonsKeys->Text = L"Buttons / Keys";
			// 
			// groupBoxPipelineControlOptions
			// 
			this->groupBoxPipelineControlOptions->Controls->Add(this->radioButtonContinuous);
			this->groupBoxPipelineControlOptions->Controls->Add(this->radioButtonOnTransitionAndPeak);
			this->groupBoxPipelineControlOptions->Location = System::Drawing::Point(368, 116);
			this->groupBoxPipelineControlOptions->Name = L"groupBoxPipelineControlOptions";
			this->groupBoxPipelineControlOptions->Size = System::Drawing::Size(144, 64);
			this->groupBoxPipelineControlOptions->TabIndex = 10;
			this->groupBoxPipelineControlOptions->TabStop = false;
			this->groupBoxPipelineControlOptions->Text = L"Pipeline Stream Control";
			// 
			// radioButtonContinuous
			// 
			this->radioButtonContinuous->AutoSize = true;
			this->radioButtonContinuous->Checked = true;
			this->radioButtonContinuous->Location = System::Drawing::Point(6, 19);
			this->radioButtonContinuous->Name = L"radioButtonContinuous";
			this->radioButtonContinuous->Size = System::Drawing::Size(78, 17);
			this->radioButtonContinuous->TabIndex = 8;
			this->radioButtonContinuous->TabStop = true;
			this->radioButtonContinuous->Text = L"Continuous";
			this->radioButtonContinuous->UseVisualStyleBackColor = true;
			this->radioButtonContinuous->CheckedChanged += gcnew System::EventHandler(this, &MyForm::radioButtonContinuous_CheckedChanged);
			// 
			// radioButtonOnTransitionAndPeak
			// 
			this->radioButtonOnTransitionAndPeak->AutoSize = true;
			this->radioButtonOnTransitionAndPeak->Location = System::Drawing::Point(6, 42);
			this->radioButtonOnTransitionAndPeak->Name = L"radioButtonOnTransitionAndPeak";
			this->radioButtonOnTransitionAndPeak->Size = System::Drawing::Size(125, 17);
			this->radioButtonOnTransitionAndPeak->TabIndex = 9;
			this->radioButtonOnTransitionAndPeak->Text = L"On Transition + Peak";
			this->radioButtonOnTransitionAndPeak->UseVisualStyleBackColor = true;
			// 
			// buttonLoadVR
			// 
			this->buttonLoadVR->Location = System::Drawing::Point(380, 84);
			this->buttonLoadVR->Name = L"buttonLoadVR";
			this->buttonLoadVR->Size = System::Drawing::Size(120, 26);
			this->buttonLoadVR->TabIndex = 7;
			this->buttonLoadVR->Text = L"Load VR Default";
			this->buttonLoadVR->UseVisualStyleBackColor = true;
			this->buttonLoadVR->Click += gcnew System::EventHandler(this, &MyForm::buttonLoadVR_Click);
			// 
			// buttonBitAssignment
			// 
			this->buttonBitAssignment->Location = System::Drawing::Point(204, 116);
			this->buttonBitAssignment->Name = L"buttonBitAssignment";
			this->buttonBitAssignment->Size = System::Drawing::Size(140, 26);
			this->buttonBitAssignment->TabIndex = 6;
			this->buttonBitAssignment->Text = L"Bit Assignment...";
			this->buttonBitAssignment->UseVisualStyleBackColor = true;
			this->buttonBitAssignment->Click += gcnew System::EventHandler(this, &MyForm::buttonBitAssignment_Click);
			// 
			// labelPipeClientAcknowledge
			// 
			this->labelPipeClientAcknowledge->AutoSize = true;
			this->labelPipeClientAcknowledge->Location = System::Drawing::Point(470, 192);
			this->labelPipeClientAcknowledge->Name = L"labelPipeClientAcknowledge";
			this->labelPipeClientAcknowledge->Size = System::Drawing::Size(13, 13);
			this->labelPipeClientAcknowledge->TabIndex = 5;
			this->labelPipeClientAcknowledge->Text = L"0";
			// 
			// labelOutputGestureSelect
			// 
			this->labelOutputGestureSelect->AutoSize = true;
			this->labelOutputGestureSelect->Location = System::Drawing::Point(25, 40);
			this->labelOutputGestureSelect->Name = L"labelOutputGestureSelect";
			this->labelOutputGestureSelect->Size = System::Drawing::Size(77, 13);
			this->labelOutputGestureSelect->TabIndex = 4;
			this->labelOutputGestureSelect->Text = L"Select Gesture";
			// 
			// comboBoxOutputGestureSelect
			// 
			this->comboBoxOutputGestureSelect->FormattingEnabled = true;
			this->comboBoxOutputGestureSelect->Location = System::Drawing::Point(28, 56);
			this->comboBoxOutputGestureSelect->Name = L"comboBoxOutputGestureSelect";
			this->comboBoxOutputGestureSelect->Size = System::Drawing::Size(140, 21);
			this->comboBoxOutputGestureSelect->TabIndex = 3;
			// 
			// buttonClearAssignment
			// 
			this->buttonClearAssignment->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->buttonClearAssignment->Location = System::Drawing::Point(380, 52);
			this->buttonClearAssignment->Name = L"buttonClearAssignment";
			this->buttonClearAssignment->Size = System::Drawing::Size(120, 26);
			this->buttonClearAssignment->TabIndex = 2;
			this->buttonClearAssignment->Text = L"Clear Assignment";
			this->buttonClearAssignment->UseVisualStyleBackColor = true;
			this->buttonClearAssignment->Click += gcnew System::EventHandler(this, &MyForm::buttonClearAssignment_Click);
			// 
			// buttonJoystickAssignment
			// 
			this->buttonJoystickAssignment->Location = System::Drawing::Point(204, 84);
			this->buttonJoystickAssignment->Name = L"buttonJoystickAssignment";
			this->buttonJoystickAssignment->Size = System::Drawing::Size(140, 26);
			this->buttonJoystickAssignment->TabIndex = 1;
			this->buttonJoystickAssignment->Text = L"Joystick Assignment...";
			this->buttonJoystickAssignment->UseVisualStyleBackColor = true;
			this->buttonJoystickAssignment->Click += gcnew System::EventHandler(this, &MyForm::buttonJoystickAssignment_Click);
			// 
			// buttonKeyAssignment
			// 
			this->buttonKeyAssignment->Location = System::Drawing::Point(204, 52);
			this->buttonKeyAssignment->Name = L"buttonKeyAssignment";
			this->buttonKeyAssignment->Size = System::Drawing::Size(140, 26);
			this->buttonKeyAssignment->TabIndex = 0;
			this->buttonKeyAssignment->Text = L"Key Assignment...";
			this->buttonKeyAssignment->UseVisualStyleBackColor = true;
			this->buttonKeyAssignment->Click += gcnew System::EventHandler(this, &MyForm::buttonKeyAssignment_Click);
			// 
			// groupBoxAxes
			// 
			this->groupBoxAxes->Controls->Add(this->buttonClearAxis);
			this->groupBoxAxes->Controls->Add(this->checkBoxOutputAxisInvert);
			this->groupBoxAxes->Controls->Add(this->textBoxInputAxisUpper);
			this->groupBoxAxes->Controls->Add(this->textBoxInputAxisLower);
			this->groupBoxAxes->Controls->Add(this->labelInputAxisUpper);
			this->groupBoxAxes->Controls->Add(this->labelInputAxisLower);
			this->groupBoxAxes->Controls->Add(this->labelOutputAxis);
			this->groupBoxAxes->Controls->Add(this->comboBoxOutputAxis);
			this->groupBoxAxes->Controls->Add(this->buttonBindAxis);
			this->groupBoxAxes->Controls->Add(this->labelInputAxis);
			this->groupBoxAxes->Controls->Add(this->comboBoxInputAxis);
			this->groupBoxAxes->Location = System::Drawing::Point(10, 255);
			this->groupBoxAxes->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxAxes->Name = L"groupBoxAxes";
			this->groupBoxAxes->Size = System::Drawing::Size(528, 211);
			this->groupBoxAxes->TabIndex = 1;
			this->groupBoxAxes->TabStop = false;
			this->groupBoxAxes->Text = L"Axes";
			// 
			// buttonClearAxis
			// 
			this->buttonClearAxis->Location = System::Drawing::Point(380, 84);
			this->buttonClearAxis->Name = L"buttonClearAxis";
			this->buttonClearAxis->Size = System::Drawing::Size(120, 26);
			this->buttonClearAxis->TabIndex = 14;
			this->buttonClearAxis->Text = L"Clear Axis";
			this->buttonClearAxis->UseVisualStyleBackColor = true;
			this->buttonClearAxis->Click += gcnew System::EventHandler(this, &MyForm::buttonClearAxis_Click);
			// 
			// checkBoxOutputAxisInvert
			// 
			this->checkBoxOutputAxisInvert->AutoSize = true;
			this->checkBoxOutputAxisInvert->Location = System::Drawing::Point(204, 89);
			this->checkBoxOutputAxisInvert->Name = L"checkBoxOutputAxisInvert";
			this->checkBoxOutputAxisInvert->Size = System::Drawing::Size(75, 17);
			this->checkBoxOutputAxisInvert->TabIndex = 13;
			this->checkBoxOutputAxisInvert->Text = L"Invert Axis";
			this->checkBoxOutputAxisInvert->UseVisualStyleBackColor = true;
			// 
			// textBoxInputAxisUpper
			// 
			this->textBoxInputAxisUpper->Location = System::Drawing::Point(101, 86);
			this->textBoxInputAxisUpper->Name = L"textBoxInputAxisUpper";
			this->textBoxInputAxisUpper->Size = System::Drawing::Size(67, 20);
			this->textBoxInputAxisUpper->TabIndex = 12;
			// 
			// textBoxInputAxisLower
			// 
			this->textBoxInputAxisLower->Location = System::Drawing::Point(101, 112);
			this->textBoxInputAxisLower->Name = L"textBoxInputAxisLower";
			this->textBoxInputAxisLower->Size = System::Drawing::Size(67, 20);
			this->textBoxInputAxisLower->TabIndex = 11;
			// 
			// labelInputAxisUpper
			// 
			this->labelInputAxisUpper->AutoSize = true;
			this->labelInputAxisUpper->Location = System::Drawing::Point(25, 115);
			this->labelInputAxisUpper->Name = L"labelInputAxisUpper";
			this->labelInputAxisUpper->Size = System::Drawing::Size(70, 13);
			this->labelInputAxisUpper->TabIndex = 10;
			this->labelInputAxisUpper->Text = L"Upper Bound";
			// 
			// labelInputAxisLower
			// 
			this->labelInputAxisLower->AutoSize = true;
			this->labelInputAxisLower->Location = System::Drawing::Point(25, 89);
			this->labelInputAxisLower->Name = L"labelInputAxisLower";
			this->labelInputAxisLower->Size = System::Drawing::Size(70, 13);
			this->labelInputAxisLower->TabIndex = 9;
			this->labelInputAxisLower->Text = L"Lower Bound";
			// 
			// labelOutputAxis
			// 
			this->labelOutputAxis->AutoSize = true;
			this->labelOutputAxis->Location = System::Drawing::Point(201, 40);
			this->labelOutputAxis->Name = L"labelOutputAxis";
			this->labelOutputAxis->Size = System::Drawing::Size(94, 13);
			this->labelOutputAxis->TabIndex = 8;
			this->labelOutputAxis->Text = L"Select Output Axis";
			// 
			// comboBoxOutputAxis
			// 
			this->comboBoxOutputAxis->FormattingEnabled = true;
			this->comboBoxOutputAxis->Location = System::Drawing::Point(204, 56);
			this->comboBoxOutputAxis->Name = L"comboBoxOutputAxis";
			this->comboBoxOutputAxis->Size = System::Drawing::Size(121, 21);
			this->comboBoxOutputAxis->TabIndex = 7;
			// 
			// buttonBindAxis
			// 
			this->buttonBindAxis->Location = System::Drawing::Point(380, 52);
			this->buttonBindAxis->Name = L"buttonBindAxis";
			this->buttonBindAxis->Size = System::Drawing::Size(120, 26);
			this->buttonBindAxis->TabIndex = 6;
			this->buttonBindAxis->Text = L"Bind Axis";
			this->buttonBindAxis->UseVisualStyleBackColor = true;
			this->buttonBindAxis->Click += gcnew System::EventHandler(this, &MyForm::buttonBindAxis_Click);
			// 
			// labelInputAxis
			// 
			this->labelInputAxis->AutoSize = true;
			this->labelInputAxis->Location = System::Drawing::Point(25, 40);
			this->labelInputAxis->Name = L"labelInputAxis";
			this->labelInputAxis->Size = System::Drawing::Size(67, 13);
			this->labelInputAxis->TabIndex = 5;
			this->labelInputAxis->Text = L"Select Value";
			// 
			// comboBoxInputAxis
			// 
			this->comboBoxInputAxis->FormattingEnabled = true;
			this->comboBoxInputAxis->Location = System::Drawing::Point(28, 56);
			this->comboBoxInputAxis->Name = L"comboBoxInputAxis";
			this->comboBoxInputAxis->Size = System::Drawing::Size(140, 21);
			this->comboBoxInputAxis->TabIndex = 0;
			// 
			// panelSettings
			// 
			this->panelSettings->Controls->Add(this->buttonOutputSave);
			this->panelSettings->Controls->Add(this->buttonOuputLoad);
			this->panelSettings->Controls->Add(this->buttonOutputClearAll);
			this->panelSettings->Location = System::Drawing::Point(10, 486);
			this->panelSettings->Margin = System::Windows::Forms::Padding(10);
			this->panelSettings->Name = L"panelSettings";
			this->panelSettings->Size = System::Drawing::Size(528, 77);
			this->panelSettings->TabIndex = 0;
			// 
			// buttonOutputSave
			// 
			this->buttonOutputSave->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->buttonOutputSave->Location = System::Drawing::Point(380, 19);
			this->buttonOutputSave->Name = L"buttonOutputSave";
			this->buttonOutputSave->Size = System::Drawing::Size(120, 26);
			this->buttonOutputSave->TabIndex = 5;
			this->buttonOutputSave->Text = L"Save Settings...";
			this->buttonOutputSave->UseVisualStyleBackColor = true;
			this->buttonOutputSave->Click += gcnew System::EventHandler(this, &MyForm::buttonOutputSave_Click);
			// 
			// buttonOuputLoad
			// 
			this->buttonOuputLoad->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->buttonOuputLoad->Location = System::Drawing::Point(204, 19);
			this->buttonOuputLoad->Name = L"buttonOuputLoad";
			this->buttonOuputLoad->Size = System::Drawing::Size(120, 26);
			this->buttonOuputLoad->TabIndex = 4;
			this->buttonOuputLoad->Text = L"Load Settings...";
			this->buttonOuputLoad->UseVisualStyleBackColor = true;
			this->buttonOuputLoad->Click += gcnew System::EventHandler(this, &MyForm::buttonOuputLoad_Click);
			// 
			// buttonOutputClearAll
			// 
			this->buttonOutputClearAll->Location = System::Drawing::Point(28, 19);
			this->buttonOutputClearAll->Name = L"buttonOutputClearAll";
			this->buttonOutputClearAll->Size = System::Drawing::Size(120, 26);
			this->buttonOutputClearAll->TabIndex = 3;
			this->buttonOutputClearAll->Text = L"Clear All";
			this->buttonOutputClearAll->UseVisualStyleBackColor = true;
			this->buttonOutputClearAll->Click += gcnew System::EventHandler(this, &MyForm::buttonOutputClearAll_Click);
			// 
			// groupBoxOutput
			// 
			this->groupBoxOutput->Controls->Add(this->checkBoxMyoJoystick);
			this->groupBoxOutput->Controls->Add(this->checkBoxMyoKeyboard);
			this->groupBoxOutput->Controls->Add(this->checkBoxMyoMouse);
			this->groupBoxOutput->Controls->Add(this->labelMyoKeyboard);
			this->groupBoxOutput->Controls->Add(this->labelMyoMouse);
			this->groupBoxOutput->Controls->Add(this->labelMyoJoystick);
			this->groupBoxOutput->Location = System::Drawing::Point(558, 10);
			this->groupBoxOutput->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxOutput->Name = L"groupBoxOutput";
			this->groupBoxOutput->Size = System::Drawing::Size(275, 553);
			this->groupBoxOutput->TabIndex = 0;
			this->groupBoxOutput->TabStop = false;
			this->groupBoxOutput->Text = L"Output";
			// 
			// checkBoxMyoJoystick
			// 
			this->checkBoxMyoJoystick->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->checkBoxMyoJoystick->Appearance = System::Windows::Forms::Appearance::Button;
			this->checkBoxMyoJoystick->Location = System::Drawing::Point(119, 193);
			this->checkBoxMyoJoystick->Name = L"checkBoxMyoJoystick";
			this->checkBoxMyoJoystick->Size = System::Drawing::Size(120, 26);
			this->checkBoxMyoJoystick->TabIndex = 2;
			this->checkBoxMyoJoystick->Text = L"Disabled";
			this->checkBoxMyoJoystick->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->checkBoxMyoJoystick->UseVisualStyleBackColor = true;
			this->checkBoxMyoJoystick->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxMyoJoystick_CheckedChanged);
			// 
			// checkBoxMyoKeyboard
			// 
			this->checkBoxMyoKeyboard->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->checkBoxMyoKeyboard->Appearance = System::Windows::Forms::Appearance::Button;
			this->checkBoxMyoKeyboard->Location = System::Drawing::Point(119, 129);
			this->checkBoxMyoKeyboard->Name = L"checkBoxMyoKeyboard";
			this->checkBoxMyoKeyboard->Size = System::Drawing::Size(120, 26);
			this->checkBoxMyoKeyboard->TabIndex = 1;
			this->checkBoxMyoKeyboard->Text = L"Disabled";
			this->checkBoxMyoKeyboard->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->checkBoxMyoKeyboard->UseVisualStyleBackColor = true;
			this->checkBoxMyoKeyboard->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxMyoKeyboard_CheckedChanged);
			// 
			// checkBoxMyoMouse
			// 
			this->checkBoxMyoMouse->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->checkBoxMyoMouse->Appearance = System::Windows::Forms::Appearance::Button;
			this->checkBoxMyoMouse->Location = System::Drawing::Point(119, 65);
			this->checkBoxMyoMouse->Name = L"checkBoxMyoMouse";
			this->checkBoxMyoMouse->Size = System::Drawing::Size(120, 26);
			this->checkBoxMyoMouse->TabIndex = 0;
			this->checkBoxMyoMouse->Text = L"Disabled";
			this->checkBoxMyoMouse->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->checkBoxMyoMouse->UseVisualStyleBackColor = true;
			// 
			// labelMyoKeyboard
			// 
			this->labelMyoKeyboard->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->labelMyoKeyboard->Location = System::Drawing::Point(2, 129);
			this->labelMyoKeyboard->Name = L"labelMyoKeyboard";
			this->labelMyoKeyboard->Size = System::Drawing::Size(120, 26);
			this->labelMyoKeyboard->TabIndex = 4;
			this->labelMyoKeyboard->Text = L"Fake Keyboard";
			this->labelMyoKeyboard->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// labelMyoMouse
			// 
			this->labelMyoMouse->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->labelMyoMouse->Location = System::Drawing::Point(2, 65);
			this->labelMyoMouse->Name = L"labelMyoMouse";
			this->labelMyoMouse->Size = System::Drawing::Size(120, 26);
			this->labelMyoMouse->TabIndex = 3;
			this->labelMyoMouse->Text = L"Myo Mouse";
			this->labelMyoMouse->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// labelMyoJoystick
			// 
			this->labelMyoJoystick->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->labelMyoJoystick->Location = System::Drawing::Point(2, 193);
			this->labelMyoJoystick->Name = L"labelMyoJoystick";
			this->labelMyoJoystick->Size = System::Drawing::Size(120, 26);
			this->labelMyoJoystick->TabIndex = 5;
			this->labelMyoJoystick->Text = L"vJoy";
			this->labelMyoJoystick->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// tabPageClassifier
			// 
			this->tabPageClassifier->Controls->Add(this->groupBox1);
			this->tabPageClassifier->Controls->Add(this->groupBoxClassify);
			this->tabPageClassifier->Controls->Add(this->groupBoxTrain);
			this->tabPageClassifier->Controls->Add(this->groupBoxRealtimeTesting);
			this->tabPageClassifier->Controls->Add(this->groupBoxGesture);
			this->tabPageClassifier->Controls->Add(this->groupBox17);
			this->tabPageClassifier->Location = System::Drawing::Point(4, 40);
			this->tabPageClassifier->Name = L"tabPageClassifier";
			this->tabPageClassifier->Padding = System::Windows::Forms::Padding(3);
			this->tabPageClassifier->Size = System::Drawing::Size(1256, 637);
			this->tabPageClassifier->TabIndex = 5;
			this->tabPageClassifier->Text = L"Classifier";
			this->tabPageClassifier->UseVisualStyleBackColor = true;
			this->tabPageClassifier->Enter += gcnew System::EventHandler(this, &MyForm::tabPageClassifier_Enter);
			this->tabPageClassifier->Leave += gcnew System::EventHandler(this, &MyForm::tabPageClassifier_Leave);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->textBoxTrainNextGestureWarning);
			this->groupBox1->Controls->Add(this->progressBarTrainNextGesture);
			this->groupBox1->Controls->Add(this->pictureBoxGesture);
			this->groupBox1->Location = System::Drawing::Point(500, 1);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(350, 329);
			this->groupBox1->TabIndex = 48;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Visual";
			this->groupBox1->Enter += gcnew System::EventHandler(this, &MyForm::groupBox1_Enter);
			// 
			// textBoxTrainNextGestureWarning
			// 
			this->textBoxTrainNextGestureWarning->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxTrainNextGestureWarning->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBoxTrainNextGestureWarning->Font = (gcnew System::Drawing::Font(L"Lucida Sans Unicode", 15.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBoxTrainNextGestureWarning->Location = System::Drawing::Point(69, 245);
			this->textBoxTrainNextGestureWarning->Name = L"textBoxTrainNextGestureWarning";
			this->textBoxTrainNextGestureWarning->ReadOnly = true;
			this->textBoxTrainNextGestureWarning->Size = System::Drawing::Size(225, 33);
			this->textBoxTrainNextGestureWarning->TabIndex = 48;
			this->textBoxTrainNextGestureWarning->Text = L"Do gesture in 3...";
			this->textBoxTrainNextGestureWarning->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->textBoxTrainNextGestureWarning->Visible = false;
			// 
			// progressBarTrainNextGesture
			// 
			this->progressBarTrainNextGesture->Location = System::Drawing::Point(6, 299);
			this->progressBarTrainNextGesture->Maximum = 300;
			this->progressBarTrainNextGesture->Name = L"progressBarTrainNextGesture";
			this->progressBarTrainNextGesture->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->progressBarTrainNextGesture->RightToLeftLayout = true;
			this->progressBarTrainNextGesture->Size = System::Drawing::Size(338, 23);
			this->progressBarTrainNextGesture->Step = 1;
			this->progressBarTrainNextGesture->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBarTrainNextGesture->TabIndex = 47;
			this->progressBarTrainNextGesture->Visible = false;
			// 
			// pictureBoxGesture
			// 
			this->pictureBoxGesture->Location = System::Drawing::Point(6, 20);
			this->pictureBoxGesture->Name = L"pictureBoxGesture";
			this->pictureBoxGesture->Size = System::Drawing::Size(338, 303);
			this->pictureBoxGesture->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBoxGesture->TabIndex = 46;
			this->pictureBoxGesture->TabStop = false;
			this->pictureBoxGesture->Click += gcnew System::EventHandler(this, &MyForm::pictureBoxGesture_Click);
			// 
			// groupBoxClassify
			// 
			this->groupBoxClassify->Controls->Add(this->groupBoxClassifyLive);
			this->groupBoxClassify->Controls->Add(this->groupBoxClassifyFile);
			this->groupBoxClassify->Controls->Add(this->flowLayoutPanelClassifySource);
			this->groupBoxClassify->Controls->Add(this->buttonRecordDecisionDataDialog);
			this->groupBoxClassify->Controls->Add(this->textBoxRecordDecisionDataFilename);
			this->groupBoxClassify->Controls->Add(this->labelRecordDecisionDataTimestamp);
			this->groupBoxClassify->Location = System::Drawing::Point(500, 333);
			this->groupBoxClassify->Name = L"groupBoxClassify";
			this->groupBoxClassify->Size = System::Drawing::Size(350, 263);
			this->groupBoxClassify->TabIndex = 45;
			this->groupBoxClassify->TabStop = false;
			this->groupBoxClassify->Text = L"Classify";
			// 
			// groupBoxClassifyLive
			// 
			this->groupBoxClassifyLive->Controls->Add(this->buttonStartClassifier);
			this->groupBoxClassifyLive->Controls->Add(this->textBoxClassifierDecision);
			this->groupBoxClassifyLive->Controls->Add(this->checkBoxRecordDecisionData);
			this->groupBoxClassifyLive->Location = System::Drawing::Point(27, 159);
			this->groupBoxClassifyLive->Name = L"groupBoxClassifyLive";
			this->groupBoxClassifyLive->Size = System::Drawing::Size(296, 98);
			this->groupBoxClassifyLive->TabIndex = 47;
			this->groupBoxClassifyLive->TabStop = false;
			this->groupBoxClassifyLive->Text = L"Live";
			// 
			// buttonStartClassifier
			// 
			this->buttonStartClassifier->Enabled = false;
			this->buttonStartClassifier->ForeColor = System::Drawing::SystemColors::ActiveCaptionText;
			this->buttonStartClassifier->Location = System::Drawing::Point(141, 13);
			this->buttonStartClassifier->Name = L"buttonStartClassifier";
			this->buttonStartClassifier->Size = System::Drawing::Size(149, 34);
			this->buttonStartClassifier->TabIndex = 25;
			this->buttonStartClassifier->Text = L"Start Classifier";
			this->buttonStartClassifier->UseVisualStyleBackColor = true;
			this->buttonStartClassifier->Click += gcnew System::EventHandler(this, &MyForm::buttonStartClassifier_Click);
			// 
			// textBoxClassifierDecision
			// 
			this->textBoxClassifierDecision->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxClassifierDecision->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 20.25F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBoxClassifierDecision->ForeColor = System::Drawing::SystemColors::WindowText;
			this->textBoxClassifierDecision->Location = System::Drawing::Point(45, 53);
			this->textBoxClassifierDecision->Name = L"textBoxClassifierDecision";
			this->textBoxClassifierDecision->ReadOnly = true;
			this->textBoxClassifierDecision->Size = System::Drawing::Size(198, 38);
			this->textBoxClassifierDecision->TabIndex = 26;
			// 
			// checkBoxRecordDecisionData
			// 
			this->checkBoxRecordDecisionData->AutoSize = true;
			this->checkBoxRecordDecisionData->Location = System::Drawing::Point(9, 24);
			this->checkBoxRecordDecisionData->Name = L"checkBoxRecordDecisionData";
			this->checkBoxRecordDecisionData->Size = System::Drawing::Size(131, 17);
			this->checkBoxRecordDecisionData->TabIndex = 41;
			this->checkBoxRecordDecisionData->Text = L"Record Decision Data";
			this->checkBoxRecordDecisionData->UseVisualStyleBackColor = true;
			// 
			// groupBoxClassifyFile
			// 
			this->groupBoxClassifyFile->Controls->Add(this->panelClassifyFile);
			this->groupBoxClassifyFile->Enabled = false;
			this->groupBoxClassifyFile->Location = System::Drawing::Point(27, 71);
			this->groupBoxClassifyFile->Name = L"groupBoxClassifyFile";
			this->groupBoxClassifyFile->Size = System::Drawing::Size(296, 89);
			this->groupBoxClassifyFile->TabIndex = 46;
			this->groupBoxClassifyFile->TabStop = false;
			this->groupBoxClassifyFile->Text = L"File";
			// 
			// panelClassifyFile
			// 
			this->panelClassifyFile->Controls->Add(this->textBoxClassifyFile);
			this->panelClassifyFile->Controls->Add(this->buttonClassifyLoadData);
			this->panelClassifyFile->Controls->Add(this->buttonClassifyTestFile);
			this->panelClassifyFile->Controls->Add(this->radioButtonClassifyData);
			this->panelClassifyFile->Controls->Add(this->radioButtonClassifyFeatures);
			this->panelClassifyFile->Location = System::Drawing::Point(6, 19);
			this->panelClassifyFile->Name = L"panelClassifyFile";
			this->panelClassifyFile->Size = System::Drawing::Size(284, 64);
			this->panelClassifyFile->TabIndex = 49;
			// 
			// textBoxClassifyFile
			// 
			this->textBoxClassifyFile->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxClassifyFile->Location = System::Drawing::Point(5, 35);
			this->textBoxClassifyFile->Name = L"textBoxClassifyFile";
			this->textBoxClassifyFile->ReadOnly = true;
			this->textBoxClassifyFile->Size = System::Drawing::Size(151, 20);
			this->textBoxClassifyFile->TabIndex = 0;
			// 
			// buttonClassifyLoadData
			// 
			this->buttonClassifyLoadData->Location = System::Drawing::Point(160, 3);
			this->buttonClassifyLoadData->Name = L"buttonClassifyLoadData";
			this->buttonClassifyLoadData->Size = System::Drawing::Size(120, 26);
			this->buttonClassifyLoadData->TabIndex = 41;
			this->buttonClassifyLoadData->Text = L"Load File...";
			this->buttonClassifyLoadData->UseVisualStyleBackColor = true;
			this->buttonClassifyLoadData->Click += gcnew System::EventHandler(this, &MyForm::buttonClassifyLoadData_Click);
			// 
			// buttonClassifyTestFile
			// 
			this->buttonClassifyTestFile->Enabled = false;
			this->buttonClassifyTestFile->Location = System::Drawing::Point(160, 32);
			this->buttonClassifyTestFile->Name = L"buttonClassifyTestFile";
			this->buttonClassifyTestFile->Size = System::Drawing::Size(120, 26);
			this->buttonClassifyTestFile->TabIndex = 48;
			this->buttonClassifyTestFile->Text = L"Test";
			this->buttonClassifyTestFile->UseVisualStyleBackColor = true;
			this->buttonClassifyTestFile->Click += gcnew System::EventHandler(this, &MyForm::buttonClassifyTestFile_Click);
			// 
			// radioButtonClassifyData
			// 
			this->radioButtonClassifyData->AutoSize = true;
			this->radioButtonClassifyData->Checked = true;
			this->radioButtonClassifyData->Location = System::Drawing::Point(10, 7);
			this->radioButtonClassifyData->Name = L"radioButtonClassifyData";
			this->radioButtonClassifyData->Size = System::Drawing::Size(48, 17);
			this->radioButtonClassifyData->TabIndex = 42;
			this->radioButtonClassifyData->TabStop = true;
			this->radioButtonClassifyData->Text = L"Data";
			this->radioButtonClassifyData->UseVisualStyleBackColor = true;
			this->radioButtonClassifyData->CheckedChanged += gcnew System::EventHandler(this, &MyForm::radioButtonClassifyData_CheckedChanged);
			// 
			// radioButtonClassifyFeatures
			// 
			this->radioButtonClassifyFeatures->AutoSize = true;
			this->radioButtonClassifyFeatures->Location = System::Drawing::Point(80, 8);
			this->radioButtonClassifyFeatures->Name = L"radioButtonClassifyFeatures";
			this->radioButtonClassifyFeatures->Size = System::Drawing::Size(66, 17);
			this->radioButtonClassifyFeatures->TabIndex = 43;
			this->radioButtonClassifyFeatures->Text = L"Features";
			this->radioButtonClassifyFeatures->UseVisualStyleBackColor = true;
			// 
			// flowLayoutPanelClassifySource
			// 
			this->flowLayoutPanelClassifySource->Controls->Add(this->radioButtonClassifyLive);
			this->flowLayoutPanelClassifySource->Controls->Add(this->radioButtonClassifyFile);
			this->flowLayoutPanelClassifySource->Location = System::Drawing::Point(27, 45);
			this->flowLayoutPanelClassifySource->Name = L"flowLayoutPanelClassifySource";
			this->flowLayoutPanelClassifySource->Size = System::Drawing::Size(296, 23);
			this->flowLayoutPanelClassifySource->TabIndex = 46;
			// 
			// radioButtonClassifyLive
			// 
			this->radioButtonClassifyLive->AutoSize = true;
			this->radioButtonClassifyLive->Checked = true;
			this->radioButtonClassifyLive->Location = System::Drawing::Point(3, 3);
			this->radioButtonClassifyLive->Name = L"radioButtonClassifyLive";
			this->radioButtonClassifyLive->Size = System::Drawing::Size(45, 17);
			this->radioButtonClassifyLive->TabIndex = 45;
			this->radioButtonClassifyLive->TabStop = true;
			this->radioButtonClassifyLive->Text = L"Live";
			this->radioButtonClassifyLive->UseVisualStyleBackColor = true;
			this->radioButtonClassifyLive->CheckedChanged += gcnew System::EventHandler(this, &MyForm::radioButtonClassifier_CheckedChanged);
			// 
			// radioButtonClassifyFile
			// 
			this->radioButtonClassifyFile->AutoSize = true;
			this->radioButtonClassifyFile->Location = System::Drawing::Point(54, 3);
			this->radioButtonClassifyFile->Name = L"radioButtonClassifyFile";
			this->radioButtonClassifyFile->Size = System::Drawing::Size(41, 17);
			this->radioButtonClassifyFile->TabIndex = 46;
			this->radioButtonClassifyFile->Text = L"File";
			this->radioButtonClassifyFile->UseVisualStyleBackColor = true;
			// 
			// buttonRecordDecisionDataDialog
			// 
			this->buttonRecordDecisionDataDialog->Location = System::Drawing::Point(293, 15);
			this->buttonRecordDecisionDataDialog->Name = L"buttonRecordDecisionDataDialog";
			this->buttonRecordDecisionDataDialog->Size = System::Drawing::Size(30, 26);
			this->buttonRecordDecisionDataDialog->TabIndex = 42;
			this->buttonRecordDecisionDataDialog->Text = L"...";
			this->buttonRecordDecisionDataDialog->UseVisualStyleBackColor = true;
			this->buttonRecordDecisionDataDialog->Click += gcnew System::EventHandler(this, &MyForm::buttonRecordDecisionDataDialog_Click);
			// 
			// textBoxRecordDecisionDataFilename
			// 
			this->textBoxRecordDecisionDataFilename->Location = System::Drawing::Point(27, 19);
			this->textBoxRecordDecisionDataFilename->Name = L"textBoxRecordDecisionDataFilename";
			this->textBoxRecordDecisionDataFilename->Size = System::Drawing::Size(152, 20);
			this->textBoxRecordDecisionDataFilename->TabIndex = 44;
			this->textBoxRecordDecisionDataFilename->Text = L"decision-data";
			// 
			// labelRecordDecisionDataTimestamp
			// 
			this->labelRecordDecisionDataTimestamp->AutoSize = true;
			this->labelRecordDecisionDataTimestamp->Location = System::Drawing::Point(182, 22);
			this->labelRecordDecisionDataTimestamp->Name = L"labelRecordDecisionDataTimestamp";
			this->labelRecordDecisionDataTimestamp->Size = System::Drawing::Size(112, 13);
			this->labelRecordDecisionDataTimestamp->TabIndex = 43;
			this->labelRecordDecisionDataTimestamp->Text = L".yy-mm-dd.hhmm.ss.txt";
			// 
			// groupBoxTrain
			// 
			this->groupBoxTrain->Controls->Add(this->confusionMatrix);
			this->groupBoxTrain->Controls->Add(this->numericUpDownSVMParamCoef0);
			this->groupBoxTrain->Controls->Add(this->numericUpDownSVMParamGamma);
			this->groupBoxTrain->Controls->Add(this->labelSVMParamCoef0);
			this->groupBoxTrain->Controls->Add(this->labelSVMParamGamma);
			this->groupBoxTrain->Controls->Add(this->labelSVMParamDegree);
			this->groupBoxTrain->Controls->Add(this->numericUpDownSVMParamDegree);
			this->groupBoxTrain->Controls->Add(this->labelSVMKernel);
			this->groupBoxTrain->Controls->Add(this->labelClassifierType);
			this->groupBoxTrain->Controls->Add(this->comboBoxSVMKernel);
			this->groupBoxTrain->Controls->Add(this->comboBoxClassifierType);
			this->groupBoxTrain->Controls->Add(this->buttonSaveModel);
			this->groupBoxTrain->Controls->Add(this->buttonLoadModel);
			this->groupBoxTrain->Controls->Add(this->buttonCreateModel);
			this->groupBoxTrain->Controls->Add(this->labelTrainingAccuracy);
			this->groupBoxTrain->Controls->Add(this->textBoxTrainingAccuracy);
			this->groupBoxTrain->Location = System::Drawing::Point(14, 333);
			this->groupBoxTrain->Name = L"groupBoxTrain";
			this->groupBoxTrain->Size = System::Drawing::Size(466, 263);
			this->groupBoxTrain->TabIndex = 29;
			this->groupBoxTrain->TabStop = false;
			this->groupBoxTrain->Text = L"Train";
			// 
			// confusionMatrix
			// 
			this->confusionMatrix->AllowUserToAddRows = false;
			this->confusionMatrix->AllowUserToResizeColumns = false;
			this->confusionMatrix->AllowUserToResizeRows = false;
			this->confusionMatrix->BackgroundColor = System::Drawing::SystemColors::Window;
			this->confusionMatrix->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->confusionMatrix->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->confusionMatrix->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(1) { this->dataGridViewTextBoxColumn1 });
			this->confusionMatrix->Location = System::Drawing::Point(13, 128);
			this->confusionMatrix->Name = L"confusionMatrix";
			this->confusionMatrix->RowHeadersVisible = false;
			this->confusionMatrix->RowHeadersWidthSizeMode = System::Windows::Forms::DataGridViewRowHeadersWidthSizeMode::DisableResizing;
			this->confusionMatrix->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->confusionMatrix->Size = System::Drawing::Size(315, 123);
			this->confusionMatrix->TabIndex = 37;
			this->confusionMatrix->CellContentClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MyForm::confusionMatrix_CellContentClick);
			// 
			// dataGridViewTextBoxColumn1
			// 
			this->dataGridViewTextBoxColumn1->HeaderText = L"Index";
			this->dataGridViewTextBoxColumn1->Name = L"dataGridViewTextBoxColumn1";
			this->dataGridViewTextBoxColumn1->ReadOnly = true;
			this->dataGridViewTextBoxColumn1->Width = 50;
			// 
			// numericUpDownSVMParamCoef0
			// 
			this->numericUpDownSVMParamCoef0->DecimalPlaces = 4;
			this->numericUpDownSVMParamCoef0->Location = System::Drawing::Point(165, 75);
			this->numericUpDownSVMParamCoef0->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10000, 0, 0, 0 });
			this->numericUpDownSVMParamCoef0->Name = L"numericUpDownSVMParamCoef0";
			this->numericUpDownSVMParamCoef0->Size = System::Drawing::Size(65, 20);
			this->numericUpDownSVMParamCoef0->TabIndex = 40;
			this->numericUpDownSVMParamCoef0->Visible = false;
			// 
			// numericUpDownSVMParamGamma
			// 
			this->numericUpDownSVMParamGamma->DecimalPlaces = 4;
			this->numericUpDownSVMParamGamma->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 65536 });
			this->numericUpDownSVMParamGamma->Location = System::Drawing::Point(88, 75);
			this->numericUpDownSVMParamGamma->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10000, 0, 0, 0 });
			this->numericUpDownSVMParamGamma->Name = L"numericUpDownSVMParamGamma";
			this->numericUpDownSVMParamGamma->Size = System::Drawing::Size(65, 20);
			this->numericUpDownSVMParamGamma->TabIndex = 39;
			this->numericUpDownSVMParamGamma->Visible = false;
			// 
			// labelSVMParamCoef0
			// 
			this->labelSVMParamCoef0->AutoSize = true;
			this->labelSVMParamCoef0->Location = System::Drawing::Point(163, 59);
			this->labelSVMParamCoef0->Name = L"labelSVMParamCoef0";
			this->labelSVMParamCoef0->Size = System::Drawing::Size(35, 13);
			this->labelSVMParamCoef0->TabIndex = 38;
			this->labelSVMParamCoef0->Text = L"Coef0";
			this->labelSVMParamCoef0->Visible = false;
			this->labelSVMParamCoef0->Click += gcnew System::EventHandler(this, &MyForm::labelSVMParamCoef0_Click);
			// 
			// labelSVMParamGamma
			// 
			this->labelSVMParamGamma->AutoSize = true;
			this->labelSVMParamGamma->Location = System::Drawing::Point(85, 59);
			this->labelSVMParamGamma->Name = L"labelSVMParamGamma";
			this->labelSVMParamGamma->Size = System::Drawing::Size(43, 13);
			this->labelSVMParamGamma->TabIndex = 37;
			this->labelSVMParamGamma->Text = L"Gamma";
			this->labelSVMParamGamma->Visible = false;
			// 
			// labelSVMParamDegree
			// 
			this->labelSVMParamDegree->AutoSize = true;
			this->labelSVMParamDegree->Location = System::Drawing::Point(9, 59);
			this->labelSVMParamDegree->Name = L"labelSVMParamDegree";
			this->labelSVMParamDegree->Size = System::Drawing::Size(42, 13);
			this->labelSVMParamDegree->TabIndex = 36;
			this->labelSVMParamDegree->Text = L"Degree";
			this->labelSVMParamDegree->Visible = false;
			// 
			// numericUpDownSVMParamDegree
			// 
			this->numericUpDownSVMParamDegree->Location = System::Drawing::Point(12, 75);
			this->numericUpDownSVMParamDegree->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10000, 0, 0, 0 });
			this->numericUpDownSVMParamDegree->Name = L"numericUpDownSVMParamDegree";
			this->numericUpDownSVMParamDegree->Size = System::Drawing::Size(65, 20);
			this->numericUpDownSVMParamDegree->TabIndex = 35;
			this->numericUpDownSVMParamDegree->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			this->numericUpDownSVMParamDegree->Visible = false;
			// 
			// labelSVMKernel
			// 
			this->labelSVMKernel->AutoSize = true;
			this->labelSVMKernel->Location = System::Drawing::Point(136, 16);
			this->labelSVMKernel->Name = L"labelSVMKernel";
			this->labelSVMKernel->Size = System::Drawing::Size(63, 13);
			this->labelSVMKernel->TabIndex = 34;
			this->labelSVMKernel->Text = L"SVM Kernel";
			// 
			// labelClassifierType
			// 
			this->labelClassifierType->AutoSize = true;
			this->labelClassifierType->Location = System::Drawing::Point(9, 16);
			this->labelClassifierType->Name = L"labelClassifierType";
			this->labelClassifierType->Size = System::Drawing::Size(75, 13);
			this->labelClassifierType->TabIndex = 33;
			this->labelClassifierType->Text = L"Classifier Type";
			// 
			// comboBoxSVMKernel
			// 
			this->comboBoxSVMKernel->FormattingEnabled = true;
			this->comboBoxSVMKernel->Items->AddRange(gcnew cli::array< System::Object^  >(4) { L"LINEAR", L"POLY", L"RBF", L"SIGMOID" });
			this->comboBoxSVMKernel->Location = System::Drawing::Point(139, 32);
			this->comboBoxSVMKernel->Name = L"comboBoxSVMKernel";
			this->comboBoxSVMKernel->Size = System::Drawing::Size(121, 21);
			this->comboBoxSVMKernel->TabIndex = 32;
			this->comboBoxSVMKernel->SelectionChangeCommitted += gcnew System::EventHandler(this, &MyForm::comboBoxSVMKernel_SelectionChangeCommitted);
			// 
			// comboBoxClassifierType
			// 
			this->comboBoxClassifierType->FormattingEnabled = true;
			this->comboBoxClassifierType->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"SVM C_SVC", L"SVM NU_SVC", L"LDA" });
			this->comboBoxClassifierType->Location = System::Drawing::Point(12, 32);
			this->comboBoxClassifierType->Name = L"comboBoxClassifierType";
			this->comboBoxClassifierType->Size = System::Drawing::Size(121, 21);
			this->comboBoxClassifierType->TabIndex = 31;
			this->comboBoxClassifierType->SelectionChangeCommitted += gcnew System::EventHandler(this, &MyForm::comboBoxClassifierType_SelectionChangeCommitted);
			// 
			// buttonSaveModel
			// 
			this->buttonSaveModel->Enabled = false;
			this->buttonSaveModel->Location = System::Drawing::Point(340, 82);
			this->buttonSaveModel->Name = L"buttonSaveModel";
			this->buttonSaveModel->Size = System::Drawing::Size(120, 26);
			this->buttonSaveModel->TabIndex = 27;
			this->buttonSaveModel->Text = L"Save Model...";
			this->buttonSaveModel->UseVisualStyleBackColor = true;
			this->buttonSaveModel->Click += gcnew System::EventHandler(this, &MyForm::buttonSaveModel_Click);
			// 
			// buttonLoadModel
			// 
			this->buttonLoadModel->Location = System::Drawing::Point(340, 55);
			this->buttonLoadModel->Name = L"buttonLoadModel";
			this->buttonLoadModel->Size = System::Drawing::Size(120, 26);
			this->buttonLoadModel->TabIndex = 26;
			this->buttonLoadModel->Text = L"Load Model...";
			this->buttonLoadModel->UseVisualStyleBackColor = true;
			this->buttonLoadModel->Click += gcnew System::EventHandler(this, &MyForm::buttonLoadModel_Click);
			// 
			// buttonCreateModel
			// 
			this->buttonCreateModel->Location = System::Drawing::Point(340, 28);
			this->buttonCreateModel->Name = L"buttonCreateModel";
			this->buttonCreateModel->Size = System::Drawing::Size(120, 26);
			this->buttonCreateModel->TabIndex = 12;
			this->buttonCreateModel->Text = L"Create Model";
			this->buttonCreateModel->UseVisualStyleBackColor = true;
			this->buttonCreateModel->Click += gcnew System::EventHandler(this, &MyForm::buttonCreateModel_Click);
			// 
			// labelTrainingAccuracy
			// 
			this->labelTrainingAccuracy->AutoSize = true;
			this->labelTrainingAccuracy->Location = System::Drawing::Point(9, 106);
			this->labelTrainingAccuracy->Name = L"labelTrainingAccuracy";
			this->labelTrainingAccuracy->Size = System::Drawing::Size(96, 13);
			this->labelTrainingAccuracy->TabIndex = 26;
			this->labelTrainingAccuracy->Text = L"Training Accuracy:";
			// 
			// textBoxTrainingAccuracy
			// 
			this->textBoxTrainingAccuracy->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxTrainingAccuracy->Location = System::Drawing::Point(110, 102);
			this->textBoxTrainingAccuracy->Name = L"textBoxTrainingAccuracy";
			this->textBoxTrainingAccuracy->ReadOnly = true;
			this->textBoxTrainingAccuracy->Size = System::Drawing::Size(100, 20);
			this->textBoxTrainingAccuracy->TabIndex = 25;
			// 
			// groupBoxRealtimeTesting
			// 
			this->groupBoxRealtimeTesting->Controls->Add(this->textBoxNextGestureWarning);
			this->groupBoxRealtimeTesting->Controls->Add(this->textBoxCurrentFlag);
			this->groupBoxRealtimeTesting->Controls->Add(this->buttonSetDelayNextGesture);
			this->groupBoxRealtimeTesting->Controls->Add(this->comboBoxNextGesture);
			this->groupBoxRealtimeTesting->Controls->Add(this->progressBarNextGestureWarning);
			this->groupBoxRealtimeTesting->Controls->Add(this->buttonSetNowNextGesture);
			this->groupBoxRealtimeTesting->Controls->Add(this->labelCurrentFlag);
			this->groupBoxRealtimeTesting->Controls->Add(this->labelNextGesture);
			this->groupBoxRealtimeTesting->Location = System::Drawing::Point(872, 333);
			this->groupBoxRealtimeTesting->Name = L"groupBoxRealtimeTesting";
			this->groupBoxRealtimeTesting->Size = System::Drawing::Size(371, 263);
			this->groupBoxRealtimeTesting->TabIndex = 30;
			this->groupBoxRealtimeTesting->TabStop = false;
			this->groupBoxRealtimeTesting->Text = L"Real-time Testing";
			// 
			// textBoxNextGestureWarning
			// 
			this->textBoxNextGestureWarning->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxNextGestureWarning->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBoxNextGestureWarning->Font = (gcnew System::Drawing::Font(L"Lucida Sans Unicode", 15.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBoxNextGestureWarning->Location = System::Drawing::Point(72, 116);
			this->textBoxNextGestureWarning->Name = L"textBoxNextGestureWarning";
			this->textBoxNextGestureWarning->ReadOnly = true;
			this->textBoxNextGestureWarning->Size = System::Drawing::Size(225, 33);
			this->textBoxNextGestureWarning->TabIndex = 8;
			this->textBoxNextGestureWarning->Text = L"Do gesture in 3...";
			this->textBoxNextGestureWarning->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->textBoxNextGestureWarning->Visible = false;
			this->textBoxNextGestureWarning->TextChanged += gcnew System::EventHandler(this, &MyForm::textBoxNextGestureWarning_TextChanged);
			// 
			// textBoxCurrentFlag
			// 
			this->textBoxCurrentFlag->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxCurrentFlag->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 20.25F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBoxCurrentFlag->Location = System::Drawing::Point(83, 213);
			this->textBoxCurrentFlag->Name = L"textBoxCurrentFlag";
			this->textBoxCurrentFlag->ReadOnly = true;
			this->textBoxCurrentFlag->Size = System::Drawing::Size(200, 38);
			this->textBoxCurrentFlag->TabIndex = 7;
			// 
			// buttonSetDelayNextGesture
			// 
			this->buttonSetDelayNextGesture->Location = System::Drawing::Point(256, 39);
			this->buttonSetDelayNextGesture->Name = L"buttonSetDelayNextGesture";
			this->buttonSetDelayNextGesture->Size = System::Drawing::Size(80, 26);
			this->buttonSetDelayNextGesture->TabIndex = 6;
			this->buttonSetDelayNextGesture->Text = L"Set in 3s";
			this->buttonSetDelayNextGesture->UseVisualStyleBackColor = true;
			this->buttonSetDelayNextGesture->Click += gcnew System::EventHandler(this, &MyForm::buttonSetDelayNextGesture_Click);
			// 
			// comboBoxNextGesture
			// 
			this->comboBoxNextGesture->FormattingEnabled = true;
			this->comboBoxNextGesture->Location = System::Drawing::Point(20, 43);
			this->comboBoxNextGesture->Name = L"comboBoxNextGesture";
			this->comboBoxNextGesture->Size = System::Drawing::Size(141, 21);
			this->comboBoxNextGesture->TabIndex = 5;
			// 
			// progressBarNextGestureWarning
			// 
			this->progressBarNextGestureWarning->Location = System::Drawing::Point(72, 155);
			this->progressBarNextGestureWarning->Maximum = 300;
			this->progressBarNextGestureWarning->Name = L"progressBarNextGestureWarning";
			this->progressBarNextGestureWarning->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->progressBarNextGestureWarning->RightToLeftLayout = true;
			this->progressBarNextGestureWarning->Size = System::Drawing::Size(225, 21);
			this->progressBarNextGestureWarning->Step = 1;
			this->progressBarNextGestureWarning->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBarNextGestureWarning->TabIndex = 4;
			this->progressBarNextGestureWarning->Visible = false;
			// 
			// buttonSetNowNextGesture
			// 
			this->buttonSetNowNextGesture->Location = System::Drawing::Point(170, 39);
			this->buttonSetNowNextGesture->Name = L"buttonSetNowNextGesture";
			this->buttonSetNowNextGesture->Size = System::Drawing::Size(80, 26);
			this->buttonSetNowNextGesture->TabIndex = 3;
			this->buttonSetNowNextGesture->Text = L"Set Now";
			this->buttonSetNowNextGesture->UseVisualStyleBackColor = true;
			this->buttonSetNowNextGesture->Click += gcnew System::EventHandler(this, &MyForm::buttonSetNowNextGesture_Click);
			// 
			// labelCurrentFlag
			// 
			this->labelCurrentFlag->AutoSize = true;
			this->labelCurrentFlag->Location = System::Drawing::Point(37, 279);
			this->labelCurrentFlag->Name = L"labelCurrentFlag";
			this->labelCurrentFlag->Size = System::Drawing::Size(64, 13);
			this->labelCurrentFlag->TabIndex = 2;
			this->labelCurrentFlag->Text = L"Current Flag";
			// 
			// labelNextGesture
			// 
			this->labelNextGesture->AutoSize = true;
			this->labelNextGesture->Location = System::Drawing::Point(17, 27);
			this->labelNextGesture->Name = L"labelNextGesture";
			this->labelNextGesture->Size = System::Drawing::Size(69, 13);
			this->labelNextGesture->TabIndex = 1;
			this->labelNextGesture->Text = L"Next Gesture";
			// 
			// groupBoxGesture
			// 
			this->groupBoxGesture->Controls->Add(this->textBoxAutoRunSeconds);
			this->groupBoxGesture->Controls->Add(this->label1);
			this->groupBoxGesture->Controls->Add(this->checkBoxAutoRun);
			this->groupBoxGesture->Controls->Add(this->buttonSaveGestures);
			this->groupBoxGesture->Controls->Add(this->buttonLoadGestures);
			this->groupBoxGesture->Controls->Add(this->buttonSaveFeatures);
			this->groupBoxGesture->Controls->Add(this->buttonImportData);
			this->groupBoxGesture->Controls->Add(this->buttonClearSamples);
			this->groupBoxGesture->Controls->Add(this->gestureList);
			this->groupBoxGesture->Controls->Add(this->buttonSaveData);
			this->groupBoxGesture->Controls->Add(this->button34);
			this->groupBoxGesture->Controls->Add(this->numericUpDown4);
			this->groupBoxGesture->Controls->Add(this->label75);
			this->groupBoxGesture->Controls->Add(this->textBox35);
			this->groupBoxGesture->Controls->Add(this->textBox30);
			this->groupBoxGesture->Controls->Add(this->textBox29);
			this->groupBoxGesture->Controls->Add(this->label88);
			this->groupBoxGesture->Controls->Add(this->label72);
			this->groupBoxGesture->Controls->Add(this->label71);
			this->groupBoxGesture->Controls->Add(this->label70);
			this->groupBoxGesture->Controls->Add(this->buttonDeleteAll);
			this->groupBoxGesture->Controls->Add(this->buttonAddGesture);
			this->groupBoxGesture->Controls->Add(this->buttonTrainGesture);
			this->groupBoxGesture->Controls->Add(this->textBox14);
			this->groupBoxGesture->Location = System::Drawing::Point(13, 0);
			this->groupBoxGesture->Name = L"groupBoxGesture";
			this->groupBoxGesture->Size = System::Drawing::Size(467, 329);
			this->groupBoxGesture->TabIndex = 23;
			this->groupBoxGesture->TabStop = false;
			this->groupBoxGesture->Text = L"Gesture";
			// 
			// textBoxAutoRunSeconds
			// 
			this->textBoxAutoRunSeconds->Location = System::Drawing::Point(346, 225);
			this->textBoxAutoRunSeconds->Name = L"textBoxAutoRunSeconds";
			this->textBoxAutoRunSeconds->Size = System::Drawing::Size(21, 20);
			this->textBoxAutoRunSeconds->TabIndex = 50;
			this->textBoxAutoRunSeconds->Text = L"3";
			this->textBoxAutoRunSeconds->Visible = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(373, 232);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(46, 13);
			this->label1->TabIndex = 49;
			this->label1->Text = L"delay (s)";
			this->label1->Visible = false;
			// 
			// checkBoxAutoRun
			// 
			this->checkBoxAutoRun->AutoSize = true;
			this->checkBoxAutoRun->Location = System::Drawing::Point(346, 170);
			this->checkBoxAutoRun->Name = L"checkBoxAutoRun";
			this->checkBoxAutoRun->Size = System::Drawing::Size(71, 17);
			this->checkBoxAutoRun->TabIndex = 48;
			this->checkBoxAutoRun->Text = L"Auto Run";
			this->checkBoxAutoRun->UseVisualStyleBackColor = true;
			this->checkBoxAutoRun->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxAutoRun_CheckedChanged);
			// 
			// buttonSaveGestures
			// 
			this->buttonSaveGestures->Location = System::Drawing::Point(341, 265);
			this->buttonSaveGestures->Name = L"buttonSaveGestures";
			this->buttonSaveGestures->Size = System::Drawing::Size(120, 26);
			this->buttonSaveGestures->TabIndex = 47;
			this->buttonSaveGestures->Text = L"Save Gestures...";
			this->buttonSaveGestures->UseVisualStyleBackColor = true;
			this->buttonSaveGestures->Click += gcnew System::EventHandler(this, &MyForm::buttonSaveGestures_Click);
			// 
			// buttonLoadGestures
			// 
			this->buttonLoadGestures->Location = System::Drawing::Point(341, 44);
			this->buttonLoadGestures->Name = L"buttonLoadGestures";
			this->buttonLoadGestures->Size = System::Drawing::Size(120, 26);
			this->buttonLoadGestures->TabIndex = 46;
			this->buttonLoadGestures->Text = L"Import Gestures";
			this->buttonLoadGestures->UseVisualStyleBackColor = true;
			this->buttonLoadGestures->Click += gcnew System::EventHandler(this, &MyForm::buttonLoadGestures_Click);
			// 
			// buttonSaveFeatures
			// 
			this->buttonSaveFeatures->Location = System::Drawing::Point(63, 297);
			this->buttonSaveFeatures->Name = L"buttonSaveFeatures";
			this->buttonSaveFeatures->Size = System::Drawing::Size(120, 26);
			this->buttonSaveFeatures->TabIndex = 39;
			this->buttonSaveFeatures->Text = L"Save Features...";
			this->buttonSaveFeatures->UseVisualStyleBackColor = true;
			this->buttonSaveFeatures->Click += gcnew System::EventHandler(this, &MyForm::buttonSaveFeatures_Click);
			// 
			// buttonImportData
			// 
			this->buttonImportData->Location = System::Drawing::Point(215, 297);
			this->buttonImportData->Name = L"buttonImportData";
			this->buttonImportData->Size = System::Drawing::Size(120, 26);
			this->buttonImportData->TabIndex = 38;
			this->buttonImportData->Text = L"Import Data...";
			this->buttonImportData->UseVisualStyleBackColor = true;
			this->buttonImportData->Click += gcnew System::EventHandler(this, &MyForm::buttonImportData_Click);
			// 
			// buttonClearSamples
			// 
			this->buttonClearSamples->Enabled = false;
			this->buttonClearSamples->Location = System::Drawing::Point(341, 105);
			this->buttonClearSamples->Name = L"buttonClearSamples";
			this->buttonClearSamples->Size = System::Drawing::Size(120, 26);
			this->buttonClearSamples->TabIndex = 37;
			this->buttonClearSamples->Text = L"Clear Samples";
			this->buttonClearSamples->UseVisualStyleBackColor = true;
			this->buttonClearSamples->Click += gcnew System::EventHandler(this, &MyForm::buttonClearSamples_Click);
			// 
			// gestureList
			// 
			this->gestureList->AllowUserToAddRows = false;
			this->gestureList->AllowUserToResizeColumns = false;
			this->gestureList->AllowUserToResizeRows = false;
			this->gestureList->BackgroundColor = System::Drawing::SystemColors::Window;
			this->gestureList->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->gestureList->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->gestureList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(3) {
				this->Index, this->Gesture,
					this->Samples
			});
			this->gestureList->Location = System::Drawing::Point(13, 73);
			this->gestureList->Name = L"gestureList";
			this->gestureList->RowHeadersVisible = false;
			this->gestureList->RowHeadersWidthSizeMode = System::Windows::Forms::DataGridViewRowHeadersWidthSizeMode::DisableResizing;
			this->gestureList->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->gestureList->Size = System::Drawing::Size(322, 218);
			this->gestureList->TabIndex = 36;
			this->gestureList->CellContentClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MyForm::gestureList_CellContentClick);
			this->gestureList->CellValueChanged += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MyForm::gestureList_CellValueChanged);
			this->gestureList->CurrentCellChanged += gcnew System::EventHandler(this, &MyForm::gestureList_CurrentCellChanged);
			this->gestureList->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MyForm::gesture_KeyPress);
			// 
			// Index
			// 
			this->Index->HeaderText = L"Index";
			this->Index->Name = L"Index";
			this->Index->ReadOnly = true;
			this->Index->Width = 50;
			// 
			// Gesture
			// 
			this->Gesture->HeaderText = L"Gesture";
			this->Gesture->Name = L"Gesture";
			this->Gesture->Width = 150;
			// 
			// Samples
			// 
			this->Samples->HeaderText = L"Samples";
			this->Samples->Name = L"Samples";
			this->Samples->ReadOnly = true;
			// 
			// buttonSaveData
			// 
			this->buttonSaveData->Location = System::Drawing::Point(341, 297);
			this->buttonSaveData->Name = L"buttonSaveData";
			this->buttonSaveData->Size = System::Drawing::Size(120, 26);
			this->buttonSaveData->TabIndex = 35;
			this->buttonSaveData->Text = L"Save Data...";
			this->buttonSaveData->UseVisualStyleBackColor = true;
			this->buttonSaveData->Click += gcnew System::EventHandler(this, &MyForm::buttonSaveData_Click);
			// 
			// button34
			// 
			this->button34->Location = System::Drawing::Point(341, 137);
			this->button34->Name = L"button34";
			this->button34->Size = System::Drawing::Size(120, 26);
			this->button34->TabIndex = 26;
			this->button34->Text = L"Select Picture";
			this->button34->UseVisualStyleBackColor = true;
			this->button34->Click += gcnew System::EventHandler(this, &MyForm::button34_Click_1);
			// 
			// numericUpDown4
			// 
			this->numericUpDown4->Location = System::Drawing::Point(418, 18);
			this->numericUpDown4->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
			this->numericUpDown4->Name = L"numericUpDown4";
			this->numericUpDown4->Size = System::Drawing::Size(41, 20);
			this->numericUpDown4->TabIndex = 34;
			this->numericUpDown4->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, 0 });
			// 
			// label75
			// 
			this->label75->AutoSize = true;
			this->label75->Location = System::Drawing::Point(124, 20);
			this->label75->Name = L"label75";
			this->label75->Size = System::Drawing::Size(40, 13);
			this->label75->TabIndex = 11;
			this->label75->Text = L"Status:";
			// 
			// textBox35
			// 
			this->textBox35->BackColor = System::Drawing::SystemColors::Window;
			this->textBox35->Location = System::Drawing::Point(168, 17);
			this->textBox35->Name = L"textBox35";
			this->textBox35->ReadOnly = true;
			this->textBox35->Size = System::Drawing::Size(54, 20);
			this->textBox35->TabIndex = 10;
			// 
			// textBox30
			// 
			this->textBox30->BackColor = System::Drawing::SystemColors::Window;
			this->textBox30->Location = System::Drawing::Point(284, 17);
			this->textBox30->Name = L"textBox30";
			this->textBox30->ReadOnly = true;
			this->textBox30->Size = System::Drawing::Size(49, 20);
			this->textBox30->TabIndex = 8;
			// 
			// textBox29
			// 
			this->textBox29->BackColor = System::Drawing::SystemColors::Window;
			this->textBox29->Location = System::Drawing::Point(93, 17);
			this->textBox29->Name = L"textBox29";
			this->textBox29->ReadOnly = true;
			this->textBox29->Size = System::Drawing::Size(29, 20);
			this->textBox29->TabIndex = 8;
			// 
			// label88
			// 
			this->label88->AutoSize = true;
			this->label88->Location = System::Drawing::Point(343, 20);
			this->label88->Name = L"label88";
			this->label88->Size = System::Drawing::Size(73, 13);
			this->label88->TabIndex = 7;
			this->label88->Text = L"Max Samples:";
			// 
			// label72
			// 
			this->label72->AutoSize = true;
			this->label72->Location = System::Drawing::Point(232, 20);
			this->label72->Name = L"label72";
			this->label72->Size = System::Drawing::Size(53, 13);
			this->label72->TabIndex = 7;
			this->label72->Text = L"Samples: ";
			// 
			// label71
			// 
			this->label71->AutoSize = true;
			this->label71->Location = System::Drawing::Point(13, 20);
			this->label71->Name = L"label71";
			this->label71->Size = System::Drawing::Size(74, 13);
			this->label71->TabIndex = 7;
			this->label71->Text = L"# of Gestures:";
			// 
			// label70
			// 
			this->label70->AutoSize = true;
			this->label70->Location = System::Drawing::Point(12, 50);
			this->label70->Name = L"label70";
			this->label70->Size = System::Drawing::Size(38, 13);
			this->label70->TabIndex = 6;
			this->label70->Text = L"Name:";
			// 
			// buttonDeleteAll
			// 
			this->buttonDeleteAll->Enabled = false;
			this->buttonDeleteAll->Location = System::Drawing::Point(341, 193);
			this->buttonDeleteAll->Name = L"buttonDeleteAll";
			this->buttonDeleteAll->Size = System::Drawing::Size(120, 26);
			this->buttonDeleteAll->TabIndex = 4;
			this->buttonDeleteAll->Text = L"Delete All";
			this->buttonDeleteAll->UseVisualStyleBackColor = true;
			this->buttonDeleteAll->Click += gcnew System::EventHandler(this, &MyForm::buttonDeleteAll_Click);
			// 
			// buttonAddGesture
			// 
			this->buttonAddGesture->Location = System::Drawing::Point(230, 43);
			this->buttonAddGesture->Name = L"buttonAddGesture";
			this->buttonAddGesture->Size = System::Drawing::Size(105, 26);
			this->buttonAddGesture->TabIndex = 2;
			this->buttonAddGesture->Text = L"Add";
			this->buttonAddGesture->UseVisualStyleBackColor = true;
			this->buttonAddGesture->Click += gcnew System::EventHandler(this, &MyForm::buttonAddGesture_Click);

			// 
			// buttonTrainGesture
			// 
			this->buttonTrainGesture->Enabled = false;
			this->buttonTrainGesture->Location = System::Drawing::Point(341, 73);
			this->buttonTrainGesture->Name = L"buttonTrainGesture";
			this->buttonTrainGesture->Size = System::Drawing::Size(120, 26);
			this->buttonTrainGesture->TabIndex = 3;
			this->buttonTrainGesture->Text = L"Train";
			this->buttonTrainGesture->UseVisualStyleBackColor = true;
			this->buttonTrainGesture->Click += gcnew System::EventHandler(this, &MyForm::buttonTrainGesture_Click_1);
			// 
			// textBox14
			// 
			this->textBox14->Location = System::Drawing::Point(56, 47);
			this->textBox14->Name = L"textBox14";
			this->textBox14->Size = System::Drawing::Size(166, 20);
			this->textBox14->TabIndex = 1;
			this->textBox14->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MyForm::textBox14_KeyPress);
			// 
			// groupBox17
			// 
			this->groupBox17->Controls->Add(this->label76);
			this->groupBox17->Controls->Add(this->checkBox5);
			this->groupBox17->Controls->Add(this->label40);
			this->groupBox17->Controls->Add(this->chart4);
			this->groupBox17->Controls->Add(this->numericUpDown1);
			this->groupBox17->Location = System::Drawing::Point(872, 2);
			this->groupBox17->Name = L"groupBox17";
			this->groupBox17->Size = System::Drawing::Size(371, 329);
			this->groupBox17->TabIndex = 22;
			this->groupBox17->TabStop = false;
			this->groupBox17->Text = L"Mean Mean Average Value (MMAV)";
			// 
			// label76
			// 
			this->label76->AutoSize = true;
			this->label76->Location = System::Drawing::Point(219, 22);
			this->label76->Name = L"label76";
			this->label76->Size = System::Drawing::Size(49, 13);
			this->label76->TabIndex = 22;
			this->label76->Text = L"AvgMAV";
			// 
			// checkBox5
			// 
			this->checkBox5->AutoSize = true;
			this->checkBox5->Checked = true;
			this->checkBox5->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox5->Location = System::Drawing::Point(8, 22);
			this->checkBox5->Name = L"checkBox5";
			this->checkBox5->Size = System::Drawing::Size(95, 17);
			this->checkBox5->TabIndex = 19;
			this->checkBox5->Text = L"Display MMAV";
			this->checkBox5->UseVisualStyleBackColor = true;
			// 
			// label40
			// 
			this->label40->AutoSize = true;
			this->label40->Location = System::Drawing::Point(107, 23);
			this->label40->Name = L"label40";
			this->label40->Size = System::Drawing::Size(54, 13);
			this->label40->TabIndex = 21;
			this->label40->Text = L"Threshold";
			// 
			// chart4
			// 
			this->chart4->BackColor = System::Drawing::Color::Transparent;
			this->chart4->BackSecondaryColor = System::Drawing::Color::White;
			this->chart4->BorderlineColor = System::Drawing::Color::Black;
			this->chart4->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chart4->Location = System::Drawing::Point(6, 42);
			this->chart4->Name = L"chart4";
			this->chart4->Size = System::Drawing::Size(344, 278);
			this->chart4->TabIndex = 18;
			this->chart4->Text = L"chart2";
			// 
			// numericUpDown1
			// 
			this->numericUpDown1->Location = System::Drawing::Point(161, 20);
			this->numericUpDown1->Name = L"numericUpDown1";
			this->numericUpDown1->Size = System::Drawing::Size(50, 20);
			this->numericUpDown1->TabIndex = 20;
			// 
			// tabPage3
			// 
			this->tabPage3->Controls->Add(this->label68);
			this->tabPage3->Controls->Add(this->numericUpDown5);
			this->tabPage3->Controls->Add(this->chart3);
			this->tabPage3->Controls->Add(this->label67);
			this->tabPage3->Controls->Add(this->label65);
			this->tabPage3->Controls->Add(this->label66);
			this->tabPage3->Controls->Add(this->label54);
			this->tabPage3->Controls->Add(this->comboBox7);
			this->tabPage3->Controls->Add(this->comboBox9);
			this->tabPage3->Controls->Add(this->comboBox8);
			this->tabPage3->Controls->Add(this->comboBox6);
			this->tabPage3->Controls->Add(this->numericUpDown3);
			this->tabPage3->Controls->Add(this->numericUpDown2);
			this->tabPage3->Controls->Add(this->chart2);
			this->tabPage3->Controls->Add(this->chart1);
			this->tabPage3->Controls->Add(this->groupBox11);
			this->tabPage3->Controls->Add(this->label78);
			this->tabPage3->Controls->Add(this->label27);
			this->tabPage3->Location = System::Drawing::Point(4, 40);
			this->tabPage3->Name = L"tabPage3";
			this->tabPage3->Padding = System::Windows::Forms::Padding(3);
			this->tabPage3->Size = System::Drawing::Size(1256, 637);
			this->tabPage3->TabIndex = 2;
			this->tabPage3->Text = L"EMG";
			this->tabPage3->UseVisualStyleBackColor = true;
			this->tabPage3->Enter += gcnew System::EventHandler(this, &MyForm::tabPage3_Enter);
			// 
			// label68
			// 
			this->label68->AutoSize = true;
			this->label68->Location = System::Drawing::Point(917, 167);
			this->label68->Name = L"label68";
			this->label68->Size = System::Drawing::Size(54, 13);
			this->label68->TabIndex = 141;
			this->label68->Text = L"Threshold";
			// 
			// numericUpDown5
			// 
			this->numericUpDown5->Location = System::Drawing::Point(977, 163);
			this->numericUpDown5->Name = L"numericUpDown5";
			this->numericUpDown5->Size = System::Drawing::Size(120, 20);
			this->numericUpDown5->TabIndex = 140;
			this->numericUpDown5->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->numericUpDown5->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown5_ValueChanged);
			// 
			// chart3
			// 
			this->chart3->BorderlineColor = System::Drawing::Color::Black;
			this->chart3->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chart3->Location = System::Drawing::Point(57, 188);
			this->chart3->Name = L"chart3";
			this->chart3->Size = System::Drawing::Size(383, 370);
			this->chart3->TabIndex = 138;
			this->chart3->Text = L"chart3";
			// 
			// label67
			// 
			this->label67->AutoSize = true;
			this->label67->Location = System::Drawing::Point(1014, 540);
			this->label67->Name = L"label67";
			this->label67->Size = System::Drawing::Size(46, 13);
			this->label67->TabIndex = 137;
			this->label67->Text = L"Data #2";
			// 
			// label65
			// 
			this->label65->AutoSize = true;
			this->label65->Location = System::Drawing::Point(617, 540);
			this->label65->Name = L"label65";
			this->label65->Size = System::Drawing::Size(46, 13);
			this->label65->TabIndex = 137;
			this->label65->Text = L"Data #2";
			// 
			// label66
			// 
			this->label66->AutoSize = true;
			this->label66->Location = System::Drawing::Point(1014, 192);
			this->label66->Name = L"label66";
			this->label66->Size = System::Drawing::Size(46, 13);
			this->label66->TabIndex = 137;
			this->label66->Text = L"Data #1";
			// 
			// label54
			// 
			this->label54->AutoSize = true;
			this->label54->Location = System::Drawing::Point(617, 192);
			this->label54->Name = L"label54";
			this->label54->Size = System::Drawing::Size(46, 13);
			this->label54->TabIndex = 137;
			this->label54->Text = L"Data #1";
			// 
			// comboBox7
			// 
			this->comboBox7->FormattingEnabled = true;
			this->comboBox7->Items->AddRange(gcnew cli::array< System::Object^  >(5) { L"WAVE", L"MAV", L"ZERO", L"TURN", L"MAV/MMAV" });
			this->comboBox7->Location = System::Drawing::Point(1064, 190);
			this->comboBox7->Name = L"comboBox7";
			this->comboBox7->Size = System::Drawing::Size(126, 21);
			this->comboBox7->TabIndex = 136;
			this->comboBox7->SelectionChangeCommitted += gcnew System::EventHandler(this, &MyForm::comboBox7_SelectionChangeCommitted);
			// 
			// comboBox9
			// 
			this->comboBox9->FormattingEnabled = true;
			this->comboBox9->Items->AddRange(gcnew cli::array< System::Object^  >(5) { L"WAVE", L"MAV", L"ZERO", L"TURN", L"MAV/MMAV" });
			this->comboBox9->Location = System::Drawing::Point(1064, 535);
			this->comboBox9->Name = L"comboBox9";
			this->comboBox9->Size = System::Drawing::Size(126, 21);
			this->comboBox9->TabIndex = 136;
			this->comboBox9->SelectedIndexChanged += gcnew System::EventHandler(this, &MyForm::comboBox9_SelectedIndexChanged);
			// 
			// comboBox8
			// 
			this->comboBox8->FormattingEnabled = true;
			this->comboBox8->Items->AddRange(gcnew cli::array< System::Object^  >(5) { L"WAVE", L"MAV", L"ZERO", L"TURN", L"MAV/MMAV" });
			this->comboBox8->Location = System::Drawing::Point(667, 535);
			this->comboBox8->Name = L"comboBox8";
			this->comboBox8->Size = System::Drawing::Size(147, 21);
			this->comboBox8->TabIndex = 136;
			this->comboBox8->SelectionChangeCommitted += gcnew System::EventHandler(this, &MyForm::comboBox8_SelectionChangeCommitted);
			// 
			// comboBox6
			// 
			this->comboBox6->FormattingEnabled = true;
			this->comboBox6->Items->AddRange(gcnew cli::array< System::Object^  >(5) { L"WAVE", L"MAV", L"ZERO", L"TURN", L"MAV/MMAV" });
			this->comboBox6->Location = System::Drawing::Point(667, 190);
			this->comboBox6->Name = L"comboBox6";
			this->comboBox6->Size = System::Drawing::Size(147, 21);
			this->comboBox6->TabIndex = 136;
			this->comboBox6->SelectionChangeCommitted += gcnew System::EventHandler(this, &MyForm::comboBox6_SelectionChangeCommitted);
			// 
			// numericUpDown3
			// 
			this->numericUpDown3->Location = System::Drawing::Point(290, 36);
			this->numericUpDown3->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 500, 0, 0, 0 });
			this->numericUpDown3->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDown3->Name = L"numericUpDown3";
			this->numericUpDown3->Size = System::Drawing::Size(93, 20);
			this->numericUpDown3->TabIndex = 135;
			this->numericUpDown3->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 40, 0, 0, 0 });
			this->numericUpDown3->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown3_ValueChanged);
			// 
			// numericUpDown2
			// 
			this->numericUpDown2->Location = System::Drawing::Point(156, 37);
			this->numericUpDown2->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 500, 0, 0, 0 });
			this->numericUpDown2->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDown2->Name = L"numericUpDown2";
			this->numericUpDown2->Size = System::Drawing::Size(93, 20);
			this->numericUpDown2->TabIndex = 134;
			this->numericUpDown2->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
			this->numericUpDown2->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown2_ValueChanged);
			// 
			// chart2
			// 
			this->chart2->BackColor = System::Drawing::Color::Transparent;
			this->chart2->BorderlineColor = System::Drawing::Color::Black;
			this->chart2->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chart2->Location = System::Drawing::Point(822, 188);
			this->chart2->Name = L"chart2";
			this->chart2->Size = System::Drawing::Size(370, 370);
			this->chart2->TabIndex = 132;
			this->chart2->Text = L"chart2";
			// 
			// chart1
			// 
			this->chart1->BackColor = System::Drawing::Color::Transparent;
			this->chart1->BorderlineColor = System::Drawing::Color::Black;
			this->chart1->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chart1->BorderSkin->BorderDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chart1->Location = System::Drawing::Point(446, 188);
			this->chart1->Name = L"chart1";
			this->chart1->Size = System::Drawing::Size(370, 370);
			this->chart1->TabIndex = 131;
			this->chart1->Text = L"chart2";
			// 
			// groupBox11
			// 
			this->groupBox11->Controls->Add(this->textBox54);
			this->groupBox11->Controls->Add(this->label148);
			this->groupBox11->Controls->Add(this->textBox53);
			this->groupBox11->Controls->Add(this->textBox52);
			this->groupBox11->Controls->Add(this->textBox15);
			this->groupBox11->Controls->Add(this->label147);
			this->groupBox11->Controls->Add(this->textBox51);
			this->groupBox11->Controls->Add(this->textBox50);
			this->groupBox11->Controls->Add(this->label137);
			this->groupBox11->Controls->Add(this->label146);
			this->groupBox11->Controls->Add(this->label138);
			this->groupBox11->Controls->Add(this->label144);
			this->groupBox11->Controls->Add(this->label141);
			this->groupBox11->Controls->Add(this->textBox49);
			this->groupBox11->Controls->Add(this->label142);
			this->groupBox11->Controls->Add(this->textBox48);
			this->groupBox11->Location = System::Drawing::Point(156, 62);
			this->groupBox11->Name = L"groupBox11";
			this->groupBox11->Size = System::Drawing::Size(942, 92);
			this->groupBox11->TabIndex = 127;
			this->groupBox11->TabStop = false;
			this->groupBox11->Text = L"Raw EMG";
			// 
			// textBox54
			// 
			this->textBox54->BackColor = System::Drawing::SystemColors::Window;
			this->textBox54->Location = System::Drawing::Point(50, 49);
			this->textBox54->Name = L"textBox54";
			this->textBox54->ReadOnly = true;
			this->textBox54->Size = System::Drawing::Size(100, 20);
			this->textBox54->TabIndex = 13;
			// 
			// label148
			// 
			this->label148->AutoSize = true;
			this->label148->Location = System::Drawing::Point(71, 33);
			this->label148->Name = L"label148";
			this->label148->Size = System::Drawing::Size(62, 13);
			this->label148->TabIndex = 24;
			this->label148->Text = L"Channel #1";
			// 
			// textBox53
			// 
			this->textBox53->BackColor = System::Drawing::SystemColors::Window;
			this->textBox53->Location = System::Drawing::Point(474, 49);
			this->textBox53->Name = L"textBox53";
			this->textBox53->ReadOnly = true;
			this->textBox53->Size = System::Drawing::Size(100, 20);
			this->textBox53->TabIndex = 14;
			// 
			// textBox52
			// 
			this->textBox52->BackColor = System::Drawing::SystemColors::Window;
			this->textBox52->Location = System::Drawing::Point(262, 49);
			this->textBox52->Name = L"textBox52";
			this->textBox52->ReadOnly = true;
			this->textBox52->Size = System::Drawing::Size(100, 20);
			this->textBox52->TabIndex = 15;
			// 
			// textBox15
			// 
			this->textBox15->BackColor = System::Drawing::SystemColors::Window;
			this->textBox15->Location = System::Drawing::Point(580, 49);
			this->textBox15->Name = L"textBox15";
			this->textBox15->ReadOnly = true;
			this->textBox15->Size = System::Drawing::Size(100, 20);
			this->textBox15->TabIndex = 14;
			// 
			// label147
			// 
			this->label147->AutoSize = true;
			this->label147->Location = System::Drawing::Point(496, 33);
			this->label147->Name = L"label147";
			this->label147->Size = System::Drawing::Size(62, 13);
			this->label147->TabIndex = 27;
			this->label147->Text = L"Channel #5";
			// 
			// textBox51
			// 
			this->textBox51->BackColor = System::Drawing::SystemColors::Window;
			this->textBox51->Location = System::Drawing::Point(156, 49);
			this->textBox51->Name = L"textBox51";
			this->textBox51->ReadOnly = true;
			this->textBox51->Size = System::Drawing::Size(100, 20);
			this->textBox51->TabIndex = 16;
			// 
			// textBox50
			// 
			this->textBox50->BackColor = System::Drawing::SystemColors::Window;
			this->textBox50->Location = System::Drawing::Point(686, 49);
			this->textBox50->Name = L"textBox50";
			this->textBox50->ReadOnly = true;
			this->textBox50->Size = System::Drawing::Size(100, 20);
			this->textBox50->TabIndex = 17;
			// 
			// label137
			// 
			this->label137->AutoSize = true;
			this->label137->Location = System::Drawing::Point(810, 33);
			this->label137->Name = L"label137";
			this->label137->Size = System::Drawing::Size(62, 13);
			this->label137->TabIndex = 20;
			this->label137->Text = L"Channel #8";
			// 
			// label146
			// 
			this->label146->AutoSize = true;
			this->label146->Location = System::Drawing::Point(281, 33);
			this->label146->Name = L"label146";
			this->label146->Size = System::Drawing::Size(62, 13);
			this->label146->TabIndex = 26;
			this->label146->Text = L"Channel #3";
			// 
			// label138
			// 
			this->label138->AutoSize = true;
			this->label138->Location = System::Drawing::Point(389, 33);
			this->label138->Name = L"label138";
			this->label138->Size = System::Drawing::Size(62, 13);
			this->label138->TabIndex = 21;
			this->label138->Text = L"Channel #4";
			// 
			// label144
			// 
			this->label144->AutoSize = true;
			this->label144->Location = System::Drawing::Point(704, 33);
			this->label144->Name = L"label144";
			this->label144->Size = System::Drawing::Size(62, 13);
			this->label144->TabIndex = 25;
			this->label144->Text = L"Channel #7";
			// 
			// label141
			// 
			this->label141->AutoSize = true;
			this->label141->Location = System::Drawing::Point(599, 33);
			this->label141->Name = L"label141";
			this->label141->Size = System::Drawing::Size(62, 13);
			this->label141->TabIndex = 22;
			this->label141->Text = L"Channel #6";
			// 
			// textBox49
			// 
			this->textBox49->BackColor = System::Drawing::SystemColors::Window;
			this->textBox49->Location = System::Drawing::Point(368, 49);
			this->textBox49->Name = L"textBox49";
			this->textBox49->ReadOnly = true;
			this->textBox49->Size = System::Drawing::Size(100, 20);
			this->textBox49->TabIndex = 19;
			// 
			// label142
			// 
			this->label142->AutoSize = true;
			this->label142->Location = System::Drawing::Point(173, 33);
			this->label142->Name = L"label142";
			this->label142->Size = System::Drawing::Size(62, 13);
			this->label142->TabIndex = 23;
			this->label142->Text = L"Channel #2";
			// 
			// textBox48
			// 
			this->textBox48->BackColor = System::Drawing::SystemColors::Window;
			this->textBox48->Location = System::Drawing::Point(792, 49);
			this->textBox48->Name = L"textBox48";
			this->textBox48->ReadOnly = true;
			this->textBox48->Size = System::Drawing::Size(100, 20);
			this->textBox48->TabIndex = 18;
			// 
			// label78
			// 
			this->label78->AutoSize = true;
			this->label78->Location = System::Drawing::Point(153, 20);
			this->label78->Name = L"label78";
			this->label78->Size = System::Drawing::Size(96, 13);
			this->label78->TabIndex = 12;
			this->label78->Text = L"Window Increment";
			// 
			// label27
			// 
			this->label27->AutoSize = true;
			this->label27->Location = System::Drawing::Point(287, 20);
			this->label27->Name = L"label27";
			this->label27->Size = System::Drawing::Size(82, 13);
			this->label27->TabIndex = 9;
			this->label27->Text = L"Window Length";
			// 
			// tabPageIMU
			// 
			this->tabPageIMU->Controls->Add(this->tableLayoutPanelIMU);
			this->tabPageIMU->Location = System::Drawing::Point(4, 40);
			this->tabPageIMU->Name = L"tabPageIMU";
			this->tabPageIMU->Padding = System::Windows::Forms::Padding(3);
			this->tabPageIMU->Size = System::Drawing::Size(1256, 637);
			this->tabPageIMU->TabIndex = 1;
			this->tabPageIMU->Text = L"IMU";
			this->tabPageIMU->UseVisualStyleBackColor = true;
			this->tabPageIMU->Enter += gcnew System::EventHandler(this, &MyForm::tabPageIMU_Enter);
			// 
			// tableLayoutPanelIMU
			// 
			this->tableLayoutPanelIMU->ColumnCount = 1;
			this->tableLayoutPanelIMU->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelIMU->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				20)));
			this->tableLayoutPanelIMU->Controls->Add(this->tableLayoutPanelIMUCenter, 0, 0);
			this->tableLayoutPanelIMU->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanelIMU->Location = System::Drawing::Point(3, 3);
			this->tableLayoutPanelIMU->Name = L"tableLayoutPanelIMU";
			this->tableLayoutPanelIMU->RowCount = 1;
			this->tableLayoutPanelIMU->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelIMU->Size = System::Drawing::Size(1250, 631);
			this->tableLayoutPanelIMU->TabIndex = 48;
			// 
			// tableLayoutPanelIMUCenter
			// 
			this->tableLayoutPanelIMUCenter->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->tableLayoutPanelIMUCenter->ColumnCount = 3;
			this->tableLayoutPanelIMUCenter->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				238)));
			this->tableLayoutPanelIMUCenter->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				280)));
			this->tableLayoutPanelIMUCenter->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelIMUCenter->Controls->Add(this->tableLayoutPanelIMUText, 0, 0);
			this->tableLayoutPanelIMUCenter->Controls->Add(this->panelAccelerationVector, 2, 0);
			this->tableLayoutPanelIMUCenter->Controls->Add(this->panelAttitudeHeading, 1, 0);
			this->tableLayoutPanelIMUCenter->Location = System::Drawing::Point(199, 153);
			this->tableLayoutPanelIMUCenter->MinimumSize = System::Drawing::Size(851, 324);
			this->tableLayoutPanelIMUCenter->Name = L"tableLayoutPanelIMUCenter";
			this->tableLayoutPanelIMUCenter->RowCount = 1;
			this->tableLayoutPanelIMUCenter->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelIMUCenter->Size = System::Drawing::Size(851, 324);
			this->tableLayoutPanelIMUCenter->TabIndex = 47;
			// 
			// tableLayoutPanelIMUText
			// 
			this->tableLayoutPanelIMUText->ColumnCount = 1;
			this->tableLayoutPanelIMUText->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle()));
			this->tableLayoutPanelIMUText->Controls->Add(this->groupBoxIMUGyroscope, 0, 2);
			this->tableLayoutPanelIMUText->Controls->Add(this->groupBoxIMUAccelerometer, 0, 1);
			this->tableLayoutPanelIMUText->Controls->Add(this->groupBoxIMUOrientation, 0, 0);
			this->tableLayoutPanelIMUText->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanelIMUText->Location = System::Drawing::Point(3, 3);
			this->tableLayoutPanelIMUText->Name = L"tableLayoutPanelIMUText";
			this->tableLayoutPanelIMUText->RowCount = 3;
			this->tableLayoutPanelIMUText->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				106)));
			this->tableLayoutPanelIMUText->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				106)));
			this->tableLayoutPanelIMUText->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				106)));
			this->tableLayoutPanelIMUText->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				20)));
			this->tableLayoutPanelIMUText->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				20)));
			this->tableLayoutPanelIMUText->Size = System::Drawing::Size(232, 318);
			this->tableLayoutPanelIMUText->TabIndex = 48;
			// 
			// groupBoxIMUGyroscope
			// 
			this->groupBoxIMUGyroscope->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyZUnit);
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyYUnit);
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyXUnit);
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyZ);
			this->groupBoxIMUGyroscope->Controls->Add(this->textBoxGyroX);
			this->groupBoxIMUGyroscope->Controls->Add(this->textBoxGyroY);
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyX);
			this->groupBoxIMUGyroscope->Controls->Add(this->labelIMUGyY);
			this->groupBoxIMUGyroscope->Controls->Add(this->textBoxGyroZ);
			this->groupBoxIMUGyroscope->Location = System::Drawing::Point(3, 215);
			this->groupBoxIMUGyroscope->Margin = System::Windows::Forms::Padding(0);
			this->groupBoxIMUGyroscope->Name = L"groupBoxIMUGyroscope";
			this->groupBoxIMUGyroscope->Size = System::Drawing::Size(226, 100);
			this->groupBoxIMUGyroscope->TabIndex = 1;
			this->groupBoxIMUGyroscope->TabStop = false;
			this->groupBoxIMUGyroscope->Text = L"Gyroscope";
			// 
			// labelIMUGyZUnit
			// 
			this->labelIMUGyZUnit->AutoSize = true;
			this->labelIMUGyZUnit->Location = System::Drawing::Point(153, 77);
			this->labelIMUGyZUnit->Name = L"labelIMUGyZUnit";
			this->labelIMUGyZUnit->Size = System::Drawing::Size(37, 13);
			this->labelIMUGyZUnit->TabIndex = 20;
			this->labelIMUGyZUnit->Text = L"Deg/s";
			// 
			// labelIMUGyYUnit
			// 
			this->labelIMUGyYUnit->AutoSize = true;
			this->labelIMUGyYUnit->Location = System::Drawing::Point(153, 51);
			this->labelIMUGyYUnit->Name = L"labelIMUGyYUnit";
			this->labelIMUGyYUnit->Size = System::Drawing::Size(37, 13);
			this->labelIMUGyYUnit->TabIndex = 19;
			this->labelIMUGyYUnit->Text = L"Deg/s";
			// 
			// labelIMUGyXUnit
			// 
			this->labelIMUGyXUnit->AutoSize = true;
			this->labelIMUGyXUnit->Location = System::Drawing::Point(153, 25);
			this->labelIMUGyXUnit->Name = L"labelIMUGyXUnit";
			this->labelIMUGyXUnit->Size = System::Drawing::Size(37, 13);
			this->labelIMUGyXUnit->TabIndex = 18;
			this->labelIMUGyXUnit->Text = L"Deg/s";
			// 
			// labelIMUGyZ
			// 
			this->labelIMUGyZ->AutoSize = true;
			this->labelIMUGyZ->Location = System::Drawing::Point(18, 77);
			this->labelIMUGyZ->Name = L"labelIMUGyZ";
			this->labelIMUGyZ->Size = System::Drawing::Size(14, 13);
			this->labelIMUGyZ->TabIndex = 17;
			this->labelIMUGyZ->Text = L"Z";
			// 
			// textBoxGyroX
			// 
			this->textBoxGyroX->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxGyroX->Location = System::Drawing::Point(49, 22);
			this->textBoxGyroX->Name = L"textBoxGyroX";
			this->textBoxGyroX->ReadOnly = true;
			this->textBoxGyroX->Size = System::Drawing::Size(100, 20);
			this->textBoxGyroX->TabIndex = 12;
			// 
			// textBoxGyroY
			// 
			this->textBoxGyroY->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxGyroY->Location = System::Drawing::Point(49, 48);
			this->textBoxGyroY->Name = L"textBoxGyroY";
			this->textBoxGyroY->ReadOnly = true;
			this->textBoxGyroY->Size = System::Drawing::Size(100, 20);
			this->textBoxGyroY->TabIndex = 14;
			// 
			// labelIMUGyX
			// 
			this->labelIMUGyX->AutoSize = true;
			this->labelIMUGyX->Location = System::Drawing::Point(18, 25);
			this->labelIMUGyX->Name = L"labelIMUGyX";
			this->labelIMUGyX->Size = System::Drawing::Size(14, 13);
			this->labelIMUGyX->TabIndex = 13;
			this->labelIMUGyX->Text = L"X";
			// 
			// labelIMUGyY
			// 
			this->labelIMUGyY->AutoSize = true;
			this->labelIMUGyY->Location = System::Drawing::Point(18, 51);
			this->labelIMUGyY->Name = L"labelIMUGyY";
			this->labelIMUGyY->Size = System::Drawing::Size(14, 13);
			this->labelIMUGyY->TabIndex = 15;
			this->labelIMUGyY->Text = L"Y";
			// 
			// textBoxGyroZ
			// 
			this->textBoxGyroZ->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxGyroZ->Location = System::Drawing::Point(49, 74);
			this->textBoxGyroZ->Name = L"textBoxGyroZ";
			this->textBoxGyroZ->ReadOnly = true;
			this->textBoxGyroZ->Size = System::Drawing::Size(100, 20);
			this->textBoxGyroZ->TabIndex = 16;
			// 
			// groupBoxIMUAccelerometer
			// 
			this->groupBoxIMUAccelerometer->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcZUnit);
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcYUnit);
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcXUnit);
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcZ);
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcX);
			this->groupBoxIMUAccelerometer->Controls->Add(this->textBoxAccelZ);
			this->groupBoxIMUAccelerometer->Controls->Add(this->textBoxAccelX);
			this->groupBoxIMUAccelerometer->Controls->Add(this->labelIMUAcY);
			this->groupBoxIMUAccelerometer->Controls->Add(this->textBoxAccelY);
			this->groupBoxIMUAccelerometer->Location = System::Drawing::Point(3, 109);
			this->groupBoxIMUAccelerometer->Margin = System::Windows::Forms::Padding(0);
			this->groupBoxIMUAccelerometer->Name = L"groupBoxIMUAccelerometer";
			this->groupBoxIMUAccelerometer->Size = System::Drawing::Size(226, 100);
			this->groupBoxIMUAccelerometer->TabIndex = 1;
			this->groupBoxIMUAccelerometer->TabStop = false;
			this->groupBoxIMUAccelerometer->Text = L"Accelerometer";
			// 
			// labelIMUAcZUnit
			// 
			this->labelIMUAcZUnit->AutoSize = true;
			this->labelIMUAcZUnit->Location = System::Drawing::Point(153, 77);
			this->labelIMUAcZUnit->Name = L"labelIMUAcZUnit";
			this->labelIMUAcZUnit->Size = System::Drawing::Size(20, 13);
			this->labelIMUAcZUnit->TabIndex = 14;
			this->labelIMUAcZUnit->Text = L"g\'s";
			// 
			// labelIMUAcYUnit
			// 
			this->labelIMUAcYUnit->AutoSize = true;
			this->labelIMUAcYUnit->Location = System::Drawing::Point(153, 51);
			this->labelIMUAcYUnit->Name = L"labelIMUAcYUnit";
			this->labelIMUAcYUnit->Size = System::Drawing::Size(20, 13);
			this->labelIMUAcYUnit->TabIndex = 13;
			this->labelIMUAcYUnit->Text = L"g\'s";
			// 
			// labelIMUAcXUnit
			// 
			this->labelIMUAcXUnit->AutoSize = true;
			this->labelIMUAcXUnit->Location = System::Drawing::Point(153, 25);
			this->labelIMUAcXUnit->Name = L"labelIMUAcXUnit";
			this->labelIMUAcXUnit->Size = System::Drawing::Size(20, 13);
			this->labelIMUAcXUnit->TabIndex = 12;
			this->labelIMUAcXUnit->Text = L"g\'s";
			// 
			// labelIMUAcZ
			// 
			this->labelIMUAcZ->AutoSize = true;
			this->labelIMUAcZ->Location = System::Drawing::Point(18, 77);
			this->labelIMUAcZ->Name = L"labelIMUAcZ";
			this->labelIMUAcZ->Size = System::Drawing::Size(14, 13);
			this->labelIMUAcZ->TabIndex = 11;
			this->labelIMUAcZ->Text = L"Z";
			// 
			// labelIMUAcX
			// 
			this->labelIMUAcX->AutoSize = true;
			this->labelIMUAcX->Location = System::Drawing::Point(18, 25);
			this->labelIMUAcX->Name = L"labelIMUAcX";
			this->labelIMUAcX->Size = System::Drawing::Size(14, 13);
			this->labelIMUAcX->TabIndex = 7;
			this->labelIMUAcX->Text = L"X";
			// 
			// textBoxAccelZ
			// 
			this->textBoxAccelZ->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxAccelZ->Location = System::Drawing::Point(49, 74);
			this->textBoxAccelZ->Name = L"textBoxAccelZ";
			this->textBoxAccelZ->ReadOnly = true;
			this->textBoxAccelZ->Size = System::Drawing::Size(100, 20);
			this->textBoxAccelZ->TabIndex = 10;
			// 
			// textBoxAccelX
			// 
			this->textBoxAccelX->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxAccelX->Location = System::Drawing::Point(49, 22);
			this->textBoxAccelX->Name = L"textBoxAccelX";
			this->textBoxAccelX->ReadOnly = true;
			this->textBoxAccelX->Size = System::Drawing::Size(100, 20);
			this->textBoxAccelX->TabIndex = 6;
			// 
			// labelIMUAcY
			// 
			this->labelIMUAcY->AutoSize = true;
			this->labelIMUAcY->Location = System::Drawing::Point(18, 51);
			this->labelIMUAcY->Name = L"labelIMUAcY";
			this->labelIMUAcY->Size = System::Drawing::Size(14, 13);
			this->labelIMUAcY->TabIndex = 9;
			this->labelIMUAcY->Text = L"Y";
			// 
			// textBoxAccelY
			// 
			this->textBoxAccelY->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxAccelY->Location = System::Drawing::Point(49, 48);
			this->textBoxAccelY->Name = L"textBoxAccelY";
			this->textBoxAccelY->ReadOnly = true;
			this->textBoxAccelY->Size = System::Drawing::Size(100, 20);
			this->textBoxAccelY->TabIndex = 8;
			// 
			// groupBoxIMUOrientation
			// 
			this->groupBoxIMUOrientation->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMUYawUnit);
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMUPitchUnit);
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMURollUnit);
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMUYaw);
			this->groupBoxIMUOrientation->Controls->Add(this->textBoxOrientYaw);
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMUPitch);
			this->groupBoxIMUOrientation->Controls->Add(this->textBoxOrientPitch);
			this->groupBoxIMUOrientation->Controls->Add(this->labelIMURoll);
			this->groupBoxIMUOrientation->Controls->Add(this->textBoxOrientRoll);
			this->groupBoxIMUOrientation->Location = System::Drawing::Point(3, 3);
			this->groupBoxIMUOrientation->Margin = System::Windows::Forms::Padding(0);
			this->groupBoxIMUOrientation->Name = L"groupBoxIMUOrientation";
			this->groupBoxIMUOrientation->Size = System::Drawing::Size(226, 100);
			this->groupBoxIMUOrientation->TabIndex = 0;
			this->groupBoxIMUOrientation->TabStop = false;
			this->groupBoxIMUOrientation->Text = L"Orientation";
			// 
			// labelIMUYawUnit
			// 
			this->labelIMUYawUnit->AutoSize = true;
			this->labelIMUYawUnit->Location = System::Drawing::Point(153, 77);
			this->labelIMUYawUnit->Name = L"labelIMUYawUnit";
			this->labelIMUYawUnit->Size = System::Drawing::Size(47, 13);
			this->labelIMUYawUnit->TabIndex = 8;
			this->labelIMUYawUnit->Text = L"Degrees";
			// 
			// labelIMUPitchUnit
			// 
			this->labelIMUPitchUnit->AutoSize = true;
			this->labelIMUPitchUnit->Location = System::Drawing::Point(153, 51);
			this->labelIMUPitchUnit->Name = L"labelIMUPitchUnit";
			this->labelIMUPitchUnit->Size = System::Drawing::Size(47, 13);
			this->labelIMUPitchUnit->TabIndex = 7;
			this->labelIMUPitchUnit->Text = L"Degrees";
			// 
			// labelIMURollUnit
			// 
			this->labelIMURollUnit->AutoSize = true;
			this->labelIMURollUnit->Location = System::Drawing::Point(153, 25);
			this->labelIMURollUnit->Name = L"labelIMURollUnit";
			this->labelIMURollUnit->Size = System::Drawing::Size(47, 13);
			this->labelIMURollUnit->TabIndex = 6;
			this->labelIMURollUnit->Text = L"Degrees";
			// 
			// labelIMUYaw
			// 
			this->labelIMUYaw->AutoSize = true;
			this->labelIMUYaw->Location = System::Drawing::Point(13, 77);
			this->labelIMUYaw->Name = L"labelIMUYaw";
			this->labelIMUYaw->Size = System::Drawing::Size(28, 13);
			this->labelIMUYaw->TabIndex = 5;
			this->labelIMUYaw->Text = L"Yaw";
			// 
			// textBoxOrientYaw
			// 
			this->textBoxOrientYaw->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxOrientYaw->Location = System::Drawing::Point(49, 74);
			this->textBoxOrientYaw->Name = L"textBoxOrientYaw";
			this->textBoxOrientYaw->ReadOnly = true;
			this->textBoxOrientYaw->Size = System::Drawing::Size(100, 20);
			this->textBoxOrientYaw->TabIndex = 4;
			// 
			// labelIMUPitch
			// 
			this->labelIMUPitch->AutoSize = true;
			this->labelIMUPitch->Location = System::Drawing::Point(13, 51);
			this->labelIMUPitch->Name = L"labelIMUPitch";
			this->labelIMUPitch->Size = System::Drawing::Size(31, 13);
			this->labelIMUPitch->TabIndex = 3;
			this->labelIMUPitch->Text = L"Pitch";
			// 
			// textBoxOrientPitch
			// 
			this->textBoxOrientPitch->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxOrientPitch->Location = System::Drawing::Point(49, 48);
			this->textBoxOrientPitch->Name = L"textBoxOrientPitch";
			this->textBoxOrientPitch->ReadOnly = true;
			this->textBoxOrientPitch->Size = System::Drawing::Size(100, 20);
			this->textBoxOrientPitch->TabIndex = 2;
			// 
			// labelIMURoll
			// 
			this->labelIMURoll->AutoSize = true;
			this->labelIMURoll->Location = System::Drawing::Point(13, 25);
			this->labelIMURoll->Name = L"labelIMURoll";
			this->labelIMURoll->Size = System::Drawing::Size(25, 13);
			this->labelIMURoll->TabIndex = 1;
			this->labelIMURoll->Text = L"Roll";
			// 
			// textBoxOrientRoll
			// 
			this->textBoxOrientRoll->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxOrientRoll->Location = System::Drawing::Point(49, 22);
			this->textBoxOrientRoll->Name = L"textBoxOrientRoll";
			this->textBoxOrientRoll->ReadOnly = true;
			this->textBoxOrientRoll->Size = System::Drawing::Size(100, 20);
			this->textBoxOrientRoll->TabIndex = 0;
			// 
			// panelAccelerationVector
			// 
			this->panelAccelerationVector->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->panelAccelerationVector->Controls->Add(this->pictureBoxAccelXY);
			this->panelAccelerationVector->Controls->Add(this->pictureBoxAccelZ);
			this->panelAccelerationVector->Controls->Add(this->labelAccelVert);
			this->panelAccelerationVector->Controls->Add(this->labelAccelPlaneX);
			this->panelAccelerationVector->Controls->Add(this->labelAccelPlaneY);
			this->panelAccelerationVector->Location = System::Drawing::Point(528, 16);
			this->panelAccelerationVector->Name = L"panelAccelerationVector";
			this->panelAccelerationVector->Size = System::Drawing::Size(313, 292);
			this->panelAccelerationVector->TabIndex = 46;
			// 
			// pictureBoxAccelXY
			// 
			this->pictureBoxAccelXY->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBoxAccelXY->Location = System::Drawing::Point(23, 39);
			this->pictureBoxAccelXY->Name = L"pictureBoxAccelXY";
			this->pictureBoxAccelXY->Size = System::Drawing::Size(250, 250);
			this->pictureBoxAccelXY->TabIndex = 39;
			this->pictureBoxAccelXY->TabStop = false;
			// 
			// pictureBoxAccelZ
			// 
			this->pictureBoxAccelZ->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBoxAccelZ->Location = System::Drawing::Point(280, 39);
			this->pictureBoxAccelZ->Name = L"pictureBoxAccelZ";
			this->pictureBoxAccelZ->Size = System::Drawing::Size(30, 250);
			this->pictureBoxAccelZ->TabIndex = 41;
			this->pictureBoxAccelZ->TabStop = false;
			// 
			// labelAccelVert
			// 
			this->labelAccelVert->AutoSize = true;
			this->labelAccelVert->Location = System::Drawing::Point(287, 23);
			this->labelAccelVert->Name = L"labelAccelVert";
			this->labelAccelVert->Size = System::Drawing::Size(14, 13);
			this->labelAccelVert->TabIndex = 44;
			this->labelAccelVert->Text = L"X";
			// 
			// labelAccelPlaneX
			// 
			this->labelAccelPlaneX->AutoSize = true;
			this->labelAccelPlaneX->Location = System::Drawing::Point(3, 151);
			this->labelAccelPlaneX->Name = L"labelAccelPlaneX";
			this->labelAccelPlaneX->Size = System::Drawing::Size(14, 13);
			this->labelAccelPlaneX->TabIndex = 42;
			this->labelAccelPlaneX->Text = L"Y";
			// 
			// labelAccelPlaneY
			// 
			this->labelAccelPlaneY->AutoSize = true;
			this->labelAccelPlaneY->Location = System::Drawing::Point(143, 23);
			this->labelAccelPlaneY->Name = L"labelAccelPlaneY";
			this->labelAccelPlaneY->Size = System::Drawing::Size(14, 13);
			this->labelAccelPlaneY->TabIndex = 43;
			this->labelAccelPlaneY->Text = L"Z";
			// 
			// panelAttitudeHeading
			// 
			this->panelAttitudeHeading->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->panelAttitudeHeading->Controls->Add(this->pictureBoxHeading);
			this->panelAttitudeHeading->Controls->Add(this->pictureBoxAttitude);
			this->panelAttitudeHeading->Location = System::Drawing::Point(250, 16);
			this->panelAttitudeHeading->Name = L"panelAttitudeHeading";
			this->panelAttitudeHeading->Size = System::Drawing::Size(256, 292);
			this->panelAttitudeHeading->TabIndex = 45;
			// 
			// pictureBoxHeading
			// 
			this->pictureBoxHeading->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBoxHeading->Location = System::Drawing::Point(3, 3);
			this->pictureBoxHeading->Name = L"pictureBoxHeading";
			this->pictureBoxHeading->Size = System::Drawing::Size(250, 30);
			this->pictureBoxHeading->TabIndex = 40;
			this->pictureBoxHeading->TabStop = false;
			// 
			// pictureBoxAttitude
			// 
			this->pictureBoxAttitude->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->pictureBoxAttitude->Location = System::Drawing::Point(3, 39);
			this->pictureBoxAttitude->Name = L"pictureBoxAttitude";
			this->pictureBoxAttitude->Size = System::Drawing::Size(250, 250);
			this->pictureBoxAttitude->TabIndex = 38;
			this->pictureBoxAttitude->TabStop = false;
			// 
			// tabPageInput
			// 
			this->tabPageInput->Controls->Add(this->tableLayoutPanelInput);
			this->tabPageInput->Location = System::Drawing::Point(4, 40);
			this->tabPageInput->Name = L"tabPageInput";
			this->tabPageInput->Padding = System::Windows::Forms::Padding(3);
			this->tabPageInput->Size = System::Drawing::Size(1256, 637);
			this->tabPageInput->TabIndex = 0;
			this->tabPageInput->Text = L"Input";
			this->tabPageInput->UseVisualStyleBackColor = true;
			this->tabPageInput->Enter += gcnew System::EventHandler(this, &MyForm::tabPageInput_Enter);
			// 
			// tableLayoutPanelInput
			// 
			this->tableLayoutPanelInput->ColumnCount = 1;
			this->tableLayoutPanelInput->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelInput->Controls->Add(this->flowLayoutPanelInputCenter, 0, 0);
			this->tableLayoutPanelInput->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanelInput->Location = System::Drawing::Point(3, 3);
			this->tableLayoutPanelInput->Name = L"tableLayoutPanelInput";
			this->tableLayoutPanelInput->RowCount = 1;
			this->tableLayoutPanelInput->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelInput->Size = System::Drawing::Size(1250, 631);
			this->tableLayoutPanelInput->TabIndex = 13;
			// 
			// flowLayoutPanelInputCenter
			// 
			this->flowLayoutPanelInputCenter->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->flowLayoutPanelInputCenter->Controls->Add(this->groupBoxInputMyo);
			this->flowLayoutPanelInputCenter->Controls->Add(this->flowLayoutPanelInputData);
			this->flowLayoutPanelInputCenter->FlowDirection = System::Windows::Forms::FlowDirection::TopDown;
			this->flowLayoutPanelInputCenter->Location = System::Drawing::Point(280, 57);
			this->flowLayoutPanelInputCenter->MinimumSize = System::Drawing::Size(685, 0);
			this->flowLayoutPanelInputCenter->Name = L"flowLayoutPanelInputCenter";
			this->flowLayoutPanelInputCenter->Size = System::Drawing::Size(689, 516);
			this->flowLayoutPanelInputCenter->TabIndex = 12;
			// 
			// groupBoxInputMyo
			// 
			this->groupBoxInputMyo->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxInputMyo->Controls->Add(this->tableLayoutPanelInputMyo);
			this->groupBoxInputMyo->Location = System::Drawing::Point(10, 10);
			this->groupBoxInputMyo->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxInputMyo->Name = L"groupBoxInputMyo";
			this->groupBoxInputMyo->Size = System::Drawing::Size(669, 274);
			this->groupBoxInputMyo->TabIndex = 8;
			this->groupBoxInputMyo->TabStop = false;
			this->groupBoxInputMyo->Text = L"Myo Connections";
			// 
			// tableLayoutPanelInputMyo
			// 
			this->tableLayoutPanelInputMyo->ColumnCount = 2;
			this->tableLayoutPanelInputMyo->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelInputMyo->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelInputMyo->Controls->Add(this->groupBoxMyo2, 0, 0);
			this->tableLayoutPanelInputMyo->Controls->Add(this->groupBoxMyo1, 0, 0);
			this->tableLayoutPanelInputMyo->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanelInputMyo->Location = System::Drawing::Point(3, 16);
			this->tableLayoutPanelInputMyo->Name = L"tableLayoutPanelInputMyo";
			this->tableLayoutPanelInputMyo->RowCount = 1;
			this->tableLayoutPanelInputMyo->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanelInputMyo->Size = System::Drawing::Size(663, 255);
			this->tableLayoutPanelInputMyo->TabIndex = 11;
			// 
			// groupBoxMyo2
			// 
			this->groupBoxMyo2->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxMyo2->Controls->Add(this->flowLayoutPanelMyo2);
			this->groupBoxMyo2->Enabled = false;
			this->groupBoxMyo2->Location = System::Drawing::Point(365, 17);
			this->groupBoxMyo2->Margin = System::Windows::Forms::Padding(0);
			this->groupBoxMyo2->Name = L"groupBoxMyo2";
			this->groupBoxMyo2->Size = System::Drawing::Size(264, 221);
			this->groupBoxMyo2->TabIndex = 11;
			this->groupBoxMyo2->TabStop = false;
			this->groupBoxMyo2->Text = L"Secondary Myo (IMU only)";
			// 
			// flowLayoutPanelMyo2
			// 
			this->flowLayoutPanelMyo2->Controls->Add(this->labelMyo2ConnectionStatus);
			this->flowLayoutPanelMyo2->Controls->Add(this->buttonMyo2Connect);
			this->flowLayoutPanelMyo2->Controls->Add(this->labelMyo2StreamingStatus);
			this->flowLayoutPanelMyo2->Controls->Add(this->buttonMyo2Stream);
			this->flowLayoutPanelMyo2->Controls->Add(this->tableLayoutPanelMyo2Controls);
			this->flowLayoutPanelMyo2->Controls->Add(this->panelMyo2Battery);
			this->flowLayoutPanelMyo2->Location = System::Drawing::Point(3, 16);
			this->flowLayoutPanelMyo2->Margin = System::Windows::Forms::Padding(0);
			this->flowLayoutPanelMyo2->Name = L"flowLayoutPanelMyo2";
			this->flowLayoutPanelMyo2->Size = System::Drawing::Size(258, 202);
			this->flowLayoutPanelMyo2->TabIndex = 9;
			// 
			// labelMyo2ConnectionStatus
			// 
			this->labelMyo2ConnectionStatus->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->labelMyo2ConnectionStatus->Location = System::Drawing::Point(6, 23);
			this->labelMyo2ConnectionStatus->Margin = System::Windows::Forms::Padding(6, 10, 3, 3);
			this->labelMyo2ConnectionStatus->Name = L"labelMyo2ConnectionStatus";
			this->labelMyo2ConnectionStatus->Size = System::Drawing::Size(120, 26);
			this->labelMyo2ConnectionStatus->TabIndex = 15;
			this->labelMyo2ConnectionStatus->Text = L"Disconnected";
			this->labelMyo2ConnectionStatus->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// buttonMyo2Connect
			// 
			this->buttonMyo2Connect->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo2Connect->Location = System::Drawing::Point(132, 10);
			this->buttonMyo2Connect->Margin = System::Windows::Forms::Padding(3, 10, 6, 3);
			this->buttonMyo2Connect->Name = L"buttonMyo2Connect";
			this->buttonMyo2Connect->Size = System::Drawing::Size(120, 52);
			this->buttonMyo2Connect->TabIndex = 6;
			this->buttonMyo2Connect->Text = L"Connect";
			this->buttonMyo2Connect->UseVisualStyleBackColor = true;
			// 
			// labelMyo2StreamingStatus
			// 
			this->labelMyo2StreamingStatus->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->labelMyo2StreamingStatus->Location = System::Drawing::Point(6, 68);
			this->labelMyo2StreamingStatus->Margin = System::Windows::Forms::Padding(6, 3, 3, 10);
			this->labelMyo2StreamingStatus->Name = L"labelMyo2StreamingStatus";
			this->labelMyo2StreamingStatus->Size = System::Drawing::Size(120, 26);
			this->labelMyo2StreamingStatus->TabIndex = 15;
			this->labelMyo2StreamingStatus->Text = L"Not streaming";
			this->labelMyo2StreamingStatus->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// buttonMyo2Stream
			// 
			this->buttonMyo2Stream->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo2Stream->Enabled = false;
			this->buttonMyo2Stream->Location = System::Drawing::Point(132, 68);
			this->buttonMyo2Stream->Margin = System::Windows::Forms::Padding(3, 3, 6, 10);
			this->buttonMyo2Stream->Name = L"buttonMyo2Stream";
			this->buttonMyo2Stream->Size = System::Drawing::Size(120, 26);
			this->buttonMyo2Stream->TabIndex = 8;
			this->buttonMyo2Stream->Text = L"Stream EMG";
			this->buttonMyo2Stream->UseVisualStyleBackColor = true;
			// 
			// tableLayoutPanelMyo2Controls
			// 
			this->tableLayoutPanelMyo2Controls->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->tableLayoutPanelMyo2Controls->ColumnCount = 2;
			this->tableLayoutPanelMyo2Controls->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo2Controls->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo2Controls->Controls->Add(this->buttonMyo2Vibrate, 1, 0);
			this->tableLayoutPanelMyo2Controls->Controls->Add(this->buttonMyo2Center, 0, 0);
			this->tableLayoutPanelMyo2Controls->Location = System::Drawing::Point(3, 107);
			this->tableLayoutPanelMyo2Controls->Name = L"tableLayoutPanelMyo2Controls";
			this->tableLayoutPanelMyo2Controls->RowCount = 1;
			this->tableLayoutPanelMyo2Controls->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo2Controls->Size = System::Drawing::Size(252, 32);
			this->tableLayoutPanelMyo2Controls->TabIndex = 12;
			// 
			// buttonMyo2Vibrate
			// 
			this->buttonMyo2Vibrate->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo2Vibrate->Enabled = false;
			this->buttonMyo2Vibrate->Location = System::Drawing::Point(129, 3);
			this->buttonMyo2Vibrate->Name = L"buttonMyo2Vibrate";
			this->buttonMyo2Vibrate->Size = System::Drawing::Size(120, 25);
			this->buttonMyo2Vibrate->TabIndex = 5;
			this->buttonMyo2Vibrate->Text = L"Vibrate";
			this->buttonMyo2Vibrate->UseVisualStyleBackColor = true;
			// 
			// buttonMyo2Center
			// 
			this->buttonMyo2Center->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo2Center->Enabled = false;
			this->buttonMyo2Center->Location = System::Drawing::Point(3, 3);
			this->buttonMyo2Center->Name = L"buttonMyo2Center";
			this->buttonMyo2Center->Size = System::Drawing::Size(120, 25);
			this->buttonMyo2Center->TabIndex = 7;
			this->buttonMyo2Center->Text = L"Center";
			this->buttonMyo2Center->UseVisualStyleBackColor = true;
			// 
			// panelMyo2Battery
			// 
			this->panelMyo2Battery->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->panelMyo2Battery->Controls->Add(this->progressBarMyo2Battery);
			this->panelMyo2Battery->Controls->Add(this->labelMyo2Battery);
			this->panelMyo2Battery->Location = System::Drawing::Point(3, 145);
			this->panelMyo2Battery->Name = L"panelMyo2Battery";
			this->panelMyo2Battery->Size = System::Drawing::Size(252, 29);
			this->panelMyo2Battery->TabIndex = 11;
			// 
			// progressBarMyo2Battery
			// 
			this->progressBarMyo2Battery->Location = System::Drawing::Point(51, 3);
			this->progressBarMyo2Battery->Name = L"progressBarMyo2Battery";
			this->progressBarMyo2Battery->Size = System::Drawing::Size(198, 23);
			this->progressBarMyo2Battery->TabIndex = 3;
			// 
			// labelMyo2Battery
			// 
			this->labelMyo2Battery->AutoSize = true;
			this->labelMyo2Battery->Location = System::Drawing::Point(5, 8);
			this->labelMyo2Battery->Name = L"labelMyo2Battery";
			this->labelMyo2Battery->Size = System::Drawing::Size(40, 13);
			this->labelMyo2Battery->TabIndex = 4;
			this->labelMyo2Battery->Text = L"Battery";
			// 
			// groupBoxMyo1
			// 
			this->groupBoxMyo1->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->groupBoxMyo1->Controls->Add(this->flowLayoutPanelMyo1);
			this->groupBoxMyo1->Location = System::Drawing::Point(33, 17);
			this->groupBoxMyo1->Margin = System::Windows::Forms::Padding(0);
			this->groupBoxMyo1->Name = L"groupBoxMyo1";
			this->groupBoxMyo1->Size = System::Drawing::Size(264, 221);
			this->groupBoxMyo1->TabIndex = 10;
			this->groupBoxMyo1->TabStop = false;
			this->groupBoxMyo1->Text = L"Primary Myo";
			// 
			// flowLayoutPanelMyo1
			// 
			this->flowLayoutPanelMyo1->Controls->Add(this->labelMyo1ConnectionStatus);
			this->flowLayoutPanelMyo1->Controls->Add(this->buttonMyo1Connect);
			this->flowLayoutPanelMyo1->Controls->Add(this->labelMyo1StreamingStatus);
			this->flowLayoutPanelMyo1->Controls->Add(this->buttonMyo1Stream);
			this->flowLayoutPanelMyo1->Controls->Add(this->tableLayoutPanelMyo1Controls);
			this->flowLayoutPanelMyo1->Controls->Add(this->panelMyo1Battery);
			this->flowLayoutPanelMyo1->Location = System::Drawing::Point(3, 16);
			this->flowLayoutPanelMyo1->Margin = System::Windows::Forms::Padding(0);
			this->flowLayoutPanelMyo1->Name = L"flowLayoutPanelMyo1";
			this->flowLayoutPanelMyo1->Size = System::Drawing::Size(258, 202);
			this->flowLayoutPanelMyo1->TabIndex = 9;
			// 
			// labelMyo1ConnectionStatus
			// 
			this->labelMyo1ConnectionStatus->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->labelMyo1ConnectionStatus->Location = System::Drawing::Point(6, 23);
			this->labelMyo1ConnectionStatus->Margin = System::Windows::Forms::Padding(6, 10, 3, 3);
			this->labelMyo1ConnectionStatus->Name = L"labelMyo1ConnectionStatus";
			this->labelMyo1ConnectionStatus->Size = System::Drawing::Size(120, 26);
			this->labelMyo1ConnectionStatus->TabIndex = 13;
			this->labelMyo1ConnectionStatus->Text = L"Disconnected";
			this->labelMyo1ConnectionStatus->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// buttonMyo1Connect
			// 
			this->buttonMyo1Connect->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo1Connect->Location = System::Drawing::Point(132, 10);
			this->buttonMyo1Connect->Margin = System::Windows::Forms::Padding(3, 10, 6, 3);
			this->buttonMyo1Connect->Name = L"buttonMyo1Connect";
			this->buttonMyo1Connect->Size = System::Drawing::Size(120, 52);
			this->buttonMyo1Connect->TabIndex = 6;
			this->buttonMyo1Connect->Text = L"Connect";
			this->buttonMyo1Connect->UseVisualStyleBackColor = true;
			this->buttonMyo1Connect->Click += gcnew System::EventHandler(this, &MyForm::buttonMyo1Connect_Click);
			// 
			// labelMyo1StreamingStatus
			// 
			this->labelMyo1StreamingStatus->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->labelMyo1StreamingStatus->Location = System::Drawing::Point(6, 68);
			this->labelMyo1StreamingStatus->Margin = System::Windows::Forms::Padding(6, 3, 3, 10);
			this->labelMyo1StreamingStatus->Name = L"labelMyo1StreamingStatus";
			this->labelMyo1StreamingStatus->Size = System::Drawing::Size(120, 26);
			this->labelMyo1StreamingStatus->TabIndex = 14;
			this->labelMyo1StreamingStatus->Text = L"Not streaming EMG";
			this->labelMyo1StreamingStatus->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// buttonMyo1Stream
			// 
			this->buttonMyo1Stream->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo1Stream->Enabled = false;
			this->buttonMyo1Stream->Location = System::Drawing::Point(132, 68);
			this->buttonMyo1Stream->Margin = System::Windows::Forms::Padding(3, 3, 6, 10);
			this->buttonMyo1Stream->Name = L"buttonMyo1Stream";
			this->buttonMyo1Stream->Size = System::Drawing::Size(120, 26);
			this->buttonMyo1Stream->TabIndex = 8;
			this->buttonMyo1Stream->Text = L"Enable";
			this->buttonMyo1Stream->UseVisualStyleBackColor = true;
			this->buttonMyo1Stream->Click += gcnew System::EventHandler(this, &MyForm::buttonMyo1Stream_Click);
			// 
			// tableLayoutPanelMyo1Controls
			// 
			this->tableLayoutPanelMyo1Controls->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->tableLayoutPanelMyo1Controls->ColumnCount = 2;
			this->tableLayoutPanelMyo1Controls->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo1Controls->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo1Controls->Controls->Add(this->buttonMyo1Vibrate, 1, 0);
			this->tableLayoutPanelMyo1Controls->Controls->Add(this->buttonMyo1Center, 0, 0);
			this->tableLayoutPanelMyo1Controls->Location = System::Drawing::Point(3, 107);
			this->tableLayoutPanelMyo1Controls->Name = L"tableLayoutPanelMyo1Controls";
			this->tableLayoutPanelMyo1Controls->RowCount = 1;
			this->tableLayoutPanelMyo1Controls->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent,
				50)));
			this->tableLayoutPanelMyo1Controls->Size = System::Drawing::Size(252, 32);
			this->tableLayoutPanelMyo1Controls->TabIndex = 12;
			// 
			// buttonMyo1Vibrate
			// 
			this->buttonMyo1Vibrate->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo1Vibrate->Location = System::Drawing::Point(129, 3);
			this->buttonMyo1Vibrate->Name = L"buttonMyo1Vibrate";
			this->buttonMyo1Vibrate->Size = System::Drawing::Size(120, 25);
			this->buttonMyo1Vibrate->TabIndex = 5;
			this->buttonMyo1Vibrate->Text = L"Vibrate";
			this->buttonMyo1Vibrate->UseVisualStyleBackColor = true;
			this->buttonMyo1Vibrate->Click += gcnew System::EventHandler(this, &MyForm::buttonMyo1Vibrate_Click);
			// 
			// buttonMyo1Center
			// 
			this->buttonMyo1Center->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->buttonMyo1Center->Location = System::Drawing::Point(3, 3);
			this->buttonMyo1Center->Name = L"buttonMyo1Center";
			this->buttonMyo1Center->Size = System::Drawing::Size(120, 25);
			this->buttonMyo1Center->TabIndex = 7;
			this->buttonMyo1Center->Text = L"Center";
			this->buttonMyo1Center->UseVisualStyleBackColor = true;
			this->buttonMyo1Center->Click += gcnew System::EventHandler(this, &MyForm::buttonMyo1Center_Click);
			// 
			// panelMyo1Battery
			// 
			this->panelMyo1Battery->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->panelMyo1Battery->Controls->Add(this->progressBarMyo1Battery);
			this->panelMyo1Battery->Controls->Add(this->labelMyo1Battery);
			this->panelMyo1Battery->Location = System::Drawing::Point(3, 145);
			this->panelMyo1Battery->Name = L"panelMyo1Battery";
			this->panelMyo1Battery->Size = System::Drawing::Size(252, 29);
			this->panelMyo1Battery->TabIndex = 11;
			// 
			// progressBarMyo1Battery
			// 
			this->progressBarMyo1Battery->Location = System::Drawing::Point(51, 3);
			this->progressBarMyo1Battery->Name = L"progressBarMyo1Battery";
			this->progressBarMyo1Battery->Size = System::Drawing::Size(198, 23);
			this->progressBarMyo1Battery->TabIndex = 3;
			// 
			// labelMyo1Battery
			// 
			this->labelMyo1Battery->AutoSize = true;
			this->labelMyo1Battery->Location = System::Drawing::Point(5, 8);
			this->labelMyo1Battery->Name = L"labelMyo1Battery";
			this->labelMyo1Battery->Size = System::Drawing::Size(40, 13);
			this->labelMyo1Battery->TabIndex = 4;
			this->labelMyo1Battery->Text = L"Battery";
			// 
			// flowLayoutPanelInputData
			// 
			this->flowLayoutPanelInputData->Anchor = System::Windows::Forms::AnchorStyles::None;
			this->flowLayoutPanelInputData->Controls->Add(this->groupBoxPlayback);
			this->flowLayoutPanelInputData->Controls->Add(this->groupBoxRecording);
			this->flowLayoutPanelInputData->Location = System::Drawing::Point(2, 294);
			this->flowLayoutPanelInputData->Margin = System::Windows::Forms::Padding(0);
			this->flowLayoutPanelInputData->Name = L"flowLayoutPanelInputData";
			this->flowLayoutPanelInputData->Size = System::Drawing::Size(685, 220);
			this->flowLayoutPanelInputData->TabIndex = 11;
			// 
			// groupBoxPlayback
			// 
			this->groupBoxPlayback->Controls->Add(this->labelPlaybackTime);
			this->groupBoxPlayback->Controls->Add(this->progressBarPlayback);
			this->groupBoxPlayback->Controls->Add(this->checkBoxPlaybackRepeat);
			this->groupBoxPlayback->Controls->Add(this->buttonPlaybackLoad);
			this->groupBoxPlayback->Controls->Add(this->textBoxPlaybackFilename);
			this->groupBoxPlayback->Controls->Add(this->buttonPlaybackPlay);
			this->groupBoxPlayback->Location = System::Drawing::Point(10, 10);
			this->groupBoxPlayback->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxPlayback->Name = L"groupBoxPlayback";
			this->groupBoxPlayback->Size = System::Drawing::Size(286, 200);
			this->groupBoxPlayback->TabIndex = 10;
			this->groupBoxPlayback->TabStop = false;
			this->groupBoxPlayback->Text = L"Playback";
			// 
			// labelPlaybackTime
			// 
			this->labelPlaybackTime->AutoSize = true;
			this->labelPlaybackTime->BackColor = System::Drawing::Color::Transparent;
			this->labelPlaybackTime->Location = System::Drawing::Point(238, 77);
			this->labelPlaybackTime->Name = L"labelPlaybackTime";
			this->labelPlaybackTime->Size = System::Drawing::Size(28, 13);
			this->labelPlaybackTime->TabIndex = 11;
			this->labelPlaybackTime->Text = L"0:00";
			// 
			// progressBarPlayback
			// 
			this->progressBarPlayback->Location = System::Drawing::Point(13, 72);
			this->progressBarPlayback->Name = L"progressBarPlayback";
			this->progressBarPlayback->Size = System::Drawing::Size(219, 23);
			this->progressBarPlayback->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBarPlayback->TabIndex = 10;
			// 
			// checkBoxPlaybackRepeat
			// 
			this->checkBoxPlaybackRepeat->Appearance = System::Windows::Forms::Appearance::Button;
			this->checkBoxPlaybackRepeat->Location = System::Drawing::Point(153, 108);
			this->checkBoxPlaybackRepeat->Margin = System::Windows::Forms::Padding(10);
			this->checkBoxPlaybackRepeat->Name = L"checkBoxPlaybackRepeat";
			this->checkBoxPlaybackRepeat->Size = System::Drawing::Size(120, 26);
			this->checkBoxPlaybackRepeat->TabIndex = 8;
			this->checkBoxPlaybackRepeat->Text = L"Repeat";
			this->checkBoxPlaybackRepeat->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			this->checkBoxPlaybackRepeat->UseVisualStyleBackColor = true;
			// 
			// buttonPlaybackLoad
			// 
			this->buttonPlaybackLoad->Location = System::Drawing::Point(13, 26);
			this->buttonPlaybackLoad->Margin = System::Windows::Forms::Padding(10);
			this->buttonPlaybackLoad->Name = L"buttonPlaybackLoad";
			this->buttonPlaybackLoad->Size = System::Drawing::Size(120, 26);
			this->buttonPlaybackLoad->TabIndex = 5;
			this->buttonPlaybackLoad->Text = L"Load EMG File";
			this->buttonPlaybackLoad->UseVisualStyleBackColor = true;
			this->buttonPlaybackLoad->Click += gcnew System::EventHandler(this, &MyForm::buttonPlaybackLoad_Click);
			// 
			// textBoxPlaybackFilename
			// 
			this->textBoxPlaybackFilename->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxPlaybackFilename->Cursor = System::Windows::Forms::Cursors::Arrow;
			this->textBoxPlaybackFilename->Location = System::Drawing::Point(146, 30);
			this->textBoxPlaybackFilename->Name = L"textBoxPlaybackFilename";
			this->textBoxPlaybackFilename->ReadOnly = true;
			this->textBoxPlaybackFilename->Size = System::Drawing::Size(127, 20);
			this->textBoxPlaybackFilename->TabIndex = 7;
			// 
			// buttonPlaybackPlay
			// 
			this->buttonPlaybackPlay->Enabled = false;
			this->buttonPlaybackPlay->Location = System::Drawing::Point(13, 108);
			this->buttonPlaybackPlay->Margin = System::Windows::Forms::Padding(10);
			this->buttonPlaybackPlay->Name = L"buttonPlaybackPlay";
			this->buttonPlaybackPlay->Size = System::Drawing::Size(120, 26);
			this->buttonPlaybackPlay->TabIndex = 6;
			this->buttonPlaybackPlay->Text = L"Play File";
			this->buttonPlaybackPlay->UseVisualStyleBackColor = true;
			this->buttonPlaybackPlay->Click += gcnew System::EventHandler(this, &MyForm::buttonPlaybackPlay_Click);
			// 
			// groupBoxRecording
			// 
			this->groupBoxRecording->Controls->Add(this->labelRecordDataInclude);
			this->groupBoxRecording->Controls->Add(this->checkBoxRecordDataEMG);
			this->groupBoxRecording->Controls->Add(this->checkBoxRecordDataIMU);
			this->groupBoxRecording->Controls->Add(this->numericUpDownRecordingFlag);
			this->groupBoxRecording->Controls->Add(this->labelRecordingTimestamp);
			this->groupBoxRecording->Controls->Add(this->buttonRecordingDialog);
			this->groupBoxRecording->Controls->Add(this->buttonRecordingFlag);
			this->groupBoxRecording->Controls->Add(this->labelRecordingFilename);
			this->groupBoxRecording->Controls->Add(this->textBoxRecordingFilename);
			this->groupBoxRecording->Controls->Add(this->buttonRecord);
			this->groupBoxRecording->Controls->Add(this->labelRecordingStatus);
			this->groupBoxRecording->Controls->Add(this->textBoxRecordingStatus);
			this->groupBoxRecording->Location = System::Drawing::Point(316, 10);
			this->groupBoxRecording->Margin = System::Windows::Forms::Padding(10);
			this->groupBoxRecording->Name = L"groupBoxRecording";
			this->groupBoxRecording->Size = System::Drawing::Size(359, 200);
			this->groupBoxRecording->TabIndex = 9;
			this->groupBoxRecording->TabStop = false;
			this->groupBoxRecording->Text = L"Recording";
			// 
			// labelRecordDataInclude
			// 
			this->labelRecordDataInclude->AutoSize = true;
			this->labelRecordDataInclude->Location = System::Drawing::Point(13, 64);
			this->labelRecordDataInclude->Margin = System::Windows::Forms::Padding(10, 0, 3, 0);
			this->labelRecordDataInclude->Name = L"labelRecordDataInclude";
			this->labelRecordDataInclude->Size = System::Drawing::Size(45, 13);
			this->labelRecordDataInclude->TabIndex = 17;
			this->labelRecordDataInclude->Text = L"Record:";
			// 
			// checkBoxRecordDataEMG
			// 
			this->checkBoxRecordDataEMG->AutoSize = true;
			this->checkBoxRecordDataEMG->Location = System::Drawing::Point(83, 63);
			this->checkBoxRecordDataEMG->Name = L"checkBoxRecordDataEMG";
			this->checkBoxRecordDataEMG->Size = System::Drawing::Size(50, 17);
			this->checkBoxRecordDataEMG->TabIndex = 0;
			this->checkBoxRecordDataEMG->Text = L"EMG";
			this->checkBoxRecordDataEMG->UseVisualStyleBackColor = true;
			// 
			// checkBoxRecordDataIMU
			// 
			this->checkBoxRecordDataIMU->AutoSize = true;
			this->checkBoxRecordDataIMU->Enabled = false;
			this->checkBoxRecordDataIMU->Location = System::Drawing::Point(139, 63);
			this->checkBoxRecordDataIMU->Name = L"checkBoxRecordDataIMU";
			this->checkBoxRecordDataIMU->Size = System::Drawing::Size(46, 17);
			this->checkBoxRecordDataIMU->TabIndex = 0;
			this->checkBoxRecordDataIMU->Text = L"IMU";
			this->checkBoxRecordDataIMU->UseVisualStyleBackColor = true;
			// 
			// numericUpDownRecordingFlag
			// 
			this->numericUpDownRecordingFlag->Location = System::Drawing::Point(74, 148);
			this->numericUpDownRecordingFlag->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 254, 0, 0, 0 });
			this->numericUpDownRecordingFlag->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
			this->numericUpDownRecordingFlag->Name = L"numericUpDownRecordingFlag";
			this->numericUpDownRecordingFlag->Size = System::Drawing::Size(45, 20);
			this->numericUpDownRecordingFlag->TabIndex = 16;
			this->numericUpDownRecordingFlag->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
			// 
			// labelRecordingTimestamp
			// 
			this->labelRecordingTimestamp->AutoSize = true;
			this->labelRecordingTimestamp->Location = System::Drawing::Point(199, 111);
			this->labelRecordingTimestamp->Margin = System::Windows::Forms::Padding(0, 3, 3, 3);
			this->labelRecordingTimestamp->Name = L"labelRecordingTimestamp";
			this->labelRecordingTimestamp->Size = System::Drawing::Size(112, 13);
			this->labelRecordingTimestamp->TabIndex = 15;
			this->labelRecordingTimestamp->Text = L".yy-mm-dd.hhmm.ss.txt";
			// 
			// buttonRecordingDialog
			// 
			this->buttonRecordingDialog->Location = System::Drawing::Point(316, 104);
			this->buttonRecordingDialog->Name = L"buttonRecordingDialog";
			this->buttonRecordingDialog->Size = System::Drawing::Size(30, 26);
			this->buttonRecordingDialog->TabIndex = 14;
			this->buttonRecordingDialog->Text = L"...";
			this->buttonRecordingDialog->UseVisualStyleBackColor = true;
			this->buttonRecordingDialog->Click += gcnew System::EventHandler(this, &MyForm::buttonRecordingDialog_Click);
			// 
			// buttonRecordingFlag
			// 
			this->buttonRecordingFlag->Location = System::Drawing::Point(125, 143);
			this->buttonRecordingFlag->Name = L"buttonRecordingFlag";
			this->buttonRecordingFlag->Size = System::Drawing::Size(120, 26);
			this->buttonRecordingFlag->TabIndex = 10;
			this->buttonRecordingFlag->Text = L"Set Flag";
			this->buttonRecordingFlag->UseVisualStyleBackColor = true;
			this->buttonRecordingFlag->Click += gcnew System::EventHandler(this, &MyForm::buttonRecordingFlag_Click);
			// 
			// labelRecordingFilename
			// 
			this->labelRecordingFilename->AutoSize = true;
			this->labelRecordingFilename->Location = System::Drawing::Point(13, 111);
			this->labelRecordingFilename->Margin = System::Windows::Forms::Padding(10, 0, 3, 0);
			this->labelRecordingFilename->Name = L"labelRecordingFilename";
			this->labelRecordingFilename->Size = System::Drawing::Size(57, 13);
			this->labelRecordingFilename->TabIndex = 9;
			this->labelRecordingFilename->Text = L"File Name:";
			// 
			// textBoxRecordingFilename
			// 
			this->textBoxRecordingFilename->Location = System::Drawing::Point(76, 108);
			this->textBoxRecordingFilename->Name = L"textBoxRecordingFilename";
			this->textBoxRecordingFilename->Size = System::Drawing::Size(120, 20);
			this->textBoxRecordingFilename->TabIndex = 8;
			this->textBoxRecordingFilename->Text = L"rawdata";
			// 
			// buttonRecord
			// 
			this->buttonRecord->Enabled = false;
			this->buttonRecord->Location = System::Drawing::Point(226, 26);
			this->buttonRecord->Margin = System::Windows::Forms::Padding(3, 3, 10, 3);
			this->buttonRecord->Name = L"buttonRecord";
			this->buttonRecord->Size = System::Drawing::Size(120, 26);
			this->buttonRecord->TabIndex = 5;
			this->buttonRecord->Text = L"Record";
			this->buttonRecord->UseVisualStyleBackColor = true;
			this->buttonRecord->Click += gcnew System::EventHandler(this, &MyForm::buttonRecord_Click);
			// 
			// labelRecordingStatus
			// 
			this->labelRecordingStatus->AutoSize = true;
			this->labelRecordingStatus->Location = System::Drawing::Point(13, 33);
			this->labelRecordingStatus->Margin = System::Windows::Forms::Padding(10, 0, 3, 0);
			this->labelRecordingStatus->Name = L"labelRecordingStatus";
			this->labelRecordingStatus->Size = System::Drawing::Size(40, 13);
			this->labelRecordingStatus->TabIndex = 1;
			this->labelRecordingStatus->Text = L"Status:";
			// 
			// textBoxRecordingStatus
			// 
			this->textBoxRecordingStatus->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxRecordingStatus->Cursor = System::Windows::Forms::Cursors::Arrow;
			this->textBoxRecordingStatus->Location = System::Drawing::Point(76, 30);
			this->textBoxRecordingStatus->Name = L"textBoxRecordingStatus";
			this->textBoxRecordingStatus->ReadOnly = true;
			this->textBoxRecordingStatus->Size = System::Drawing::Size(144, 20);
			this->textBoxRecordingStatus->TabIndex = 7;
			this->textBoxRecordingStatus->Text = L"Not Recording";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPageInput);
			this->tabControl1->Controls->Add(this->tabPageIMU);
			this->tabControl1->Controls->Add(this->tabPage3);
			this->tabControl1->Controls->Add(this->tabPage4);
			this->tabControl1->Controls->Add(this->tabPageClassifier);
			this->tabControl1->Controls->Add(this->tabPageOutput);
			this->tabControl1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tabControl1->HotTrack = true;
			this->tabControl1->ItemSize = System::Drawing::Size(140, 36);
			this->tabControl1->Location = System::Drawing::Point(0, 0);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->Padding = System::Drawing::Point(55, 5);
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(1264, 681);
			this->tabControl1->TabIndex = 1;
			// 
			// tabPage4
			// 
			this->tabPage4->Controls->Add(this->textBoxAli);
			this->tabPage4->Controls->Add(this->buttonAli);
			this->tabPage4->Controls->Add(this->groupBoxSSorTrans);
			this->tabPage4->Controls->Add(this->groupBox3);
			this->tabPage4->Controls->Add(this->groupBoxOrPitch);
			this->tabPage4->Controls->Add(this->groupBox2);
			this->tabPage4->Controls->Add(this->groupBoxFeatureSelect);
			this->tabPage4->Location = System::Drawing::Point(4, 40);
			this->tabPage4->Name = L"tabPage4";
			this->tabPage4->Padding = System::Windows::Forms::Padding(3);
			this->tabPage4->Size = System::Drawing::Size(1256, 637);
			this->tabPage4->TabIndex = 3;
			this->tabPage4->Text = L"Feature Selection";
			this->tabPage4->UseVisualStyleBackColor = true;
			this->tabPage4->Enter += gcnew System::EventHandler(this, &MyForm::tabPage4_Enter);
			// 
			// groupBoxSSorTrans
			// 
			this->groupBoxSSorTrans->Controls->Add(this->radioButtonTransition);
			this->groupBoxSSorTrans->Controls->Add(this->label11);
			this->groupBoxSSorTrans->Controls->Add(this->numericUpDownSSorTransWinsize);
			this->groupBoxSSorTrans->Controls->Add(this->label10);
			this->groupBoxSSorTrans->Controls->Add(this->numericUpDownSSorTransThresh);
			this->groupBoxSSorTrans->Controls->Add(this->chartSTD);
			this->groupBoxSSorTrans->Location = System::Drawing::Point(858, 260);
			this->groupBoxSSorTrans->Margin = System::Windows::Forms::Padding(2);
			this->groupBoxSSorTrans->Name = L"groupBoxSSorTrans";
			this->groupBoxSSorTrans->Padding = System::Windows::Forms::Padding(2);
			this->groupBoxSSorTrans->Size = System::Drawing::Size(299, 250);
			this->groupBoxSSorTrans->TabIndex = 144;
			this->groupBoxSSorTrans->TabStop = false;
			this->groupBoxSSorTrans->Text = L"Windowed MAV/MMAV Standard Deviation";
			// 
			// radioButtonTransition
			// 
			this->radioButtonTransition->AutoSize = true;
			this->radioButtonTransition->Location = System::Drawing::Point(7, 223);
			this->radioButtonTransition->Name = L"radioButtonTransition";
			this->radioButtonTransition->Size = System::Drawing::Size(132, 17);
			this->radioButtonTransition->TabIndex = 147;
			this->radioButtonTransition->TabStop = true;
			this->radioButtonTransition->Text = L"Steady State Entered\?";
			this->radioButtonTransition->UseVisualStyleBackColor = true;
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(251, 207);
			this->label11->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(44, 13);
			this->label11->TabIndex = 146;
			this->label11->Text = L"Winsize";
			// 
			// numericUpDownSSorTransWinsize
			// 
			this->numericUpDownSSorTransWinsize->Location = System::Drawing::Point(252, 223);
			this->numericUpDownSSorTransWinsize->Margin = System::Windows::Forms::Padding(2);
			this->numericUpDownSSorTransWinsize->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 50, 0, 0, 0 });
			this->numericUpDownSSorTransWinsize->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDownSSorTransWinsize->Name = L"numericUpDownSSorTransWinsize";
			this->numericUpDownSSorTransWinsize->Size = System::Drawing::Size(43, 20);
			this->numericUpDownSSorTransWinsize->TabIndex = 145;
			this->numericUpDownSSorTransWinsize->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->numericUpDownSSorTransWinsize->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDownSSorTransWinsize_ValueChanged);
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(193, 208);
			this->label10->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(54, 13);
			this->label10->TabIndex = 141;
			this->label10->Text = L"Threshold";
			// 
			// numericUpDownSSorTransThresh
			// 
			this->numericUpDownSSorTransThresh->Location = System::Drawing::Point(196, 223);
			this->numericUpDownSSorTransThresh->Margin = System::Windows::Forms::Padding(2);
			this->numericUpDownSSorTransThresh->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 40, 0, 0, 0 });
			this->numericUpDownSSorTransThresh->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDownSSorTransThresh->Name = L"numericUpDownSSorTransThresh";
			this->numericUpDownSSorTransThresh->ReadOnly = true;
			this->numericUpDownSSorTransThresh->Size = System::Drawing::Size(43, 20);
			this->numericUpDownSSorTransThresh->TabIndex = 138;
			this->numericUpDownSSorTransThresh->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 21, 0, 0, 0 });
			this->numericUpDownSSorTransThresh->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDownSSorTransThresh_ValueChanged);
			// 
			// chartSTD
			// 
			this->chartSTD->BackColor = System::Drawing::Color::Transparent;
			this->chartSTD->BackSecondaryColor = System::Drawing::Color::White;
			this->chartSTD->BorderlineColor = System::Drawing::Color::Black;
			this->chartSTD->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chartSTD->Location = System::Drawing::Point(4, 17);
			this->chartSTD->Margin = System::Windows::Forms::Padding(2);
			this->chartSTD->Name = L"chartSTD";
			this->chartSTD->Size = System::Drawing::Size(291, 188);
			this->chartSTD->TabIndex = 136;
			this->chartSTD->Text = L"chart2";
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->radioButtonMMAVPk);
			this->groupBox3->Controls->Add(this->label7);
			this->groupBox3->Controls->Add(this->numericUpDownMMAVThresh);
			this->groupBox3->Controls->Add(this->chartD2MMAV);
			this->groupBox3->Location = System::Drawing::Point(858, 6);
			this->groupBox3->Margin = System::Windows::Forms::Padding(2);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Padding = System::Windows::Forms::Padding(2);
			this->groupBox3->Size = System::Drawing::Size(299, 250);
			this->groupBox3->TabIndex = 143;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"MMAV";
			// 
			// radioButtonMMAVPk
			// 
			this->radioButtonMMAVPk->AutoSize = true;
			this->radioButtonMMAVPk->Location = System::Drawing::Point(4, 223);
			this->radioButtonMMAVPk->Name = L"radioButtonMMAVPk";
			this->radioButtonMMAVPk->Size = System::Drawing::Size(91, 17);
			this->radioButtonMMAVPk->TabIndex = 142;
			this->radioButtonMMAVPk->TabStop = true;
			this->radioButtonMMAVPk->Text = L"MMAV Peak\?";
			this->radioButtonMMAVPk->UseVisualStyleBackColor = true;
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(241, 208);
			this->label7->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(54, 13);
			this->label7->TabIndex = 141;
			this->label7->Text = L"Threshold";
			// 
			// numericUpDownMMAVThresh
			// 
			this->numericUpDownMMAVThresh->Location = System::Drawing::Point(242, 223);
			this->numericUpDownMMAVThresh->Margin = System::Windows::Forms::Padding(2);
			this->numericUpDownMMAVThresh->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 50, 0, 0, 0 });
			this->numericUpDownMMAVThresh->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDownMMAVThresh->Name = L"numericUpDownMMAVThresh";
			this->numericUpDownMMAVThresh->ReadOnly = true;
			this->numericUpDownMMAVThresh->Size = System::Drawing::Size(43, 20);
			this->numericUpDownMMAVThresh->TabIndex = 138;
			this->numericUpDownMMAVThresh->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 13, 0, 0, 0 });
			this->numericUpDownMMAVThresh->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDownMMAVThresh_ValueChanged);
			// 
			// chartD2MMAV
			// 
			this->chartD2MMAV->BackColor = System::Drawing::Color::Transparent;
			this->chartD2MMAV->BackSecondaryColor = System::Drawing::Color::White;
			this->chartD2MMAV->BorderlineColor = System::Drawing::Color::Black;
			this->chartD2MMAV->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chartD2MMAV->Location = System::Drawing::Point(4, 17);
			this->chartD2MMAV->Margin = System::Windows::Forms::Padding(2);
			this->chartD2MMAV->Name = L"chartD2MMAV";
			this->chartD2MMAV->Size = System::Drawing::Size(291, 188);
			this->chartD2MMAV->TabIndex = 136;
			this->chartD2MMAV->Text = L"chart2";
			// 
			// groupBoxOrPitch
			// 
			this->groupBoxOrPitch->Controls->Add(this->label5);
			this->groupBoxOrPitch->Controls->Add(this->label4);
			this->groupBoxOrPitch->Controls->Add(this->label3);
			this->groupBoxOrPitch->Controls->Add(this->numericUpDownSCValuesInAvg);
			this->groupBoxOrPitch->Controls->Add(this->numericUpDownSCThresh);
			this->groupBoxOrPitch->Controls->Add(this->textBoxSwingCount);
			this->groupBoxOrPitch->Controls->Add(this->chartOrPitch);
			this->groupBoxOrPitch->Location = System::Drawing::Point(555, 6);
			this->groupBoxOrPitch->Margin = System::Windows::Forms::Padding(2);
			this->groupBoxOrPitch->Name = L"groupBoxOrPitch";
			this->groupBoxOrPitch->Padding = System::Windows::Forms::Padding(2);
			this->groupBoxOrPitch->Size = System::Drawing::Size(299, 250);
			this->groupBoxOrPitch->TabIndex = 137;
			this->groupBoxOrPitch->TabStop = false;
			this->groupBoxOrPitch->Text = L"Orientation (Pitch)";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(257, 208);
			this->label5->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(34, 13);
			this->label5->TabIndex = 142;
			this->label5->Text = L"k-avg";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(202, 208);
			this->label4->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(54, 13);
			this->label4->TabIndex = 141;
			this->label4->Text = L"Threshold";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(4, 208);
			this->label3->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(41, 13);
			this->label3->TabIndex = 140;
			this->label3->Text = L"Swings";
			// 
			// numericUpDownSCValuesInAvg
			// 
			this->numericUpDownSCValuesInAvg->Location = System::Drawing::Point(259, 223);
			this->numericUpDownSCValuesInAvg->Margin = System::Windows::Forms::Padding(2);
			this->numericUpDownSCValuesInAvg->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDownSCValuesInAvg->Name = L"numericUpDownSCValuesInAvg";
			this->numericUpDownSCValuesInAvg->Size = System::Drawing::Size(32, 20);
			this->numericUpDownSCValuesInAvg->TabIndex = 139;
			this->numericUpDownSCValuesInAvg->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 20, 0, 0, 0 });
			this->numericUpDownSCValuesInAvg->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDownSCValuesInAvg_ValueChanged);
			// 
			// numericUpDownSCThresh
			// 
			this->numericUpDownSCThresh->Location = System::Drawing::Point(204, 223);
			this->numericUpDownSCThresh->Margin = System::Windows::Forms::Padding(2);
			this->numericUpDownSCThresh->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->numericUpDownSCThresh->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
			this->numericUpDownSCThresh->Name = L"numericUpDownSCThresh";
			this->numericUpDownSCThresh->ReadOnly = true;
			this->numericUpDownSCThresh->Size = System::Drawing::Size(43, 20);
			this->numericUpDownSCThresh->TabIndex = 138;
			this->numericUpDownSCThresh->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->numericUpDownSCThresh->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDownSCThresh_ValueChanged);
			// 
			// textBoxSwingCount
			// 
			this->textBoxSwingCount->Location = System::Drawing::Point(4, 223);
			this->textBoxSwingCount->Margin = System::Windows::Forms::Padding(2);
			this->textBoxSwingCount->Name = L"textBoxSwingCount";
			this->textBoxSwingCount->Size = System::Drawing::Size(41, 20);
			this->textBoxSwingCount->TabIndex = 137;
			// 
			// chartOrPitch
			// 
			this->chartOrPitch->BackColor = System::Drawing::Color::Transparent;
			this->chartOrPitch->BackSecondaryColor = System::Drawing::Color::White;
			this->chartOrPitch->BorderlineColor = System::Drawing::Color::Black;
			this->chartOrPitch->BorderlineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
			this->chartOrPitch->Location = System::Drawing::Point(4, 17);
			this->chartOrPitch->Margin = System::Windows::Forms::Padding(2);
			this->chartOrPitch->Name = L"chartOrPitch";
			this->chartOrPitch->Size = System::Drawing::Size(291, 188);
			this->chartOrPitch->TabIndex = 136;
			this->chartOrPitch->Text = L"chart2";
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->checkBoxAcc);
			this->groupBox2->Controls->Add(this->checkBoxGyro);
			this->groupBox2->Controls->Add(this->checkBoxOrientation);
			this->groupBox2->Location = System::Drawing::Point(377, 6);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(173, 250);
			this->groupBox2->TabIndex = 135;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Use IMU in Static Gesture Recognition\?";
			// 
			// checkBoxAcc
			// 
			this->checkBoxAcc->AutoSize = true;
			this->checkBoxAcc->Location = System::Drawing::Point(6, 89);
			this->checkBoxAcc->Name = L"checkBoxAcc";
			this->checkBoxAcc->Size = System::Drawing::Size(85, 17);
			this->checkBoxAcc->TabIndex = 2;
			this->checkBoxAcc->Text = L"Acceleration";
			this->checkBoxAcc->UseVisualStyleBackColor = true;
			this->checkBoxAcc->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxAcc_CheckedChanged);
			// 
			// checkBoxGyro
			// 
			this->checkBoxGyro->AutoSize = true;
			this->checkBoxGyro->Location = System::Drawing::Point(6, 66);
			this->checkBoxGyro->Name = L"checkBoxGyro";
			this->checkBoxGyro->Size = System::Drawing::Size(48, 17);
			this->checkBoxGyro->TabIndex = 1;
			this->checkBoxGyro->Text = L"Gyro";
			this->checkBoxGyro->UseVisualStyleBackColor = true;
			this->checkBoxGyro->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxGyro_CheckedChanged);
			// 
			// checkBoxOrientation
			// 
			this->checkBoxOrientation->AutoSize = true;
			this->checkBoxOrientation->Location = System::Drawing::Point(6, 43);
			this->checkBoxOrientation->Name = L"checkBoxOrientation";
			this->checkBoxOrientation->Size = System::Drawing::Size(77, 17);
			this->checkBoxOrientation->TabIndex = 0;
			this->checkBoxOrientation->Text = L"Orientation";
			this->checkBoxOrientation->UseVisualStyleBackColor = true;
			this->checkBoxOrientation->CheckedChanged += gcnew System::EventHandler(this, &MyForm::checkBoxOrientation_CheckedChanged);
			// 
			// groupBoxFeatureSelect
			// 
			this->groupBoxFeatureSelect->Controls->Add(this->label2);
			this->groupBoxFeatureSelect->Controls->Add(this->dataGridFeatureSelect);
			this->groupBoxFeatureSelect->Controls->Add(this->label21);
			this->groupBoxFeatureSelect->Location = System::Drawing::Point(8, 6);
			this->groupBoxFeatureSelect->Name = L"groupBoxFeatureSelect";
			this->groupBoxFeatureSelect->Size = System::Drawing::Size(363, 250);
			this->groupBoxFeatureSelect->TabIndex = 134;
			this->groupBoxFeatureSelect->TabStop = false;
			this->groupBoxFeatureSelect->Text = L"Use in Classifier\?";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(119, 27);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(51, 13);
			this->label2->TabIndex = 136;
			this->label2->Text = L"Channels";
			// 
			// dataGridFeatureSelect
			// 
			this->dataGridFeatureSelect->AllowUserToAddRows = false;
			this->dataGridFeatureSelect->AllowUserToDeleteRows = false;
			this->dataGridFeatureSelect->AllowUserToResizeColumns = false;
			this->dataGridFeatureSelect->AllowUserToResizeRows = false;
			this->dataGridFeatureSelect->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridFeatureSelect->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(9) {
				this->col0,
					this->col1, this->col2, this->col3, this->col4, this->col5, this->col6, this->col7, this->col8
			});
			this->dataGridFeatureSelect->Location = System::Drawing::Point(6, 44);
			this->dataGridFeatureSelect->Name = L"dataGridFeatureSelect";
			this->dataGridFeatureSelect->RowHeadersWidthSizeMode = System::Windows::Forms::DataGridViewRowHeadersWidthSizeMode::AutoSizeToDisplayedHeaders;
			this->dataGridFeatureSelect->Size = System::Drawing::Size(351, 200);
			this->dataGridFeatureSelect->TabIndex = 135;
			this->dataGridFeatureSelect->CellMouseUp += gcnew System::Windows::Forms::DataGridViewCellMouseEventHandler(this, &MyForm::dataGridFeatureSelect_CellMouseUp);
			this->dataGridFeatureSelect->CellValueChanged += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MyForm::dataGridFeatureSelect_CellValueChanged);
			// 
			// col0
			// 
			this->col0->HeaderText = L"All";
			this->col0->Name = L"col0";
			this->col0->Width = 25;
			// 
			// col1
			// 
			this->col1->HeaderText = L"1";
			this->col1->Name = L"col1";
			this->col1->Width = 20;
			// 
			// col2
			// 
			this->col2->HeaderText = L"2";
			this->col2->Name = L"col2";
			this->col2->Width = 20;
			// 
			// col3
			// 
			this->col3->HeaderText = L"3";
			this->col3->Name = L"col3";
			this->col3->Width = 20;
			// 
			// col4
			// 
			this->col4->HeaderText = L"4";
			this->col4->Name = L"col4";
			this->col4->Width = 20;
			// 
			// col5
			// 
			this->col5->HeaderText = L"5";
			this->col5->Name = L"col5";
			this->col5->Width = 20;
			// 
			// col6
			// 
			this->col6->HeaderText = L"6";
			this->col6->Name = L"col6";
			this->col6->Width = 20;
			// 
			// col7
			// 
			this->col7->HeaderText = L"7";
			this->col7->Name = L"col7";
			this->col7->Width = 20;
			// 
			// col8
			// 
			this->col8->HeaderText = L"8";
			this->col8->Name = L"col8";
			this->col8->Width = 20;
			// 
			// label21
			// 
			this->label21->AutoSize = true;
			this->label21->Location = System::Drawing::Point(14, 27);
			this->label21->Name = L"label21";
			this->label21->Size = System::Drawing::Size(43, 13);
			this->label21->TabIndex = 132;
			this->label21->Text = L"Feature";
			// 
			// folderBrowserDialog2
			// 
			this->folderBrowserDialog2->SelectedPath = L"C:\\Users\\admin\\Desktop";
			// 
			// openFileDialog3
			// 
			this->openFileDialog3->FileName = L"openFileDialog3";
			// 
			// saveFileDialogModel
			// 
			this->saveFileDialogModel->DefaultExt = L"model";
			this->saveFileDialogModel->Filter = L"Model Files (*.model)|*.model|Text files (*.txt)|*.txt|All files (*.*)|*.*";
			// 
			// openFileDialogModel
			// 
			this->openFileDialogModel->Filter = L"Model Files (*.model)|*.model|Text files (*.txt)|*.txt|All files (*.*)|*.*";
			// 
			// openFileDialogOutput
			// 
			this->openFileDialogOutput->Filter = L"Configuration files (*.cfg)|*.cfg|Text File (*.txt)|*.txt|All Files (*.*)|*.*";
			// 
			// saveFileDialogOutput
			// 
			this->saveFileDialogOutput->DefaultExt = L"cfg";
			this->saveFileDialogOutput->FileName = L"*.cfg";
			this->saveFileDialogOutput->Filter = L"Configuration files (*.cfg)|*.cfg|Text File (*.txt)|*.txt|All Files (*.*)|*.*";
			// 
			// saveFileDialogSaveData
			// 
			this->saveFileDialogSaveData->DefaultExt = L"data";
			this->saveFileDialogSaveData->FileName = L"*.txt";
			this->saveFileDialogSaveData->Filter = L"Text files (*.txt)|*.txt|All files (*.*)|*.*";
			// 
			// openFileDialogImportData
			// 
			this->openFileDialogImportData->Filter = L"Text files (*.txt)|*.txt|All files (*.*)|*.*";
			// 
			// statusStrip1
			// 
			this->statusStrip1->BackColor = System::Drawing::SystemColors::Window;
			this->statusStrip1->ImageScalingSize = System::Drawing::Size(24, 24);
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
				this->toolStripStatusLabelDataSource,
					this->toolStripProgressBarDataSourceStatus, this->toolStripStatusLabelDataSourceStatus, this->toolStripStatusLabelSeparator,
					this->toolStripStatusLabelStatus
			});
			this->statusStrip1->Location = System::Drawing::Point(0, 659);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(1264, 22);
			this->statusStrip1->TabIndex = 2;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// toolStripStatusLabelDataSource
			// 
			this->toolStripStatusLabelDataSource->Name = L"toolStripStatusLabelDataSource";
			this->toolStripStatusLabelDataSource->Size = System::Drawing::Size(87, 17);
			this->toolStripStatusLabelDataSource->Text = L"No data source";
			// 
			// toolStripProgressBarDataSourceStatus
			// 
			this->toolStripProgressBarDataSourceStatus->Name = L"toolStripProgressBarDataSourceStatus";
			this->toolStripProgressBarDataSourceStatus->Size = System::Drawing::Size(100, 16);
			// 
			// toolStripStatusLabelDataSourceStatus
			// 
			this->toolStripStatusLabelDataSourceStatus->Name = L"toolStripStatusLabelDataSourceStatus";
			this->toolStripStatusLabelDataSourceStatus->Size = System::Drawing::Size(12, 17);
			this->toolStripStatusLabelDataSourceStatus->Text = L"-";
			// 
			// toolStripStatusLabelSeparator
			// 
			this->toolStripStatusLabelSeparator->Name = L"toolStripStatusLabelSeparator";
			this->toolStripStatusLabelSeparator->Size = System::Drawing::Size(1048, 17);
			this->toolStripStatusLabelSeparator->Spring = true;
			// 
			// toolStripStatusLabelStatus
			// 
			this->toolStripStatusLabelStatus->Name = L"toolStripStatusLabelStatus";
			this->toolStripStatusLabelStatus->Size = System::Drawing::Size(0, 17);
			// 
			// buttonAli
			// 
			this->buttonAli->Location = System::Drawing::Point(463, 326);
			this->buttonAli->Name = L"buttonAli";
			this->buttonAli->Size = System::Drawing::Size(105, 26);
			this->buttonAli->TabIndex = 145;
			this->buttonAli->Text = L"Add";
			this->buttonAli->UseVisualStyleBackColor = true;

			this->buttonAli->Click += gcnew System::EventHandler(this, &MyForm::buttonAli_Click);

			


			// 
			// textBoxAli
			// 
			this->textBoxAli->BackColor = System::Drawing::SystemColors::Window;
			this->textBoxAli->Location = System::Drawing::Point(594, 330);
			this->textBoxAli->Name = L"textBoxAli";
			this->textBoxAli->ReadOnly = true;
			this->textBoxAli->Size = System::Drawing::Size(100, 20);
			this->textBoxAli->TabIndex = 146;
			//this->textBoxAli->Text = L"none" 
			//this->textBoxAli->ValueChanged += gcnew System::EventHandler(this, &MyForm::buttonAli_Click);
			//this->textBoxAli->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MyForm::buttonAli_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1264, 681);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->tabControl1);
			this->Name = L"MyForm";
			this->Text = L"Myo Gesture Classification Sheet Alpha";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->tabPageOutput->ResumeLayout(false);
			this->tableLayoutPanelOutput->ResumeLayout(false);
			this->flowLayoutPanelOutput->ResumeLayout(false);
			this->groupBoxButtonsKeys->ResumeLayout(false);
			this->groupBoxButtonsKeys->PerformLayout();
			this->groupBoxPipelineControlOptions->ResumeLayout(false);
			this->groupBoxPipelineControlOptions->PerformLayout();
			this->groupBoxAxes->ResumeLayout(false);
			this->groupBoxAxes->PerformLayout();
			this->panelSettings->ResumeLayout(false);
			this->groupBoxOutput->ResumeLayout(false);
			this->tabPageClassifier->ResumeLayout(false);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxGesture))->EndInit();
			this->groupBoxClassify->ResumeLayout(false);
			this->groupBoxClassify->PerformLayout();
			this->groupBoxClassifyLive->ResumeLayout(false);
			this->groupBoxClassifyLive->PerformLayout();
			this->groupBoxClassifyFile->ResumeLayout(false);
			this->panelClassifyFile->ResumeLayout(false);
			this->panelClassifyFile->PerformLayout();
			this->flowLayoutPanelClassifySource->ResumeLayout(false);
			this->flowLayoutPanelClassifySource->PerformLayout();
			this->groupBoxTrain->ResumeLayout(false);
			this->groupBoxTrain->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->confusionMatrix))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamCoef0))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamGamma))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSVMParamDegree))->EndInit();
			this->groupBoxRealtimeTesting->ResumeLayout(false);
			this->groupBoxRealtimeTesting->PerformLayout();
			this->groupBoxGesture->ResumeLayout(false);
			this->groupBoxGesture->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->gestureList))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown4))->EndInit();
			this->groupBox17->ResumeLayout(false);
			this->groupBox17->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart4))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown1))->EndInit();
			this->tabPage3->ResumeLayout(false);
			this->tabPage3->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown5))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart3))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown3))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chart1))->EndInit();
			this->groupBox11->ResumeLayout(false);
			this->groupBox11->PerformLayout();
			this->tabPageIMU->ResumeLayout(false);
			this->tableLayoutPanelIMU->ResumeLayout(false);
			this->tableLayoutPanelIMUCenter->ResumeLayout(false);
			this->tableLayoutPanelIMUText->ResumeLayout(false);
			this->groupBoxIMUGyroscope->ResumeLayout(false);
			this->groupBoxIMUGyroscope->PerformLayout();
			this->groupBoxIMUAccelerometer->ResumeLayout(false);
			this->groupBoxIMUAccelerometer->PerformLayout();
			this->groupBoxIMUOrientation->ResumeLayout(false);
			this->groupBoxIMUOrientation->PerformLayout();
			this->panelAccelerationVector->ResumeLayout(false);
			this->panelAccelerationVector->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAccelXY))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAccelZ))->EndInit();
			this->panelAttitudeHeading->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxHeading))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxAttitude))->EndInit();
			this->tabPageInput->ResumeLayout(false);
			this->tableLayoutPanelInput->ResumeLayout(false);
			this->flowLayoutPanelInputCenter->ResumeLayout(false);
			this->groupBoxInputMyo->ResumeLayout(false);
			this->tableLayoutPanelInputMyo->ResumeLayout(false);
			this->groupBoxMyo2->ResumeLayout(false);
			this->flowLayoutPanelMyo2->ResumeLayout(false);
			this->tableLayoutPanelMyo2Controls->ResumeLayout(false);
			this->panelMyo2Battery->ResumeLayout(false);
			this->panelMyo2Battery->PerformLayout();
			this->groupBoxMyo1->ResumeLayout(false);
			this->flowLayoutPanelMyo1->ResumeLayout(false);
			this->tableLayoutPanelMyo1Controls->ResumeLayout(false);
			this->panelMyo1Battery->ResumeLayout(false);
			this->panelMyo1Battery->PerformLayout();
			this->flowLayoutPanelInputData->ResumeLayout(false);
			this->groupBoxPlayback->ResumeLayout(false);
			this->groupBoxPlayback->PerformLayout();
			this->groupBoxRecording->ResumeLayout(false);
			this->groupBoxRecording->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownRecordingFlag))->EndInit();
			this->tabControl1->ResumeLayout(false);
			this->tabPage4->ResumeLayout(false);
			this->tabPage4->PerformLayout();
			this->groupBoxSSorTrans->ResumeLayout(false);
			this->groupBoxSSorTrans->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSSorTransWinsize))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSSorTransThresh))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartSTD))->EndInit();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownMMAVThresh))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartD2MMAV))->EndInit();
			this->groupBoxOrPitch->ResumeLayout(false);
			this->groupBoxOrPitch->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSCValuesInAvg))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDownSCThresh))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->chartOrPitch))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->groupBoxFeatureSelect->ResumeLayout(false);
			this->groupBoxFeatureSelect->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridFeatureSelect))->EndInit();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	private: System::Void buttonMyo1Connect_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!md.myo) {
			if (!hub)
			{
				//construct hub
				try
				{
					hub = new myo::Hub();
				}
				catch (const std::runtime_error&)
				{
					MessageBox::Show("Myo could not start up. Is Myo Connect running?",
						"Myo Error", MessageBoxButtons::OK,
						MessageBoxIcon::Error);
					toolStripStatusLabelStatus->Text = L"Connection failed.";
					return;
				}
			}

			md.myo = hub->waitForMyo(200);

			if (md.myo) {
				//disconnect reader
				if (currentSource == DataSource::file)
				{
					stopFilePlayback();
				}

				hub->addListener(&md);
				startStreamingMyo();

				md.myo->requestBatteryLevel();

				myoThread = gcnew Thread(gcnew ThreadStart(this, &MyForm::runMyo));
				myoThread->Start();
				System::Diagnostics::Debug::Write("started myo\n");

				labelMyo1ConnectionStatus->Text = L"Connected";
				buttonMyo1Connect->Text = L"Disconnect";
				buttonMyo1Stream->Enabled = true;
				checkBoxRecordDataEMG->Checked = true;
				toolStripStatusLabelStatus->Text = L"Connection successful.";

				centerMyo(md);
			}
			else
			{
				//connection failed
				toolStripStatusLabelStatus->Text = L"Connection failed.";
				//destroy hub
				delete hub;
				hub = nullptr;
			}
		}
		else
		{
			//disconnect myo
			if (currentSource == DataSource::myo)
			{
				stopStreamingMyo();
			}
			labelMyo1ConnectionStatus->Text = L"Disconnected";
			buttonMyo1Connect->Text = L"Connect";
			buttonMyo1Stream->Enabled = false;

			md.myo = nullptr;	//signal myoThread to close
			myoThread->Join();
			hub->removeListener(&md);
			Thread::Sleep(100);
			//destroy hub
			delete hub;
			hub = nullptr;
		}
	}
	private: System::Void buttonMyo1Stream_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!md.myo)
		{
			return;
		}

		if (!myo1Streaming)
		{
			//disconnect reader
			if (currentSource == DataSource::file)
			{
				stopFilePlayback();
			}

			startStreamingMyo();
		}
		else
		{
			stopStreamingMyo();
		}
	}
	private: System::Void buttonMyo1Center_Click(System::Object^  sender, System::EventArgs^  e) {
		if (md.myo)
		{
			centerMyo(md);
			toolStripStatusLabelStatus->Text = L"Primary Myo centered.";
		}
	}
	private: System::Void buttonRecord_Click(System::Object^  sender, System::EventArgs^  e) {
		if (!recordingRawData)
		{
			md.addConsumer(dr);
			dr.openFile(
				msclr::interop::marshal_as<std::string>(
					folderBrowserDialogRecording->SelectedPath) +
				"\\" +
				msclr::interop::marshal_as<std::string>(
					textBoxRecordingFilename->Text) +
				timestamp() +
				".txt");

			System::Diagnostics::Debug::Write("writing data\n");

			checkBoxRecordDataEMG->Enabled = false;
			//checkBoxRecordDataIMU->Enabled = false;

			buttonRecord->Text = L"Stop Recording";
			toolStripStatusLabelStatus->Text = L"Recording started.";

			recordingRawData = true;

			timeRecordStart = DateTime::Now;
			textBoxRecordingStatus->Text = String::Format("{0,2:00}:{1,2:00}", 0, 0);
			RecordTimer->Start();
		}
		else
		{
			md.removeConsumer(dr);
			dr.stopWrite();
			System::Diagnostics::Debug::Write("stopped recording\n");

			checkBoxRecordDataEMG->Enabled = true;
			//checkBoxRecordDataIMU->Enabled = true;

			recordingRawData = false;

			buttonRecord->Text = L"Record";
			toolStripStatusLabelStatus->Text = L"Recording stopped.";

			textBoxRecordingStatus->Text = "Stopped Recording!";
			RecordTimer->Stop();
		}
	}

	//main loop
	//GUI update/refresh loop
	private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
		//timer loop

		//timer1->Stop();

		//int ex(0);

		//try {

		//	/*
		//	6/24/15 12:58
		//	Check if the hub.run delay on top of the timer delay is slowing down the data stream more than intended (count the data points in a second)
		//	*/

		//	/// NEED A FEATURE FUNCTION
		//	
		//	// enter if playing back from FILE
		//	if (myoStreaming == false && fileOpened == true) {
		//		
		//		if (playbackLoop){
		//			playbackEMG(openEMGFile);

		//			playbackIMU(openIMUFile);

		//		}
		//	
		//		if ((!(checkBox6->Checked) && currentNumberLinesEmgPlayback >= maxNumberLinesEmgPlayback)) // || (!(checkBox6->Checked) && currentNumberLinesImuPlayback >= maxNumberLinesImuPlayback))
		//		{
		//			playbackLoop = false;
		//		}
		//		else {
		//			playbackLoop = true;
		//		}
		//		if (playbackLoop){
		//			if (currentNumberLinesEmgPlayback >= maxNumberLinesEmgPlayback) {
		//				currentNumberLinesEmgPlayback = 0;
		//			}
		//			if (currentNumberLinesImuPlayback >= maxNumberLinesImuPlayback) {
		//				currentNumberLinesImuPlayback = 0;
		//			}

		//		}

		//		progressBar1->Maximum = maxNumberLinesEmgPlayback;

		//		if (currentNumberLinesEmgPlayback <= maxNumberLinesEmgPlayback){
		//			progressBar1->Value = currentNumberLinesEmgPlayback;
		//			label39->Text = "" + (currentNumberLinesEmgPlayback)* 100 / (maxNumberLinesEmgPlayback)+"%";
		//		}

		//	}
		//	else {
		//		deviceMyo.hub.run(1000 / rrate);
		//	}

		//	// we pulled this stuff out of the conditional statments, because it wasnt working as intended
		//	int p = deviceMyo.collector.iteration_emg - deviceMyo.collector.lastCall;
		//	int o = p & 127;
		//	int q = deviceMyo.collector.sampling;

		//	while (o >= q)
		//	{
		//		
		//		//calculate features after minimum samples taken
		//		deviceMyo.collector.featCalc();
		//		if (deviceMyo.collector.armed) {
		//			classifier_training();
		//		}

		//		p = deviceMyo.collector.iteration_emg - deviceMyo.collector.lastCall;
		//		o = p & 127;
		//	}
		//	// IMU outside because iteration_emg is incremented inside of onEMG event, but IMU is not
		//	deviceMyo.collector.iteration_imu++;
		//	deviceMyo.collector.iteration_imu &= 127;

		//
		//	// displays graph unviersally
		//	
		//	int it_emg_view = (deviceMyo.collector.iteration_emg - 1) & 127;
		//	int it_imu_view = (deviceMyo.collector.iteration_imu - 1) & 127;

		//	// should enter whenever data is being processed
		//	if (myoStreaming == true || playbackLoop == true){

				switch (tabSelected)
				{
				case input:
					updateStatus();
					break;

				case imu:
					printIMUText();
					drawIMUDisplays();
					break;

				case emg:
					Display_EMG();

					printEMG();

					break;

		//		case feature:
		//			break;

				case classifier:
		//			//updates the # of gestures and # of sampmles textboxes
		//			textBox30->Text = "" + deviceMyo.collector.window_index;

		//			//updates "armed" status on classifier page
		//			if (deviceMyo.collector.armed == true) {
		//				textBox35->Text = "armed";
		//			}
		//			else { 
		//				textBox35->Text = "not armed"; 
		//			}


		//			if (accuracy_file_created == 5) {
		//				get_accuracy();
		//				accuracy_file_created = 0;
		//			}
		//			if (accuracy_file_created > 0) {
		//				accuracy_file_created++;
		//			}
		//			
		//			
		//			if (textBox36->Text != "" && comboBox10->Items->Count != 0) {
		//				button12->Enabled = true;
		//			}
		//			else {
		//				button12->Enabled = false;
		//			}

		//			if (!classify) { textBox36->Enabled = true; }
		//			else{ textBox36->Enabled = false;
		//			}

					//plots MMAV chart
					if (checkBox5->Checked == true) {
						mmav_display_line();
						label76->Text = "" + fc.getFeatureAverage(0);
					}
		//			
		//			deviceMyo.collector.iteration_imu = 0; //test later
		//							
					//check on ClassifyTrainer collection status
					if (training == true && readyToTrainGesture == true)
					{
						textBox30->Text = "" + ct.getSampleCount();
						gestureList->CurrentRow->Cells[2]->Value = ct.getClassSampleCount(md.getFlag());

						int cRow = (int)gestureList->CurrentRow->Cells[0]->Value;
						int nRow = (int)gestureList->Rows->Count;

						buttonTrainGesture->Focus();

						if (!ct.isActive())
						{
							md.setFlag(-1);
							md.removeConsumer(dbuf);
							fc.removeConsumer(ct);
							ct.stopCollect();
							buttonTrainGesture->Text = L"Train";
							training = false;
							textBoxTrainNextGestureWarning->Visible = false;

							//int cRow = (int)gestureList->CurrentRow->Cells[0]->Value;
							//int nRow = (int)gestureList->Rows->Count;
							if (cRow < nRow - 1) {
								//gestureList->Rows(cRow)->Se
								gestureList->CurrentCell = gestureList->Rows[cRow + 1]->Cells[0];
							}
							else {
								gestureList->CurrentCell = gestureList->Rows[0]->Cells[0];
							}
							//////////////////

							gestureList->Enabled = true;
							gestureList->Focus();
						}

						if (checkBoxAutoRun->Checked == false && training == false) {
							readyToTrainGesture = true; 
							md.setFlag(-1);
						}
						else if ((cRow < nRow - 1) && training == false)
						{
							buttonTrainGesture_Click_1(sender, e);
						}
					}

					//update classification
					if (classify)
					{
						int indexOfCurrentClass = cl->getLatestPrediction();
						// JTY textBoxClassifierDecision->Text and pictureBoxGesture->Image should change only when a new prediction is made
						// not for every frame classifier is running
						if (indexOfPreviousClass != indexOfCurrentClass) {
							
							// JTY Update pictureBoxGesture image to picture associated with current classification
							// or make invisible if no picture
							textBoxClassifierDecision->Text = gcnew String(ct.getClassName(indexOfCurrentClass).c_str());
							updatePicture(indexOfCurrentClass);
							indexOfPreviousClass = indexOfCurrentClass;
						}
						textBoxClassifierDecision->Text = gcnew String(ct.getClassName(cl->getLatestPrediction()).c_str());
						////ICE update picture?
					}

					if (md.getFlag() >= 0)
					{
						textBoxCurrentFlag->Text = gcnew String(ct.getClassName(md.getFlag()).c_str());
					}
					else
					{
						textBoxCurrentFlag->Text = L"null";
					}

					break;

		//		case output:
		//			break;

				}

				if (classify) {
					int prediction = cl->getLatestPrediction();
					if (checkBoxMyoKeyboard->Checked)
					{
						go.setInput(prediction);
					}
					//nps->sendValue(cl->getLatestPrediction(), fc.getFeatureAverage(0));
					updatePipe();
				}
				if (checkBoxMyoJoystick->Checked)
				{
					myo::Quaternion<float> orientation = md.orientation();
					//multiply, noncommutative
					orientation = (myo::Quaternion<float>(myoInitOrientation[0], myoInitOrientation[1], myoInitOrientation[2], myoInitOrientation[3])) * orientation;
					myo::Vector3<float> acceleration = md.acceleration();
					go.setInputAxis(inputAxes->at("Roll"), MyoData::roll(orientation) * 180 / Math::PI);
					go.setInputAxis(inputAxes->at("Pitch"), MyoData::pitch(orientation) * 180 / Math::PI);
					go.setInputAxis(inputAxes->at("Yaw"), MyoData::yaw(orientation) * 180 / Math::PI);

					myo::Vector3<float> accel = md.acceleration();
					go.setInputAxis(inputAxes->at("AccelX"), accel.x());
					go.setInputAxis(inputAxes->at("AccelY"), accel.y());
					go.setInputAxis(inputAxes->at("AccelZ"), accel.z());

					go.setInputAxis(inputAxes->at("Mean MAV"), fc.getFeatureAverage(0));
				}

				switch (currentSource)
				{
				case DataSource::none:
					buttonPlaybackLoad->Enabled = true;
					buttonPlaybackPlay->Text = L"Play File";
					toolStripStatusLabelDataSource->Text = L"No data source";
					toolStripProgressBarDataSourceStatus->Value = 0;
					toolStripStatusLabelDataSourceStatus->Text = L"-";
					break;
				case DataSource::myo:
					orPitch_display_line();
					d2mmav_display_line();
					std_display_line();
					break;
				case DataSource::file:
					progressBarPlayback->Value = reader.getCurrentIndex();
					{
						int sec = reader.getCurrentIndex() / 200;
						labelPlaybackTime->Text = String::Format("{0,1:0}:{1,2:00}", sec / 60, sec % 60);
						toolStripStatusLabelDataSourceStatus->Text = labelPlaybackTime->Text;
					}
					toolStripProgressBarDataSourceStatus->Value = reader.getCurrentIndex();
					break;
				default:
					break;
				}
		//	
		//	}


		//	if (record == true){
		//		recordData(it_emg_view, it_imu_view);
		//	}

		//	Application::DoEvents();

		//}
		//
		//catch (int)
		//{
		//	textBox1->Text = "Error!";
		//	return;
		//}
		//
		//
		//if (endtimer1 == false){

		//	timer1->Start();

		//}
				if (sf.hasSwOccured()) {
					swingcount++;
				}
				textBoxSwingCount->Text = "" + swingcount;

				if (mpf.hasPkOccured()) {
					radioButtonMMAVPk->Checked = true;
				}
				else radioButtonMMAVPk->Checked = false;

				if (sst.hasEnteredStedayState()) {
					radioButtonTransition->Checked = true;
				}
				else radioButtonTransition->Checked = false;
	}

	private: System::Void updatePicture(int index) {
		if (ct.getClassPicturePath(index) != "")
		{
			pictureBoxGesture->Visible = true;
			pictureBoxGesture->Image = Image::FromFile(ct.getClassPicturePath(index));
		}
		else pictureBoxGesture->Visible = false;
	}
	private: System::Void addGesture(System::String^ name)
	{
		//if (!String::IsNullOrWhiteSpace(textBox14->Text))
		//{
	    std::string name_std = context.marshal_as<std::string>(name);
		

			if (ct.getClassIndex(name_std) >= 0)
			{
				MessageBox::Show("Warning: Gesture name '" + name + "' already exists!",
					"Myo Gesture Classification Sheet", MessageBoxButtons::OK,
					MessageBoxIcon::Asterisk);
			}
			else
			{
				int flag = ct.addClassName(name_std);
				//textBox14->Text = name;
				gestureList->Rows->Add(flag, name , 0); //textBox14->Text
				//confusionMatrix->Rows->Add(flag, textBox14->Text, 0);
				//confusionMatrix->ColumnAdded(flag, textBox14->Text, 0);
				gestureList->CurrentCell = gestureList->Rows[gestureList->Rows->Count - 1]->Cells[1];

				buttonTrainGesture->Enabled = true;
				buttonDeleteAll->Enabled = true;
				buttonClearSamples->Enabled = true;
				updateComboBoxNextGesture();
				int nGest = ct.getNumberOfClasses();
				textBox29->Text = "" + nGest;
				confusionMatrix->ColumnCount = 1 + nGest;
				confusionMatrix->Columns[nGest]->Name = name;
				confusionMatrix->Columns[nGest]->Width = 40;
				confusionMatrix->Rows->Add(name);
			}
		//}
	}
	private: System::Void updateGestureName(int index)
	{
		System::String^ name = gcnew System::String(ct.getClassName(index).c_str());
		gestureList->Rows[index]->Cells[1]->Value = name;
		confusionMatrix->Rows[index]->Cells[0]->Value = name;
		confusionMatrix->Columns[index+1]->Name = name;
	}

	private: System::Void buttonAddGesture_Click(System::Object^  sender, System::EventArgs^  e) {
		//"Add" in classifier
		if (!String::IsNullOrWhiteSpace(textBox14->Text)) {
			addGesture(textBox14->Text);
		}
	}
			//////////////////////////////////////////////////////////////////////
			 int i01 = 0;
				private: System::Void buttonAli_Click(System::Object^  sender, System::EventArgs^  e) {
					//"Add" in classifier
					
					textBoxAli->Text = "" + ct.getSampleCount();

					if (textBoxAli->Text != "")
						i01++;
					textBoxAli->Text = "" + i01;

				}
				


				//////////////////////////////////////////////////////////////////////

	private: System::Void gesture_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
				 //carriage return keypress
				 if ((e->KeyChar == (char)' ') && !classify)//predict off (!String::IsNullOrWhiteSpace(textBox14->Text)))
				 {
					 buttonTrainGesture_Click_1(sender, e);
					
				 }
    }


	private: System::Void textBox14_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
		//carriage return keypress
		if ((e->KeyChar == (char)'\r')&& (!String::IsNullOrWhiteSpace(textBox14->Text)))
		{
			addGesture(textBox14->Text);
			textBox14->SelectAll();
			e->Handled = true;
		}
	}
	private: System::Void buttonRecordingFlag_Click(System::Object^  sender, System::EventArgs^  e) {
		Myo_app::md.setFlag((int)numericUpDownRecordingFlag->Value);
		toolStripStatusLabelStatus->Text = L"Flag " + md.getFlag() + L" set.";
	}
		 
	private: System::Void buttonPlaybackLoad_Click(System::Object^  sender, System::EventArgs^  e) {
		if (openFileDialogPlayback->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			textBoxPlaybackFilename->Text = openFileDialogPlayback->FileName;
			buttonPlaybackPlay->Enabled = true;
			reader.load(msclr::interop::marshal_as<std::string>(openFileDialogPlayback->FileName), NUM_EMG_SENSORS);
			toolStripStatusLabelStatus->Text = L"EMG file ready for playback.";
		}
	}
	private: System::Void buttonPlaybackPlay_Click(System::Object^  sender, System::EventArgs^  e) {
		if (currentSource == DataSource::file)
		{
			stopFilePlayback();
		}
		else
		{
			//disconnect MyoData
			if (currentSource == DataSource::myo)
			{
				stopStreamingMyo();
			}
			startFilePlayback();
		}
	}
	private: System::Void buttonRecordingDialog_Click(System::Object^  sender, System::EventArgs^  e) {
		//folder browser for choosing where to save recordings
		folderBrowserDialogRecording->ShowDialog();
	}
	private: System::Void RecordTimer_Tick(System::Object^  sender, System::EventArgs^  e) {
		TimeSpan diff = DateTime::Now - timeRecordStart;
		int seconds = diff.Seconds + ((diff.Milliseconds + 500) / 1000);
		textBoxRecordingStatus->Text = String::Format("{0,2:00}:{1,2:00}",
			diff.Minutes,
			seconds);
	}

	// myo vibration
	private: System::Void buttonMyo1Vibrate_Click(System::Object^  sender, System::EventArgs^  e) {
		if (md.myo)
		{
			md.myo->vibrate(myo::Myo::vibrationShort);
		}
	}
	// next 4 button events for Play/Pause on IMU, EMG Tab
	private: System::Void button28_Click(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void button29_Click(System::Object^  sender, System::EventArgs^  e) {
	}
	private: System::Void comboBox3_SelectedIndexChanged_1(System::Object^  sender, System::EventArgs^  e) {
		//if (comboBox3->SelectedIndex == 1){
		//	groupBox10->Visible = false;	//Hide myo 1
		//	groupBox12->Visible = true;		//Show myo 2
		//}
		//else{
		//	groupBox10->Visible = true;		//Hide myo 1
		//	groupBox12->Visible = false;	//Show myo 2

		//}
	
	}
	private: System::Void tabPageIMU_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = imu;
	}

	private: System::Void comboBox4_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		//selectedChart_Cbox4 = comboBox4->SelectedIndex;
		//Relabel_IMU_chart_area(comboBox4,chart6);
	}
	private: System::Void comboBox5_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		//selectedChart_Cbox5 = comboBox5->SelectedIndex;
		//Relabel_IMU_chart_area(comboBox5, chart5);
	}
	private: System::Void comboBox11_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		//selectedChart_Cbox11 = comboBox11->SelectedIndex;
		//Relabel_IMU_chart_area(comboBox11, chart7);
	} 

			 // classifier enter
	private: System::Void tabPageClassifier_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = classifier;
		/*comboBox11->SelectedIndex = 0;
		ite = 100;
		xmin = 0;
		xmax = 100;
		deviceMyo.collector.iteration_emg = 0;*/
	}
	private: System::Void tabPage3_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = emg;
		comboBox6->SelectedIndex = 0;
		comboBox8->SelectedIndex = 1;
		comboBox7->SelectedIndex = 2;
		comboBox9->SelectedIndex = 3;

	}
	private: System::Void comboBox6_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		Relabel_EMG_chart_area(chart1);
	}
	private: System::Void comboBox7_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		//Relabel_EMG_chart_area(chart2);
	}
	private: System::Void comboBox8_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		Relabel_EMG_chart_area(chart1);
	}
	private: System::Void comboBox9_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
		//Relabel_EMG_chart_area(chart2);
	}
	/*		 
	private: System::Void checkBox7_CheckStateChanged(System::Object^  sender, System::EventArgs^  e) {
		Relabel_EMG_chart_area(chart1);
		//Relabel_EMG_chart_area(chart2);

	}*/
	private: System::Void numericUpDown5_ValueChanged(System::Object^  sender, System::EventArgs^  e) {

		//deviceMyo.collector.threshold = (int)numericUpDown5->Value;

	}
	private: System::Void tabPageClassifier_Leave(System::Object^  sender, System::EventArgs^  e) {
		chart4->Series["mmav"]->Points->Clear();
		chart4->Series["threshold"]->Points->Clear();
	}
	private: System::Void tabPageOutput_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = output;
		//fill combobox
		updateComboBoxGestureOutput();
	}

	private: System::Void tabPage4_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = feature;
	}


	private: System::Void tabPageInput_Enter(System::Object^  sender, System::EventArgs^  e) {
		tabSelected = input;
	}

	private: System::Void buttonTrainGesture_Click_1(System::Object^  sender, System::EventArgs^  e) {
		//train/stop button
		if (ct.isActive())
		{
			md.mute();

			md.setFlag(-1);
			md.removeConsumer(dbuf);
			fc.removeConsumer(ct);
			ct.stopCollect();
			buttonTrainGesture->Text = L"Train";
			training = false;
			readyToTrainGesture = true;
			gestureList->Enabled = true;
			textBoxTrainNextGestureWarning->Visible = false;

			md.unmute();
		}
		else if (!checkBoxAutoRun->Checked && !ct.isActive())
		{
			md.mute();

			//prep signal path for gesture training
			md.setFlag((int)gestureList->CurrentRow->Cells[0]->Value);
			md.addConsumer(dbuf);
			fc.reset();	//reset buffer
			fc.addConsumer(ct);
			ct.startCollect((int)numericUpDown4->Value);

			buttonTrainGesture->Text = L"Stop";
			training = true;
			readyToTrainGesture = true;

			gestureList->Enabled = false;

			md.unmute();
		}
		else if(checkBoxAutoRun->Checked && !ct.isActive() && readyToTrainGesture)// auto run training
		{
			// start timer
			// when timer is finished, begin taking samples
			buttonTrainGesture->Text = L"Stop";
			readyToTrainGesture = false;
			gestureList->Enabled = false;

			nextTrainGestureDelay->Interval = 10;
			nextTrainGestureDelay->Start();
			progressBarTrainNextGesture->Maximum = Int32::Parse(textBoxAutoRunSeconds->Text)*100;
			progressBarTrainNextGesture->Value = progressBarTrainNextGesture->Maximum;
			textBoxTrainNextGestureWarning->Visible = true;
			progressBarTrainNextGesture->Visible = true;
			buttonTrainGesture->Focus();
		} 
		else if(checkBoxAutoRun->Checked && !readyToTrainGesture) // cancel auto run training
		{
			buttonTrainGesture->Text = L"Train";
			nextGestureDelay->Stop();
			textBoxTrainNextGestureWarning->Visible = false;
			progressBarTrainNextGesture->Visible = false;
			buttonTrainGesture->Focus();

			readyToTrainGesture = true;
			training = false;
			cancelAutoTrain = true;
			gestureList->Enabled = true;
		}

	}

	private: System::Void buttonDeleteAll_Click(System::Object^  sender, System::EventArgs^  e) {
		//"remove / delete all " button in classifier
		this->gestureList->CurrentCellChanged -= gcnew System::EventHandler(this, &MyForm::gestureList_CurrentCellChanged);
		gestureList->Rows->Clear();
		confusionMatrix->Rows->Clear();
		confusionMatrix->ColumnCount = 1;
		this->gestureList->CurrentCellChanged += gcnew System::EventHandler(this, &MyForm::gestureList_CurrentCellChanged);
		textBox14->Text = "";
		ct.clearAll();
		dbuf.clear();
		textBox29->Text = "" + ct.getNumberOfClasses();
		buttonTrainGesture->Enabled = false;
		buttonDeleteAll->Enabled = false;
		buttonClearSamples->Enabled = false;
	}

	private: System::Void buttonCreateModel_Click(System::Object^  sender, System::EventArgs^  e) {
		// create model button
		if (ct.data().size() == 0)
		{
			MessageBox::Show("There is no gesture data.", "No data",
				MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}
		switch (comboBoxClassifierType->SelectedIndex)
		{
		case 0:
		case 1:
			svm.setType(comboBoxClassifierType->SelectedIndex);
			switch (comboBoxSVMKernel->SelectedIndex)
			{
			case 0:
				svm.setKernelLinear();
				break;
			case 1:
				svm.setKernelPoly((int)numericUpDownSVMParamDegree->Value, (double)numericUpDownSVMParamGamma->Value, (double)numericUpDownSVMParamCoef0->Value);
				break;
			case 2:
				svm.setKernelRBF((double)numericUpDownSVMParamGamma->Value);
				break;
			case 3:
				svm.setKernelSigmoid((double)numericUpDownSVMParamGamma->Value, (double)numericUpDownSVMParamCoef0->Value);
			default:
				break;
			}
			cl = &svm;
			break;
		case 2:
			cl = &lda;
			//warn if data not rectangular
			{
				std::vector<std::pair<int, std::string>> classesVector = ct.getAllClasses();
				int length = ct.getClassSampleCount(classesVector[0].first);
				for (size_t i = 1; i < classesVector.size(); i++)
				{
					if (length != ct.getClassSampleCount(classesVector[i].first))
					{
						MessageBox::Show("Training data will be trimmed to the shortest class set.", "LDA - Warning",
							MessageBoxButtons::OK, MessageBoxIcon::Warning);
						break;
					}
				}
			}
			break;
		default:
			break;
		}

		float *confM;
		int nC = ct.getNumberOfClasses();
		confM = cl->crossAccuracy(ct.data(), nC, 5);// 5 to replaced by number box
		if (confM != nullptr)
		{
			textBoxTrainingAccuracy->Text = "" + ((float)confM[0] * 100) + "%"; // test.first;// *100 / test.second + "%";
			for (int i = 0; i < nC; i++) {
				for (int j = 0; j < nC; j++) {
					confusionMatrix->Rows[i]->Cells[j + 1]->Value = confM[1 + j + i*nC] + "%";
				}
			}
			delete confM;
			confM = nullptr;
		}
		
		cl->train(ct.data());
		//std::pair<int, int> test = cl->testAccuracy(ct.data());

		if (cl->modelReady())
		{
			buttonStartClassifier->Enabled = true;
			buttonSaveModel->Enabled = true;
			if (classifyFileData != nullptr && classifyFileData->size() > 0)
			{
				buttonClassifyTestFile->Enabled = true;
			}
			toolStripStatusLabelStatus->Text = L"Model ready.";
		}
	}

	private: System::Void buttonStartClassifier_Click(System::Object^  sender, System::EventArgs^  e) {
		//start/stop classifier button
		if (classify) {
			md.setFlag(-1);
			fc.removeConsumer(*cl);
			fc.removeConsumer(acr);
			cl->removeConsumer(acr);
			acr.stopWrite();
			nps->stop();
			classify = false;
			buttonCreateModel->Enabled = true;
			checkBoxRecordDecisionData->Enabled = true;
			flowLayoutPanelClassifySource->Enabled = true;
			buttonStartClassifier->Text = "Start Classifier";
			if (checkBoxRecordDecisionData->Checked)
			{
				toolStripStatusLabelStatus->Text = L"Decision data saved.";
			}
			else
			{
				toolStripStatusLabelStatus->Text = L"Classifier stopped.";
			}
			textBoxClassifierDecision->Text = "";
			textBoxNextGestureWarning->Visible = false;

			updatePicture(gestureList->CurrentRow->Index);
			
		}
		else
		{
			classify = true;
			buttonCreateModel->Enabled = false;
			checkBoxRecordDecisionData->Enabled = false;
			flowLayoutPanelClassifySource->Enabled = false;
			buttonStartClassifier->Text = "Stop Classifier";
			toolStripStatusLabelStatus->Text = L"Classifier started.";
			fc.addConsumer(*cl);
			nps->start();
			if (checkBoxRecordDecisionData->Checked)
			{
				fc.addConsumer(acr);
				cl->addConsumer(acr);
				acr.startWrite(
					msclr::interop::marshal_as<std::string>(
						folderBrowserDialogRecordDecisionData->SelectedPath) +
					"\\" +
					msclr::interop::marshal_as<std::string>(
						textBoxRecordDecisionDataFilename->Text) +
					timestamp() +
					".txt");
			}
			pictureBoxGesture->Visible = false;
			indexOfPreviousClass = -1;

		}
	}

	private: System::Void button34_Click_1(System::Object^  sender, System::EventArgs^  e) {
		System::Windows::Forms::DialogResult result2 = openFileDialog3->ShowDialog();
		if (result2 == System::Windows::Forms::DialogResult::OK) {
			PictureDirectory = openFileDialog3->FileName ;
			msclr::interop::marshal_context context;
			std::string PictureDirectory_std = context.marshal_as<std::string>(PictureDirectory);
			// JTY Link filepath of picture to selected gesture
			ct.selectPicture(gestureList->SelectedRows[0]->Index, PictureDirectory_std);
			updatePicture(gestureList->CurrentRow->Index);
		}
	}

	private: System::Void button35_Click_2(System::Object^  sender, System::EventArgs^  e) {
		/*try {
			String^ PictureDirectory_system = gcnew String(pictures[deviceMyo.collector.classifier_selected_class_index].c_str());

			pictureBox1->Load(PictureDirectory_system);

		}
		catch (...) {
			MessageBox::Show("NOT A PICTURE!");
		}*/
	}

	private: System::Void numericUpDown2_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
		fc.setWindowIncrement((int)numericUpDown2->Value);
	}
	private: System::Void numericUpDown3_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
		fc.setWindowSize((int)numericUpDown3->Value);
	}
	private: System::Void gestureList_CurrentCellChanged(System::Object^  sender, System::EventArgs^  e) {
		try
		{
			//NullReferenceException occurs when selecting the column header of gestureList
			textBox14->Text = gestureList->CurrentRow->Cells[1]->Value->ToString();
		}
		catch (System::NullReferenceException^ e)
		{
			return;
		}

		if (classify)
		{
			md.setFlag(gestureList->CurrentRow->Index);
		}
		else {
			updatePicture(gestureList->CurrentRow->Index);
			
		}
	}
	private: System::Void buttonClearSamples_Click(System::Object^  sender, System::EventArgs^  e) {
		int classIndex = (int)gestureList->CurrentRow->Cells[0]->Value;
		ct.deleteClassData(classIndex);
		dbuf.clearClassData(classIndex);
		gestureList->CurrentRow->Cells[2]->Value = ct.getClassSampleCount(classIndex);
	}
	private: System::Void buttonSaveModel_Click(System::Object^  sender, System::EventArgs^  e) {
		if (saveFileDialogModel->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::string filepath = context.marshal_as<std::string>(saveFileDialogModel->FileName);
			cl->saveModel(filepath);
			toolStripStatusLabelStatus->Text = L"Model saved.";
		}
	}
	private: System::Void buttonLoadModel_Click(System::Object^  sender, System::EventArgs^  e) {
		if (openFileDialogModel->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::string filepath = context.marshal_as<std::string>(openFileDialogModel->FileName);
			switch (comboBoxClassifierType->SelectedIndex)
			{
			case 0:
			case 1:
				cl = &svm;
				break;
			case 2:
				cl = &lda;
				break;
			default:
				break;
			}
			cl->loadModel(filepath);
			if (cl->modelReady())
			{
				if (ct.getNumberOfClasses() == 0)
				{
					MessageBox::Show("There are no gesture names.", "Warning",
						MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
				}
				buttonStartClassifier->Enabled = true;
				if (classifyFileData != nullptr && classifyFileData->size() > 0)
				{
					buttonClassifyTestFile->Enabled = true;
				}
				buttonSaveModel->Enabled = true;
				toolStripStatusLabelStatus->Text = L"Model loaded.";
			}
			else
			{
				MessageBox::Show("Model was not loaded.", "Model Load Error",
					MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
				buttonStartClassifier->Enabled = false;
				buttonClassifyTestFile->Enabled = false;
				buttonSaveModel->Enabled = false;
				toolStripStatusLabelStatus->Text = L"Model could not be loaded.";
			}
		}
	}
	private: System::Void buttonOuputLoad_Click(System::Object^  sender, System::EventArgs^  e) {
		if (openFileDialogOutput->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			go.loadSettings(msclr::interop::marshal_as<std::string>(openFileDialogOutput->FileName));
		}
	}
	private: System::Void buttonOutputSave_Click(System::Object^  sender, System::EventArgs^  e) {
		if (saveFileDialogOutput->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			go.saveSettings(msclr::interop::marshal_as<std::string>(saveFileDialogOutput->FileName));
		}
	}
	private: System::Void checkBoxMyoKeyboard_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		if (checkBoxMyoKeyboard->Checked)
		{
			checkBoxMyoKeyboard->Text = L"Enabled";
		}
		else
		{
			checkBoxMyoKeyboard->Text = L"Disabled";
		}
	}
	private: System::Void buttonKeyAssignment_Click(System::Object^  sender, System::EventArgs^  e) {
		SetKeyOutputDialog^ form = gcnew SetKeyOutputDialog();
		if (form->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			go.mapKey(comboBoxOutputGestureSelect->SelectedIndex, (char)form->keycode);
		}
		delete form;
	}
	private: System::Void buttonJoystickAssignment_Click(System::Object^  sender, System::EventArgs^  e) {
		SetJoyOutputDialog^ form = gcnew SetJoyOutputDialog();
		if (form->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			if (form->axis)
			{
				go.mapAxisValue(comboBoxOutputGestureSelect->SelectedIndex, form->axis, form->value);
			}
			else
			{
				go.mapButton(comboBoxOutputGestureSelect->SelectedIndex, form->value);
			}
		}
		delete form;
	}
	private: System::Void buttonBindAxis_Click(System::Object^  sender, System::EventArgs^  e) {
		//check lower and upper bounds
		float lower, upper;
		if ( (sscanf(msclr::interop::marshal_as<std::string>(textBoxInputAxisLower->Text).c_str(), "%f", &lower) != 1) ||
			(sscanf(msclr::interop::marshal_as<std::string>(textBoxInputAxisUpper->Text).c_str(), "%f", &upper) != 1) )
		{
			MessageBox::Show("Invalid Input Bound Values", "Invalid Input Bound Values",
				MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
			return;
		}

		//get input and output axis, switch lower/upper if invert
		if (checkBoxOutputAxisInvert->Checked)
		{
			go.mapAxisRange(inputAxes->at(msclr::interop::marshal_as<std::string>(comboBoxInputAxis->Text)),
				outputAxes->at(msclr::interop::marshal_as<std::string>(comboBoxOutputAxis->Text)),
				upper, lower);	//inverted
		}
		else
		{
			go.mapAxisRange(inputAxes->at(msclr::interop::marshal_as<std::string>(comboBoxInputAxis->Text)),
				outputAxes->at(msclr::interop::marshal_as<std::string>(comboBoxOutputAxis->Text)),
				lower, upper);
		}
	}
	private: System::Void buttonClearAssignment_Click(System::Object^  sender, System::EventArgs^  e) {
		go.clearMapping(comboBoxOutputGestureSelect->SelectedIndex);
	}
	private: System::Void buttonClearAxis_Click(System::Object^  sender, System::EventArgs^  e) {
		go.clearMapping(inputAxes->at(msclr::interop::marshal_as<std::string>(comboBoxInputAxis->Text)));
	}
	private: System::Void buttonOutputClearAll_Click(System::Object^  sender, System::EventArgs^  e) {
		go.clearAllMappings();
	}
	private: System::Void buttonImportData_Click(System::Object^  sender, System::EventArgs^  e) {
		//prompt file
		if (openFileDialogImportData->ShowDialog() != System::Windows::Forms::DialogResult::OK)
		{
			return;	//file prompt failed
		}

		FeatureCalculator fcalc;

		//delete all samples
		ct.clearAll();
		dbuf.clear();

		std::vector<DataVector> rawdata =
			DataReader::readFile(
				msclr::interop::marshal_as<std::string>(openFileDialogImportData->FileName),
				NUM_EMG_SENSORS
				);

		dbuf.addDataset(rawdata);
		ct.addDataset(
			fcalc.calculateFeaturesNoMix(
				rawdata,
				fc.getWindowSize(),
				fc.getWindowIncrement()
				)
			);

		//update GUI, gesturelist
		this->gestureList->CurrentCellChanged -= gcnew System::EventHandler(this, &MyForm::gestureList_CurrentCellChanged);
		gestureList->Rows->Clear();
		this->gestureList->CurrentCellChanged += gcnew System::EventHandler(this, &MyForm::gestureList_CurrentCellChanged);

		std::vector<std::pair<int, std::string>> classesVector = ct.getAllClasses();
		confusionMatrix->Rows->Clear();
		confusionMatrix->ColumnCount = ct.getNumberOfClasses() + 1;
		int i = 1;
		for (std::pair<int, std::string> pair : classesVector)
		{
			gestureList->Rows->Add(pair.first, gcnew String(pair.second.c_str()), ct.getClassSampleCount(pair.first));
			confusionMatrix->Columns[i]->Name = L"" + pair.first;
			confusionMatrix->Columns[i]->Width = 40;
			confusionMatrix->Rows->Add(L"" + pair.first);
			i++;
		}

		updateComboBoxNextGesture();

		buttonTrainGesture->Enabled = true;
		buttonDeleteAll->Enabled = true;
		buttonClearSamples->Enabled = true;
		textBox29->Text = "" + ct.getNumberOfClasses();
		toolStripStatusLabelStatus->Text = L"Training data loaded.";
	}
	private: System::Void buttonSaveData_Click(System::Object^  sender, System::EventArgs^  e) {
		if (saveFileDialogSaveData->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			dbuf.printData(msclr::interop::marshal_as<std::string>(saveFileDialogSaveData->FileName));
			toolStripStatusLabelStatus->Text = L"Training data saved.";
		}
	}

	private: System::Void buttonSaveGestures_Click(System::Object^  sender, System::EventArgs^  e) {
		if (saveFileDialogSaveData->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::ofstream gfile;
			gfile.open(msclr::interop::marshal_as<std::string>(saveFileDialogSaveData->FileName).c_str(), std::ios::ios_base::out | std::ios::ios_base::trunc);
			int ng = ct.getNumberOfClasses();
			for (int i = 0; i < ng; i++) {
				gfile << ct.getClassName(i) << "\t" << context.marshal_as<std::string>(ct.getClassPicturePath(i));
				gfile << '\n';
			}
			gfile.flush();
			gfile.close();
			//dbuf.printData(msclr::interop::marshal_as<std::string>(saveFileDialogSaveData->FileName));
			toolStripStatusLabelStatus->Text = L"Gestures Names Saved.";
		}
	}
	private: System::Void buttonLoadGestures_Click(System::Object^  sender, System::EventArgs^  e) {
		if (openFileDialogImportData->ShowDialog() != System::Windows::Forms::DialogResult::OK)
		{
			return;	//file prompt failed
		}
		std::ifstream gfile;
		//std::string line;// [256];
		char line[256];
		int index = 0;
		gfile.open(msclr::interop::marshal_as<std::string>(openFileDialogImportData->FileName).c_str(), std::ios::ios_base::in);
		
		
		while (gfile.getline(line, 256)) {
			System::String^ line_ms = gcnew System::String(line);//
			array<String^>^ parts;
			parts = line_ms->Split('\t');
			std::string name_std;
			std::string pfile_std;
			name_std = context.marshal_as<std::string>(parts[0]);
			if (parts->Length == 2) {
				pfile_std = context.marshal_as<std::string>(parts[1]);
			}
			else {
				pfile_std = "";
			}
				//msclr::interop::marshal_as<std::string>(parts[0]);
				            //#include <msclr\marshal_cppstd.h>
			
			//name = parts[0];
			//name_std.append((char*)(void*)Marshal::StringToHGlobalAnsi(parts[0]));
			if ((index + 1) > ct.getNumberOfClasses()){//new class
				addGesture(parts[0]);

			}
			else {
				 // = gcnew String(line);
				//std::string s(reinterpret_cast< char const* >(uc));
				ct.renameClass(index, name_std);
				updateGestureName(index);
			}
			ct.selectPicture(index, pfile_std);
			index++;
		}
		gestureList->CurrentCell = gestureList->Rows[0]->Cells[0];


	}

	private: System::Void checkBoxMyoJoystick_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		if (checkBoxMyoJoystick->Checked)
		{
			go.startvJoy();
			checkBoxMyoJoystick->Text = L"Enabled";
		}
		else
		{
			go.stopvJoy();
			checkBoxMyoJoystick->Text = L"Disabled";
		}
	}
	private: System::Void buttonSaveFeatures_Click(System::Object^  sender, System::EventArgs^  e) {
		if (saveFileDialogSaveData->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			DataRecorder recorder;
			std::vector<DataVector> data = ct.data();
			recorder.startWrite(msclr::interop::marshal_as<std::string>(saveFileDialogSaveData->FileName));
			for (int i = 0; i < data.size(); i++)
			{
				recorder.acceptData(data[i]);
			}
			recorder.stopWrite();
			toolStripStatusLabelStatus->Text = L"Training features saved.";
		}
	}
	private: System::Void comboBoxClassifierType_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		switch (comboBoxClassifierType->SelectedIndex)
		{
		case 0:
		case 1:
			labelSVMKernel->Visible = true;
			comboBoxSVMKernel->Visible = true;
			comboBoxSVMKernel_SelectionChangeCommitted(nullptr, nullptr);
			break;
		case 2:
			labelSVMKernel->Visible = false;
			comboBoxSVMKernel->Visible = false;
			labelSVMParamDegree->Visible = false;
			labelSVMParamGamma->Visible = false;
			labelSVMParamCoef0->Visible = false;
			numericUpDownSVMParamDegree->Visible = false;
			numericUpDownSVMParamGamma->Visible = false;
			numericUpDownSVMParamCoef0->Visible = false;
			break;
		default:
			break;
		}
	}
	private: System::Void comboBoxSVMKernel_SelectionChangeCommitted(System::Object^  sender, System::EventArgs^  e) {
		labelSVMParamDegree->Visible = false;
		labelSVMParamGamma->Visible = false;
		labelSVMParamCoef0->Visible = false;
		numericUpDownSVMParamDegree->Visible = false;
		numericUpDownSVMParamGamma->Visible = false;
		numericUpDownSVMParamCoef0->Visible = false;

		switch (comboBoxSVMKernel->SelectedIndex)
		{
		case 0:	//linear - no params
			break;
		case 1:	//poly - degree, gamma, coef0
			labelSVMParamDegree->Visible = true;
			labelSVMParamGamma->Visible = true;
			labelSVMParamCoef0->Visible = true;
			numericUpDownSVMParamDegree->Visible = true;
			numericUpDownSVMParamGamma->Visible = true;
			numericUpDownSVMParamCoef0->Visible = true;
			break;
		case 2:	//rbf - gamma
			labelSVMParamGamma->Visible = true;
			numericUpDownSVMParamGamma->Visible = true;
			break;
		case 3: //sigmoid - gamma, coef0
			labelSVMParamGamma->Visible = true;
			labelSVMParamCoef0->Visible = true;
			numericUpDownSVMParamGamma->Visible = true;
			numericUpDownSVMParamCoef0->Visible = true;
			break;
		default:
			break;
		}
	}
	private: System::Void buttonRecordDecisionDataDialog_Click(System::Object^  sender, System::EventArgs^  e) {
		folderBrowserDialogRecordDecisionData->ShowDialog();
	}
	private: System::Void gestureList_CellValueChanged(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e) {
		try
		{
			String^ name = (String^)gestureList->Rows[e->RowIndex]->Cells[1]->Value;
			ct.renameClass((int)gestureList->Rows[e->RowIndex]->Cells[0]->Value, msclr::interop::marshal_as<std::string>(name));
			updateComboBoxNextGesture();
		}
		catch (...)
		{

		}
	}
	private: System::Void buttonSetNowNextGesture_Click(System::Object^  sender, System::EventArgs^  e) {
		md.setFlag(comboBoxNextGesture->SelectedIndex - 1);
	}
	private: System::Void buttonSetDelayNextGesture_Click(System::Object^  sender, System::EventArgs^  e) {
		nextGestureDelay->Interval = 10;
		nextGestureDelay->Start();
		progressBarNextGestureWarning->Value = progressBarNextGestureWarning->Maximum;
		textBoxNextGestureWarning->Visible = true;
		progressBarNextGestureWarning->Visible = true;
	}
	private: System::Void nextGestureDelay_Tick(System::Object^  sender, System::EventArgs^  e) {
		progressBarNextGestureWarning->Value -= 2;
		String^ nextGestureName = L"null";
		if (comboBoxNextGesture->SelectedIndex > 0)
		{
			nextGestureName = gcnew String(ct.getClassName(comboBoxNextGesture->SelectedIndex - 1).c_str());
		}
		if (progressBarNextGestureWarning->Value == 0)
		{
			md.setFlag(comboBoxNextGesture->SelectedIndex - 1);
			nextGestureDelay->Stop();
			textBoxNextGestureWarning->Text =
				L"Do '" +
				nextGestureName +
				L"'";
			progressBarNextGestureWarning->Visible = false;
		}
		else
		{
			textBoxNextGestureWarning->Text =
				L"Do '" +
				nextGestureName +
				L"' in " +
				((progressBarNextGestureWarning->Value / 100) + 1) +
				L"...";
		}
	}
	private: System::Void nextTrainGestureDelay_Tick(System::Object^  sender, System::EventArgs^  e) {
		progressBarTrainNextGesture->Value -= 2;
		String^ nextGestureName = L"null";
		if ((int)gestureList->CurrentRow->Cells[0]->Value < (int)gestureList->Rows->Count)
		{
			nextGestureName = gcnew String(ct.getClassName((int)gestureList->CurrentRow->Cells[0]->Value).c_str());
		}
		if (progressBarTrainNextGesture->Value == 0 && !cancelAutoTrain)
		{
			md.setFlag(comboBoxNextGesture->SelectedIndex - 1);
			nextTrainGestureDelay->Stop();
			textBoxTrainNextGestureWarning->Text =
				L"Do '" +
				nextGestureName +
				L"'";
			progressBarTrainNextGesture->Visible = false;

			md.mute();

			md.setFlag((int)gestureList->CurrentRow->Cells[0]->Value);
			md.addConsumer(dbuf);
			fc.reset();	//reset buffer
			fc.addConsumer(ct);
			ct.startCollect((int)numericUpDown4->Value);

			buttonTrainGesture->Text = L"Stop";
			training = true;
			readyToTrainGesture = true;
			gestureList->Enabled = false;

			md.unmute();
		}
		else
		{
			textBoxTrainNextGestureWarning->Text =
				L"Do '" +
				nextGestureName +
				L"' in " +
				((progressBarTrainNextGesture->Value / 100) + 1) +
				L"...";
		}

		if (cancelAutoTrain) {
			nextTrainGestureDelay->Stop();
			buttonTrainGesture->Text = L"Train";
			training = false;
			readyToTrainGesture = true;
			cancelAutoTrain = false;
		}
		
	}
	private: System::Void radioButtonClassifier_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		if (radioButtonClassifyLive->Checked)
		{
			groupBoxClassifyFile->Enabled = false;
			groupBoxClassifyLive->Enabled = true;
		}
		else if (radioButtonClassifyFile->Checked)
		{
			groupBoxClassifyLive->Enabled = false;
			groupBoxClassifyFile->Enabled = true;
		}
	}
	private: System::Void buttonClassifyLoadData_Click(System::Object^  sender, System::EventArgs^  e) {
		//prompt file
		if (openFileDialogClassifyFile->ShowDialog() != System::Windows::Forms::DialogResult::OK)
		{
			return;	//file prompt failed
		}

		//check source if raw data or features
		int dataLength = NUM_EMG_SENSORS;
		if (radioButtonClassifyFeatures->Checked)
		{
			dataLength = FtNm * NUM_EMG_SENSORS;
		}

		std::vector<DataVector> data =
			DataReader::readFile(
				msclr::interop::marshal_as<std::string>(openFileDialogClassifyFile->FileName),
				dataLength
				);
		if (data.size() > 0)
		{
			if (classifyFileData) {
				delete classifyFileData;
			}
			classifyFileData = new std::vector<DataVector>(data);
			textBoxClassifyFile->Text = openFileDialogClassifyFile->FileName;

			if (radioButtonClassifyFeatures->Checked)
			{
				toolStripStatusLabelStatus->Text = L"Feature file loaded.";
			}
			else
			{
				toolStripStatusLabelStatus->Text = L"EMG file loaded.";
			}
			if (cl != nullptr && cl->modelReady())
			{
				buttonClassifyTestFile->Enabled = true;
			}
		}
	}
	private: System::Void buttonClassifyTestFile_Click(System::Object^  sender, System::EventArgs^  e) {
		//make local copy of classifyFileData
		std::vector<DataVector> data = *classifyFileData;
		DataVector decisionVec(0, 0);
		cl->addConsumer(acr);
		acr.startWrite(
			msclr::interop::marshal_as<std::string>(
				folderBrowserDialogRecordDecisionData->SelectedPath) +
			"\\" +
			msclr::interop::marshal_as<std::string>(
				textBoxRecordDecisionDataFilename->Text) +
			timestamp() +
			".txt");

		//replace data (raw) with data (features)
		if (radioButtonClassifyData->Checked)
		{
			FeatureCalculator fcalc;
			data = fcalc.calculateFeatures(data, fc.getWindowSize(), fc.getWindowIncrement());
		}

		//iterate through feature windows
		for (DataVector window : data)
		{
			//predict
			decisionVec.flag = cl->predict(window);
			decisionVec.timestamp = window.timestamp;
			//pass window and decision to acr
			acr.acceptData(window);
			acr.acceptData(decisionVec);
		}

		cl->removeConsumer(acr);
		acr.stopWrite();
		toolStripStatusLabelStatus->Text = L"Decision data saved.";
	}


private: System::Void pictureBoxGesture_Click(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void groupBox1_Enter(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void confusionMatrix_CellContentClick(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e) {
}
private: System::Void gestureList_CellContentClick(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e) {
}
private: System::Void textBoxNextGestureWarning_TextChanged(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void radioButtonClassifyData_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void labelSVMParamCoef0_Click(System::Object^  sender, System::EventArgs^  e) {
}

private: System::Void checkBoxAutoRun_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (checkBoxAutoRun->Checked) {
		label1->Visible = true;
		textBoxAutoRunSeconds->Visible = true;
	}
	else {
		label1->Visible = false;
		textBoxAutoRunSeconds->Visible = false;
	}
}

private: System::Void checkBoxOrientation_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (checkBoxOrientation->Checked) {
		fc.setOrienOnOff(true);
	}
	else {
		fc.setOrienOnOff(false);
	}
}
private: System::Void checkBoxGyro_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (checkBoxGyro->Checked) {
		fc.setGyroOnOff(true);
	}
	else {
		fc.setGyroOnOff(false);
	}
}
private: System::Void checkBoxAcc_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (checkBoxAcc->Checked) {
		fc.setAccelOnOff(true);
	}
	else {
		fc.setAccelOnOff(false);
	}
}
private: System::Void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {
	if (dataGridFeatureSelect->RowCount == 0)
	{
		dataGridFeatureSelect->Rows->Add(5);
		dataGridFeatureSelect->Rows[0]->HeaderCell->Value = "MAV";
		dataGridFeatureSelect->Rows[1]->HeaderCell->Value = "WAVE";
		dataGridFeatureSelect->Rows[2]->HeaderCell->Value = "ZEROS";
		dataGridFeatureSelect->Rows[3]->HeaderCell->Value = "TURNS";
		dataGridFeatureSelect->Rows[4]->HeaderCell->Value = "MAV/MMAV";
		// Set all features for all channels on by default
		for (int i = 0; i < dataGridFeatureSelect->Rows->Count; i++) {
			for (int j = 0; j < dataGridFeatureSelect->Columns->Count; j++) {
				dataGridFeatureSelect->Rows[i]->Cells[j]->Value = true;
			}
		}
		featSelectGridLoaded = true;
	}
}
private: System::Void dataGridFeatureSelect_CellMouseUp(System::Object^  sender, System::Windows::Forms::DataGridViewCellMouseEventArgs^  e) {
	// End edit on each click on "All" of dataGridFeatureSelect
	if (e->ColumnIndex == 0 && e->RowIndex != -1)
	{
		dataGridFeatureSelect->EndEdit();
	}
}
private: System::Void dataGridFeatureSelect_CellValueChanged(System::Object^  sender, System::Windows::Forms::DataGridViewCellEventArgs^  e) {
	if (featSelectGridLoaded && e->RowIndex != -1) {
		// If an "All" box is checked, update row
		if (e->ColumnIndex == 0) {
			if (dataGridFeatureSelect->Rows[e->RowIndex]->Cells[0]->Value->ToString() == "True") {
				for (int i = 1; i < dataGridFeatureSelect->Columns->Count; i++) {
					dataGridFeatureSelect->Rows[e->RowIndex]->Cells[i]->Value = true;
					fc.addFeatToChannel(e->RowIndex, (i - 1));
				}
			}
			else {
				for (int i = 1; i < dataGridFeatureSelect->Columns->Count; i++) {
					dataGridFeatureSelect->Rows[e->RowIndex]->Cells[i]->Value = false;
					fc.remFeatFromChannel(e->RowIndex, (i - 1));
				}
			}
		}
		// Update for non-"All" check boxes
		else {
			if (dataGridFeatureSelect->Rows[e->RowIndex]->Cells[e->ColumnIndex]->Value->ToString() == "True") {
				fc.addFeatToChannel(e->RowIndex, (e->ColumnIndex - 1));
			}
			else {
				fc.remFeatFromChannel(e->RowIndex, (e->ColumnIndex - 1));
			}
		}
	}
}

private: System::Void numericUpDownSCThresh_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
	sf.setThresh((float)numericUpDownSCThresh->Value / 170.0f);
}
private: System::Void numericUpDownSCValuesInAvg_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
	sf.setValuesInAvg((int)numericUpDownSCValuesInAvg->Value);
}
private: System::Void numericUpDownSSorTransWinsize_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
	sst.setWinsize((int)numericUpDownSSorTransWinsize->Value);
}
private: System::Void numericUpDownSSorTransThresh_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
	sst.setThresh((float)(numericUpDownSSorTransThresh->Value));
}
private: System::Void numericUpDownMMAVThresh_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
	mpf.setThresh((float)(numericUpDownMMAVThresh->Value));
}

private: System::Void buttonBitAssignment_Click(System::Object^  sender, System::EventArgs^  e) {
	SetBitOutputDialog^ form = gcnew SetBitOutputDialog();
	if (form->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		//go.mapBit(comboBoxOutputGestureSelect->SelectedIndex, (int)form->bit);
	}
	delete form;
}
private: System::Void buttonLoadVR_Click(System::Object^  sender, System::EventArgs^  e) {
	// Default bit-gesture assignments for VR game output
	// Gesture index 0 -> bit 0
	// Gesture index 1 -> bit 1
	// ...
	// Gesture index 7 -> bit 7
	/*for(int i = 0; i < 7; i++)
		go.mapBit(i, i);*/
}
private: System::Void radioButtonContinuous_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (radioButtonContinuous->Checked)
		pipelineControlSceme = 0; // continuous stream 
	else
		pipelineControlSceme = 1; // on transition + mmav peak
}
};

}