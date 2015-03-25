#pragma once


class FlashSimpleDrv
{
public:
	static FlashSimpleDrv* getInstance();
	int init(unsigned long sensorDev);
	int setOnOff(int a_isOn);
	int uninit();
	int setPreOn();
	int getPreOnTimeMs(int* ms);;
private:
	int m_fdSTROBE;
	FlashSimpleDrv();
	virtual ~FlashSimpleDrv();
	int m_preOnTime;

};