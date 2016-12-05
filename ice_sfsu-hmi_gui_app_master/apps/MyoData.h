#pragma once

#include "DataAgent.h"
#include "myo\myo.hpp"

#define NUM_EMG_SENSORS 8
#define IMU_DIM 9

/**
MyoData
This class handles data collection from the Myo device.
Add consumers (DataAgent) to this objects as needed. Consumers are notified and
pushed the data once the sampling has been received from the Myo device. 

This object must be added as listener to myo::Hub.
*/
class MyoData : public DataAgent, public myo::DeviceListener {
private:
	int flag = -1;
	int length = NUM_EMG_SENSORS;
	float bufferEmg[NUM_EMG_SENSORS];
	int imuDim = IMU_DIM;
	// JTY hold IMU data OrRoll, OrPitch, OrYaw, GyrX, ..., AccX, ...
	float bufferImu[IMU_DIM];
	myo::Quaternion<float> quat;
	myo::Vector3<float> accel;
	myo::Vector3<float> gyro;

	myo::Arm arm;
	myo::WarmupState warmupState;
	myo::XDirection xDirection;
	int batteryLevel;
public:
	myo::Myo* myo;

	MyoData();
	~MyoData();

	// Inherited via DataAgent
	virtual void acceptData(const DataVector & dvec) override;

	void setFlag(int newFlag);
	int getFlag() const;
	myo::Arm getMyoArm() const;
	myo::WarmupState getWarmupState() const;
	myo::XDirection getXDirection() const;
	int getBatteryLevel() const;
	//set myo device to stream EMG data
	void myoEnableStreamingEMG();
	//set myo device to disable streaming of EMG data
	void myoDisableStreamingEMG();
	//collect IMU data into a bufferImu
	void collectIMU();
	//prevent myo device from sending poses
	void myoLock();
	//allow myo device poses
	void myoUnlock();

	// Return latest EMG data from sensor
	float emgData(const int sensor) const;
	// Return latest orientation (Quaternion)
	myo::Quaternion<float> orientation() const;
	// Return latest acceleration (Vector3)
	myo::Vector3<float> acceleration() const;
	// Return latest gyroscope (Vector3)
	myo::Vector3<float> gyroscope() const;


	//Quaternion converions functions
	
	// Return pitch angle from given quaternion
	static float pitch(myo::Quaternion<float> quat);
	// Return roll angle from quaternion
	static float roll(myo::Quaternion<float> quat);
	// Return yaw angle from quaternion
	static float yaw(myo::Quaternion<float> quat);

	void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
	void onUnpair(myo::Myo* myo, uint64_t timestamp);
	void onConnect(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion);
	void onDisconnect(myo::Myo* myo, uint64_t timestamp);
	void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
		myo::WarmupState warmupState);
	void onArmUnsync(myo::Myo* myo, uint64_t timestamp);
	void onUnlock(myo::Myo* myo, uint64_t timestamp);
	void onLock(myo::Myo* myo, uint64_t timestamp);
	void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose);
	void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat);
	void onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& accel);
	void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro);
	void onBatteryLevelReceived(myo::Myo* myo, uint64_t timestamp, uint8_t level);
	void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emgarg);
	void onWarmupCompleted(myo::Myo* myo, uint64_t timestamp, myo::WarmupResult warmupResult);
};

