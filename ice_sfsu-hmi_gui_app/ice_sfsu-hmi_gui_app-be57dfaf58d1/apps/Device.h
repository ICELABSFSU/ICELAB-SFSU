#include "INCLUSIONS.h"
#include "myo\Data.hpp"


public class Device
{
public:
	int a;

	DataCollector collector;

	myo::Hub hub;


	myo::Myo* myo;
	myo::Myo* myo2;

	Device()
	{
		hub.setLockingPolicy(myo::Hub::lockingPolicyStandard);
	}

	void stop_emg(void)
	{
		myo->setStreamEmg(myo::Myo::streamEmgDisabled);
	}

	void resume_emg(void)
	{
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);

	}

	void stop_listener(void)
	{
		hub.removeListener(&collector);
	}

	void resume_listener(void)
	{
		hub.addListener(&collector);
	}

	void connect(void)
	{
		myo = hub.waitForMyo(1000);
		myo2 = hub.waitForMyo(1000); //db
	
	}

	~Device()
	{

	}
};

