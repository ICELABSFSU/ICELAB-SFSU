// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES

#define Buff 128
#define Sigma 5
#define	FtNm 4
#define numSensors 8

#define maxClasses 30
#define maxWindowNumber 1024
#define maxFeatures 32

#define M_PI       3.14159265358979323846

#include "INCLUSIONS.h"


// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>



// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
	
	DataCollector()
		: onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
	{
	}

	// This is a utility function implemented for this sample that maps a myo::Myo* to a unique ID starting at 1.
	// It does so by looking for the Myo pointer in knownMyos, which onPair() adds each Myo into as it is paired.
	size_t identifyMyo(myo::Myo* myo) {
		// Walk through the list of Myo devices that we've seen pairing events for.
		for (size_t i = 0; i < knownMyos.size(); ++i) {
			// If two Myo pointers compare equal, they refer to the same Myo device.
			if (knownMyos[i] == myo) {
				return i + 1;
			}
		}

		return 0;
	}

	// We store each Myo pointer that we pair with in this list, so that we can keep track of the order we've seen
	// each Myo and give it a unique short identifier (see onPair() and identifyMyo() above).
	std::vector<myo::Myo*> knownMyos;

	void onBatteryLevelReceived(myo::Myo* myo, uint64_t timestamp, uint8_t level) {
	
		if (identifyMyo(myo) == 1){
			battery1 = level;
		}
		else {
			battery2 = level;
		}
		
	}

	void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion) {
		
		knownMyos.push_back(myo);

	}

	// onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
	void onUnpair(myo::Myo* myo, uint64_t timestamp)
	{
		// We've lost a Myo.
		// Let's clean up some leftover state.
		roll_w = 0;
		pitch_w = 0;
		yaw_w = 0;
		onArm = false;
		roll_d = 0;
		pitch_d = 0;
		yaw_d = 0;
		emgSamples[8][Buff][2] = { 0 };
		x_accel = 0;
		y_accel = 0;
		z_accel = 0;
		mag_accel = 0;
		x_gyro = 0;
		y_gyro = 0;
		z_gyro = 0;
		mag_gyro = 0;


		isUnlocked = false;
	}

	// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
	// as a unit quaternion.
	void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
	{
		using std::atan2;
		using std::asin;
		using std::sqrt;
		using std::max;
		using std::min;

		accelSamples[4][Buff] = { 0 };
		gyroSamples[4][Buff] = { 0 };
		oriSamples[3][Buff] = { 0 };
		// Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
		float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
			1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
		float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
		float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
			1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));

		// Convert the floating point angles in radians to a scale from 0 to 18. FOR CLR PROGRAM
		/*roll_w = static_cast<int>((roll + (float)M_PI) / (M_PI * 2.0f) * 18);
		pitch_w = static_cast<int>((pitch + (float)M_PI / 2.0f) / M_PI * 18);
		yaw_w = static_cast<int>((yaw + (float)M_PI) / (M_PI * 2.0f) * 18);
		*/ 
		// Convert Euler angles to radians
		roll_d = roll * 180 / (float)M_PI;
		pitch_d = pitch*180/ (float)M_PI;
		yaw_d = yaw*180/ (float)M_PI;

		float test = roll_d;
		int test2 = iteration_imu;

		if (identifyMyo(myo) == 1 || identifyMyo(myo) == 0){
			IMUSamples[0][iteration_imu][0] = roll_d;
			IMUSamples[1][iteration_imu][0] = pitch_d;
			IMUSamples[2][iteration_imu][0] = yaw_d;
	
		}
		else if (identifyMyo(myo) == 2){
			IMUSamples[0][iteration_imu][1] = roll_d;
			IMUSamples[1][iteration_imu][1] = pitch_d;
			IMUSamples[2][iteration_imu][1] = yaw_d;
		}
		iteration_ori++; //original
	}

	void onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& accel)
	{
		if (identifyMyo(myo) == 1 || identifyMyo(myo) == 0){
			IMUSamples[3][iteration_imu][0] = accel.x();
			y_accel = accel.y();
			z_accel = accel.z();
			mag_accel = accel.magnitude();
			 
			IMUSamples[4][iteration_imu][0] = y_accel;
			IMUSamples[5][iteration_imu][0] = z_accel;
		}
		else if (identifyMyo(myo) == 2){
			x_accel = accel.x();
			y_accel = accel.y();
			z_accel = accel.z();
			mag_accel = accel.magnitude();
			IMUSamples[3][iteration_imu][1] = x_accel;
			IMUSamples[4][iteration_imu][1] = y_accel;
			IMUSamples[5][iteration_imu][1] = z_accel;
		}
		iteration_accel = iteration_accel + 1; // #RF needs to be implemented for two bands 

	}

	void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro) {

		int test = (int)identifyMyo(myo);

		if (identifyMyo(myo) == 1 || identifyMyo(myo) == 0){
			x_gyro = gyro.x();
			y_gyro = gyro.y();
			z_gyro = gyro.z();
			mag_gyro = gyro.magnitude();

			IMUSamples[6][iteration_imu][0] = x_gyro;
			IMUSamples[7][iteration_imu][0] = y_gyro;
			IMUSamples[8][iteration_imu][0] = z_gyro;
		}
		else if (identifyMyo(myo) == 2){
			x_gyro = gyro.x();
			y_gyro = gyro.y();
			z_gyro = gyro.z();
			mag_gyro = gyro.magnitude();

			IMUSamples[6][iteration_imu][1] = x_gyro;
			IMUSamples[7][iteration_imu][1] = y_gyro;
			IMUSamples[8][iteration_imu][1] = z_gyro;
		}
		iteration_gyro++;
		
	}



	void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	{
		
		
		for (int a(0); a < 8; a++)
		{
			emgSamples[a][iteration_emg][0] = emg[a];
		}
		iteration_emg++;




		iteration_emg &= 127;

		//featCalc();
		
	}
	
	void onConnect(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion) {
		//May not be useful, connection generally happens befor the program is started
		
	}



	void onDisconnect(myo::Myo* myo, uint64_t timestamp)
	{
		roll_w = 0;
		pitch_w = 0;
		yaw_w = 0;
		onArm = false;
		roll_d = 0;
		pitch_d = 0;
		yaw_d = 0;
		emgSamples[8][Buff][2] = { 0 };
		x_accel = 0;
		y_accel = 0;
		z_accel = 0;
		mag_accel = 0;
		x_gyro = 0;
		y_gyro = 0;
		z_gyro = 0;
		mag_gyro = 0;

	}



	// onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
	// making a fist, or not making a fist anymore.
	void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
	{
		currentPose = pose;

		if (pose != myo::Pose::unknown && pose != myo::Pose::rest) {
			// Tell the Myo to stay unlocked until told otherwise. We do that here so you can hold the poses without the
			// Myo becoming locked.
			myo->unlock(myo::Myo::unlockHold);

			// Notify the Myo that the pose has resulted in an action, in this case changing
			// the text on the screen. The Myo will vibrate.
			myo->notifyUserAction();
		}
		else {
			// Tell the Myo to stay unlocked only for a short period. This allows the Myo to stay unlocked while poses
			// are being performed, but lock after inactivity.
			myo->unlock(myo::Myo::unlockTimed);
		}
	}

	// onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
	// arm. This lets Myo know which arm it's on and which way it's facing.
	void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
		myo::WarmupState warmupState)
	{
		onArm = true;
		whichArm = arm;
	}

	// onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
	// it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
	// when Myo is moved around on the arm.
	void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
	{
		onArm = false;
	}

	// onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
	void onUnlock(myo::Myo* myo, uint64_t timestamp)
	{
		isUnlocked = true;
	}

	// onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
	void onLock(myo::Myo* myo, uint64_t timestamp)
	{
		isUnlocked = false;
		
	}

	// There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
	// For this example, the functions overridden above are sufficient.

	// We define this function to print the current values that were updated by the on...() functions above.

	void featCalc()
	{
		//if (((iteration_emg-lastCall)&127) >= sampling)
	//	if ((iteration_emg - lastCall) >= sampling || (iteration_emg - lastCall) <= -sampling) // db && activate_feature == true)
		{

			char signLast = 0;
			char slopLast = 0;

			lastCall = (lastCall + sampling)&127;
			int p = lastCall;
			// iter
			int firstcall = (lastCall - window_length) & 127;

		

			

			for (int a(0); a < 8; a++)
			{
				for (int f = 0; f < 4;f++){FeatEMG[f][a] = 0;}


				int j = firstcall;

				int p_firstcall = (firstcall - 1) & 127;

				double Delta_2 = emgSamples[a][p_firstcall][0] - emgSamples[a][firstcall][0];

				if (emgSamples[a][firstcall][0] > threshold) { signLast = 4; }
				if (emgSamples[a][firstcall][0] < -threshold) { signLast = 8; }
				if (Delta_2 > threshold) { slopLast = 4; }
				if (Delta_2 < -threshold) { slopLast = 8; }

				for (int i(0); i < window_length; i++)
				{

					int k = (j + 1) & 127;
					Delta_2 = emgSamples[a][k][0] - emgSamples[a][j][0];

					if (emgSamples[a][k][0] > threshold) { signLast += 1; }
					if (emgSamples[a][k][0] < -threshold) { signLast += 2; }
					
					if (Delta_2 > threshold) { slopLast += 1; }
					if (Delta_2 < -threshold) { slopLast += 2; }

					if (signLast == 9 || signLast == 6) {FeatEMG[2][a]++;}
					if (slopLast == 9 || slopLast == 6) {FeatEMG[3][a]++;}

					signLast = (signLast << 2) & 15;
					slopLast = (slopLast << 2) & 15;

					FeatEMG[0][a] += abs(emgSamples[a][k][0]);
					FeatEMG[1][a] += abs(Delta_2);

					j = k;
				}
				for (char f = 0;f<2;f++) {
					FeatEMG[f][a] = FeatEMG[f][a] / window_length;
					FeatEMG[f+2][a] = FeatEMG[f+2][a] * 100 / window_length;
				}
		
			}

			//iteration_emg = 0;
			state_end = true;
		}




	}
	
	/* db delete, this function moved to MyForm.cpp
	
	void classifier_training() {

		if (avg_mav >= (float)threshold_mmav && armed) {

			for (int i = 0; i < FtNm; i++) {

				for (int j = 0; j < numSensors; j++) {

					classifierTrainer[classifier_selected_class_index][window_index][feature_index] = FeatEMG[i][j];
					feature_index++;
					// Sensor1FeaT1, Sens2Feat1, SEns3Feat1, ... // Sensor1Feat2, Sens2Feat2..etc
				}

			}
			
			feature_index = 0;
			window_index++;

		}

		if (window_index == maxWindowNumber) {
			armed = false;
			numWC[classifier_selected_class_index] = window_index;
			window_index = 0;
			
		}

	}
	
	
	*/
	


	void load_trainer_array() {
	
		for (i = 0; i < height; i++)
		{
			classifierTrainer[i] = new float*[width]();
			for (j = 0; j < width; j++)
				classifierTrainer[i][j] = new float[depth]();
		}


		// [height][width][depth]
	}
	
	int height = maxClasses;
	int width = maxWindowNumber;
	int depth = maxFeatures;
	int i, j;
	float *** classifierTrainer = new float **[height]();
	
	int numWC[maxClasses];

	// classifier_training
	float avg_mav = 0;

	int classifier_selected_class_index = 0;
	int currentTotalClasses = 0;

	bool armed = false;

	int window_index = 0;

	int feature_index = 0;

	int threshold_mmav = 0;

	int lastFE_iteration = 0;

	// These values are set by onArmSync() and onArmUnsync() above.
	bool onArm;
	myo::Arm whichArm;

	// This is set by onUnlocked() and onLocked() above.
	bool isUnlocked;

	// These values are set by onOrientationData() and onPose() above.
	float roll_w, pitch_w, yaw_w, roll_d, pitch_d, yaw_d;
	myo::Pose currentPose;

	// These values are set by onAccelerationData().
	float x_accel, y_accel, z_accel, mag_accel;

	// set by onGyroscopeData().
	float x_gyro, y_gyro, z_gyro, mag_gyro;

	//std::array<int8_t, 8> emgSamples;

	int iteration_imu = 0;
	int iteration_emg = 0;

	int iterationFeat = 0;

	int iteration_accel = 0;
	int iteration_gyro = 0;
	int iteration_ori = 0;

	int iteration_emg2 = 0;

	int battery1 = 0;
	int battery2 = 0;

	int bufferDataEMG[8][128]; // circular buffer for streaming data

	int myoId;

	float IMUSamples[9][Buff][2];
	float FeatIMU[9][Buff][2];

	float FeatEMG[FtNm][numSensors];


	/*
	Feature Vector (FeatV) indices

	WAVE	0
	MAV		1
	Zero	2
	Turn	3
	*/
	float FeatEMGAvg[FtNm];


	//float LDATrainer[classNum][Windows][numFeatures];
	//num window/calss = #WL

	//float * LDATrainer = (float*)malloc([30][256][64] = { 0 }; // [# of Gestures/classes][max # windows][features]
													   //int numWC[30];



	/////////////////////////////////////////////////////////////////////////////////////
	// Gregorie's Variables

	float accelSamples[4][Buff];
	float gyroSamples[4][Buff];
	float oriSamples[3][Buff];

	int lastCall = 0;

	int threshold = Sigma;

	// window increment
	int sampling = 8;

	// window length - EMG
	int window_length = 40;
	

	int emgSamples[8][Buff][2];
	float Data[8][Buff];
	
	float DELTA[8][Buff];
	float Previous_emg[8];
	float Previous_delta[8];
	bool state_end;
	bool activate_feature = true;
	bool activate_gyro = true;
	bool activate_accel = true;
	bool activate_ori = true;
	bool activate_emg = true;
	bool connected = false;

	uint64_t timestamp1[2000];
	// FFT
	//typedef std::complex<double> cx;
	//cx result[8][Buff];
	double Ampli[8][Buff];
	double MMNF[8];
	//////////////////////////////////////////////////////////////////////////////

};