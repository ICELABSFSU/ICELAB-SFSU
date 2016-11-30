#pragma once
#include <windows.h>
#include <map>
#include "public.h"
#include "vjoyinterface.h"

#define VJOY_AXIS_RANGE	0x8000
#define NUM_SETTABLE_BUTTONS 128

struct ControlMapping {
	int joy;	//0 if keyboard
	int axis;	//for joy, 0 if button
	int range;	//set if mapped continuous range
	union {
		char key;	//keyboard key
		int bit;	// ?
		int value;	//for joy, button index/axis value
		struct {
			float scalar;	//for axis range
			float offset;	//for axis range
		};
	};
};

class GestureOutput
{
private:
	std::map<int, ControlMapping> gestureOutput;

	UINT iInterface = 1;	//vJoy device

	inline void setAxis(int axis, int value);
	inline void pressButton(int button);
	inline void releaseButton(int button);
	inline void pressKey(char key);
	inline void releaseKey(char key);
public:
	GestureOutput();
	virtual ~GestureOutput();

	void setInput(int index);
	void unsetInput(int index);
	void setInputAxis(int index, float value);
	void resetInputAxis(int index);
	//screen dimensions scaled from 0 to 0xFFFF
	void setMousePos(int x, int y);

	void mapKey(int index, char key);
	void mapBit(int index, int bit);
	void mapButton(int index, int button);
	void mapAxisValue(int index, int axis, int value);
	void mapAxisRange(int index, int axis, float lower, float upper);

	void clearMapping(int index);
	void clearAllMappings();

	int startvJoy();
	void stopvJoy();

	int loadSettings(std::string filename);
	int saveSettings(std::string filename);
};


