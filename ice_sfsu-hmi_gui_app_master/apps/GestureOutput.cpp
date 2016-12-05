#include "GestureOutput.h"
#include <WinUser.h>
#pragma comment(lib, "user32.lib")
#include <vector>

//INI file WCHAR definitions
#define WCHAR_BUF_SIZE	80
#define APPNAME_LABEL	L"class."
#define KEYNAME_JOY		L"joy"
#define KEYNAME_AXIS	L"axis"
#define KEYNAME_RANGE	L"range"
#define KEYNAME_KEYBD	L"keyboard"
#define KEYNAME_VALUE	L"value"
#define KEYNAME_SCALAR	L"scalar"
#define KEYNAME_OFFSET	L"offset"


GestureOutput::GestureOutput()
{
}


GestureOutput::~GestureOutput()
{
	stopvJoy();
}

void GestureOutput::setInput(int index)
{
	if (gestureOutput.find(index) == gestureOutput.end())
	{
		return;
	}
	if (index == 0)
	{
		ResetVJD(iInterface);
		return;
	}

	ControlMapping* control = &gestureOutput.at(index);
	if (control->joy)
	{
		if (control->axis)
		{
			setAxis(control->axis, control->value);
		}
		else
		{
			pressButton(control->value);
		}
	}
	else
	{
		pressKey(control->key);
	}
}

void GestureOutput::unsetInput(int index)
{
	if (gestureOutput.find(index) == gestureOutput.end())
	{
		return;
	}
	if (index == 0)
	{
		ResetVJD(iInterface);
		return;
	}

	ControlMapping* control = &gestureOutput.at(index);
	if (control->joy)
	{
		if (control->axis)
		{
			setAxis(control->axis, 0);
		}
		else
		{
			releaseButton(control->value);
		}
	}
	else
	{
		releaseKey(control->key);
	}
}

void GestureOutput::setInputAxis(int index, float value)
{
	if (gestureOutput.find(index) == gestureOutput.end())
	{
		return;
	}

	ControlMapping* control = &gestureOutput.at(index);
	if (control->joy && control->axis && control->range)
	{
		setAxis(control->axis, (int)(control->scalar * value + control->offset));
	}
}

void GestureOutput::resetInputAxis(int index)
{
	if (gestureOutput.find(index) == gestureOutput.end())
	{
		return;
	}

	ControlMapping* control = &gestureOutput.at(index);
	if (control->joy && control->axis && control->range)
	{
		setAxis(control->axis, 0);
	}
}

void GestureOutput::setMousePos(int x, int y)
{
	INPUT input;
	MOUSEINPUT mi;
	input.type = INPUT_MOUSE;
	input.mi = mi;

	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	input.mi.time = 0;
	input.mi.dwExtraInfo = 0;
	SendInput(1, &input, sizeof(input));
}

void GestureOutput::mapKey(int index, char key)
{
	ControlMapping control;
	control.joy = 0;
	control.key = key;	
	gestureOutput[index] = control;
}

/*void GestureOutput::mapBit(int index, int bit)
{
	ControlMapping control;
	control.joy = 0;
	control.bit = bit;
	gestureOutput[index] = control;
}*/

void GestureOutput::mapButton(int index, int button)
{
	ControlMapping control;
	control.joy = iInterface;
	control.axis = 0;
	control.value = button;
	gestureOutput[index] = control;
}

void GestureOutput::mapAxisValue(int index, int axis, int value)
{
	ControlMapping control;
	control.joy = iInterface;
	control.axis = axis;
	control.value = value;
	gestureOutput[index] = control;
}

void GestureOutput::mapAxisRange(int index, int axis, float lower, float upper)
{
	ControlMapping control;
	control.joy = iInterface;
	control.axis = axis;
	control.range = 1;
	control.scalar = VJOY_AXIS_RANGE / (upper - lower);
	control.offset = -lower * control.scalar;
	gestureOutput[index] = control;
}

void GestureOutput::clearMapping(int index)
{
	gestureOutput.erase(index);
}

void GestureOutput::clearAllMappings()
{
	gestureOutput.clear();
}

void GestureOutput::setAxis(int axis, int value)
{
	SetAxis(value, iInterface, axis);
}

void GestureOutput::pressButton(int button)
{
	SetBtn(TRUE, iInterface, button);
}

inline void GestureOutput::releaseButton(int button)
{
	SetBtn(FALSE, iInterface, button);
}

void GestureOutput::pressKey(char key)
{
	KEYBDINPUT ki = { (WORD)key };
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki = ki;
	SendInput(1, &input, sizeof(input));
}

inline void GestureOutput::releaseKey(char key)
{
	KEYBDINPUT ki = { (WORD)key };
	ki.dwFlags = KEYEVENTF_KEYUP;
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki = ki;
	SendInput(1, &input, sizeof(input));
}

int GestureOutput::startvJoy()
{
	// Get the driver attributes (Vendor ID, Product ID, Version Number)
	if (!vJoyEnabled())
	{
		printf("Function vJoyEnabled Failed - make sure that vJoy is installed and enabled\n");
		return -2;
	}
	else
	{
		wprintf(L"Vendor: %s\nProduct :%s\nVersion Number:%s\n",
			static_cast<wchar_t *>(GetvJoyManufacturerString()),
			static_cast<wchar_t *>(GetvJoyProductString()),
			static_cast<wchar_t *>(GetvJoySerialNumberString()));
	}

	// Get the state of the requested device (iInterface)
	VjdStat status = GetVJDStatus(iInterface);
	switch (status)
	{
	case VJD_STAT_OWN:
		printf("vJoy Device %d is already owned by this feeder\n", iInterface);
		break;
	case VJD_STAT_FREE:
		printf("vJoy Device %d is free\n", iInterface);
		break;
	case VJD_STAT_BUSY:
		printf("vJoy Device %d is already owned by another feeder\n\
Cannot continue\n", iInterface);
		return -3;
	case VJD_STAT_MISS:
		printf("vJoy Device %d is not installed or disabled\n\
Cannot continue\n", iInterface);
		return -4;
	default:
		printf("vJoy Device %d general error\nCannot continue\n", iInterface);
		return -1;
	};

	if ((status == VJD_STAT_OWN) ||
		((status == VJD_STAT_FREE) && (!AcquireVJD(iInterface))))
	{
		printf("Failed to acquire vJoy device number %d.\n", iInterface);
		return -1;
	}
	else
		printf("Acquired: vJoy device number %d.\n", iInterface);

	// Reset this device to default values
	ResetVJD(iInterface);

	return 0;
}

void GestureOutput::stopvJoy()
{
	ResetVJD(iInterface);
	RelinquishVJD(iInterface);
}

int GestureOutput::loadSettings(std::string filename)
{
	wchar_t pwcFilename[WCHAR_BUF_SIZE];
	swprintf(pwcFilename, L"%s", (wchar_t *)filename.c_str());
	
	//look up all section names
	std::vector<int> gesture;
	{
		wchar_t pwcSectionNames[1024];
		wchar_t* pwcbuf = pwcSectionNames;
		int offset;
		int value;
		GetPrivateProfileSectionNames(pwcSectionNames, 1024, pwcFilename);
		//extract each class index from names
		while (pwcbuf < pwcSectionNames + 1024)
		{
			if (swscanf(pwcbuf, APPNAME_LABEL L"%d%n", &value, &offset))
			{
				gesture.push_back(value);
			}
			if (*(pwcbuf + offset + 1) == '\0')
			{
				break;
			}
			pwcbuf += offset + 1;
		}
	}

	gestureOutput.clear();
	wchar_t pwcAppName[WCHAR_BUF_SIZE];
	wchar_t pwcString[WCHAR_BUF_SIZE];

	//load settings
	for (int i = 0; i < gesture.size(); i++)
	{
		ControlMapping tempMap;
		swprintf(pwcAppName, APPNAME_LABEL L"%d", gesture[i]);
		tempMap.joy = GetPrivateProfileInt(pwcAppName, KEYNAME_JOY, 0, pwcFilename);
		if (tempMap.joy == 0)
		{
			//key
			tempMap.key = GetPrivateProfileInt(pwcAppName, KEYNAME_KEYBD, 0, pwcFilename);
		}
		else
		{
			//joy
			tempMap.axis = GetPrivateProfileInt(pwcAppName, KEYNAME_AXIS, 0, pwcFilename);
			if (tempMap.axis == 0)
			{
				//button
				tempMap.value = GetPrivateProfileInt(pwcAppName, KEYNAME_VALUE, 0, pwcFilename);
			}
			else
			{
				//axis
				tempMap.range = GetPrivateProfileInt(pwcAppName, KEYNAME_RANGE, 0, pwcFilename);
				if (tempMap.range == 0)
				{
					//value
					tempMap.value = GetPrivateProfileInt(pwcAppName, KEYNAME_VALUE, 0, pwcFilename);
				}
				else
				{
					//continuous
					//float values
					GetPrivateProfileString(pwcAppName, KEYNAME_SCALAR, NULL, pwcString, WCHAR_BUF_SIZE, pwcFilename);
					swscanf(pwcString, L"%f", &tempMap.scalar);
					GetPrivateProfileString(pwcAppName, KEYNAME_OFFSET, NULL, pwcString, WCHAR_BUF_SIZE, pwcFilename);
					swscanf(pwcString, L"%f", &tempMap.offset);
				}
			}
		}
		gestureOutput.emplace(gesture[i], tempMap);
	}

	return 0;
}

int GestureOutput::saveSettings(std::string filename)
{
	wchar_t pwcAppName[WCHAR_BUF_SIZE];
	wchar_t pwcString[WCHAR_BUF_SIZE];
	wchar_t pwcFilename[WCHAR_BUF_SIZE];
	swprintf(pwcFilename, L"%s", (wchar_t *)filename.c_str());

	for (auto it = gestureOutput.begin(); it != gestureOutput.end(); it++)
	{
		swprintf(pwcAppName, APPNAME_LABEL L"%d", it->first);
		//clear section entries
		if (!WritePrivateProfileString(pwcAppName, NULL, NULL, pwcFilename))
			return 1;	//write failed

		if (it->second.joy)
		{
			swprintf(pwcString, L"%d", it->second.joy);
			WritePrivateProfileString(pwcAppName, KEYNAME_JOY, pwcString, pwcFilename);
			if (it->second.axis)
			{
				//axis
				swprintf(pwcString, L"%d", it->second.axis);
				WritePrivateProfileString(pwcAppName, KEYNAME_AXIS, pwcString, pwcFilename);
				if (it->second.range)
				{
					//continuous
					WritePrivateProfileString(pwcAppName, KEYNAME_RANGE, L"1", pwcFilename);
					//float values
					swprintf(pwcString, L"%f", it->second.scalar);
					WritePrivateProfileString(pwcAppName, KEYNAME_SCALAR, pwcString, pwcFilename);
					swprintf(pwcString, L"%f", it->second.offset);
					WritePrivateProfileString(pwcAppName, KEYNAME_OFFSET, pwcString, pwcFilename);
				}
				else
				{
					//value
					swprintf(pwcString, L"%d", it->second.value);
					WritePrivateProfileString(pwcAppName, KEYNAME_VALUE, pwcString, pwcFilename);
				}
			}
			else
			{
				//button
				swprintf(pwcString, L"%d", it->second.value);
				WritePrivateProfileString(pwcAppName, KEYNAME_VALUE, pwcString, pwcFilename);
			}
		}
		else
		{
			//keyboard
			swprintf(pwcString, L"%d", it->second.key);
			WritePrivateProfileString(pwcAppName, KEYNAME_KEYBD, pwcString, pwcFilename);
		}
	}

	return 0;
}

