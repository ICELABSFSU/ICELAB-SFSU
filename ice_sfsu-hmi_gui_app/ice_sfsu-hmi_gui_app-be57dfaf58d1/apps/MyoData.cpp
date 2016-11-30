#include "MyoData.h"
#include <cmath>
#include <algorithm>


/**
Connect and initialize myo
Start thread on which hub will run

*/
MyoData::MyoData()
{
}

MyoData::~MyoData()
{
}

void MyoData::acceptData(const DataVector& dvec)
{
	//do nothing
	//inputs come from myo events
}

void MyoData::setFlag(int newFlag)
{
	flag = newFlag;
}
int MyoData::getFlag() const {
	return flag;
}
myo::Arm MyoData::getMyoArm() const
{
	return arm;
}

myo::WarmupState MyoData::getWarmupState() const
{
	return warmupState;
}

myo::XDirection MyoData::getXDirection() const
{
	return xDirection;
}

int MyoData::getBatteryLevel() const
{
	return batteryLevel;
}

void MyoData::myoEnableStreamingEMG()
{
	myo->setStreamEmg(myo::Myo::streamEmgEnabled);
}

void MyoData::myoDisableStreamingEMG()
{
	myo->setStreamEmg(myo::Myo::streamEmgDisabled);
}

void MyoData::collectIMU()
{
	bufferImu[0] = this->roll(quat);
	bufferImu[1] = this->pitch(quat);
	bufferImu[2] = this->yaw(quat);
	bufferImu[3] = gyro.x();
	bufferImu[4] = gyro.y();
	bufferImu[5] = gyro.z();
	bufferImu[6] = accel.x();
	bufferImu[7] = accel.y();
	bufferImu[8] = accel.z();
}

void MyoData::myoLock()
{
	myo->lock();
}

void MyoData::myoUnlock()
{
	myo->unlock(myo::Myo::unlockTimed);
}

float MyoData::emgData(const int sensor) const
{
	return (sensor < NUM_EMG_SENSORS && sensor >= 0) ? bufferEmg[sensor] : 0;
}

myo::Quaternion<float> MyoData::orientation() const
{
	return quat;
}

myo::Vector3<float> MyoData::acceleration() const
{
	return accel;
}

myo::Vector3<float> MyoData::gyroscope() const
{
	return gyro;
}

float MyoData::pitch(myo::Quaternion<float> quat)
{
	return std::asin(std::max(-1.0f, std::min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
}

float MyoData::roll(myo::Quaternion<float> quat)
{
	return std::atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
		1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
}

float MyoData::yaw(myo::Quaternion<float> quat)
{
	return std::atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
		1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
}

void MyoData::onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{
	this->myo = myo;
}

void MyoData::onUnpair(myo::Myo* myo, uint64_t timestamp)
{
	//this->myo = nullptr;
	arm = myo::armUnknown;
	warmupState = myo::warmupStateUnknown;
}

void MyoData::onConnect(myo::Myo * myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
{
	this->myo = myo;
}

void MyoData::onDisconnect(myo::Myo* myo, uint64_t timestamp)
{
	//this->myo = nullptr;
	arm = myo::armUnknown;
	warmupState = myo::warmupStateUnknown;
}

void MyoData::onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
	myo::WarmupState warmupState)
{
	this->arm = arm;
	this->warmupState = warmupState;
	this->xDirection = xDirection;
}

void MyoData::onArmUnsync(myo::Myo* myo, uint64_t timestamp)
{
	//myo->setStreamEmg(myo::Myo::streamEmgDisabled);
	//this->myo = nullptr;
	arm = myo::armUnknown;
	warmupState = myo::warmupStateUnknown;
}

// onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
void MyoData::onUnlock(myo::Myo* myo, uint64_t timestamp)
{
}

// onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
void MyoData::onLock(myo::Myo* myo, uint64_t timestamp)
{
}

void MyoData::onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
{
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

void MyoData::onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
{
	this->quat = quat;
}

void MyoData::onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& accel)
{
	this->accel = accel;
}

void MyoData::onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
{
	this->gyro = gyro;
	collectIMU();
	notifyAll(DataVector(true, flag, imuDim, bufferImu, timestamp));
}

void MyoData::onBatteryLevelReceived(myo::Myo* myo, uint64_t timestamp, uint8_t level)
{
	batteryLevel = level;
}

void MyoData::onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emgarg)
{
	for (int i = 0; i < length; i++)
	{
		bufferEmg[i] = (float)emgarg[i];
	}
	notifyAll(DataVector(false, flag, length, bufferEmg, timestamp));
}

void MyoData::onWarmupCompleted(myo::Myo * myo, uint64_t timestamp, myo::WarmupResult warmupResult)
{
	warmupState = myo::warmupStateWarm;
}
