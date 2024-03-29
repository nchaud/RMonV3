#ifndef __SENSORMANAGER_H__
#define __SENSORMANAGER_H__

#include <Arduino.h>
#include "DataTypes.h"

class SensorManager
{
private:
	boolean _isMock;
	uint8_t __mockDataCounter;
	
	float readVcc();
	void getMockData(SensorData* ret);

public:
	SensorManager(boolean isMock);
	void readData(SensorData* data);
	uint16_t takeSampleAnalog(uint8_t pinNo);

};

#endif //__SENSORMANAGER_H__
