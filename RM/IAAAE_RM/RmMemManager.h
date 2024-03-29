#ifndef __RMMEMMANAGER_H__
#define __RMMEMMANAGER_H__

#include "DataTypes.h"

class RmMemManager
{
//variables
public:
	SensorData* mockSensorData;
	uint8_t numMockSensorData;
private:
	boolean _isMock;

	//Their current state - true=>high
	boolean _ledBottomPinRed=false;
	boolean _ledBottomPinGreen=false;
	boolean _ledTopPinRed=false;
	boolean _ledTopPinGreen=false;
	LED_STATE _ledBottomState = All_Clear;
	LED_STATE _ledTopState = All_Clear;
	uint8_t _flashCallCount=0;

//functions
public:
	RmMemManager(boolean isMock);
	
	uint8_t getUCharFromMemory(uint16_t address);
	uint16_t getUShortFromMemory(uint16_t address);
	uint32_t getUIntFromMemory(uint16_t address);
	uint64_t getULongFromMemory(uint16_t address);
	void setUCharToMemory(uint16_t address, uint8_t value);
	void setUShortToMemory(uint16_t address, uint16_t value);
	void setUIntToMemory(uint16_t address, uint32_t value);
	void setULongToMemory(uint16_t address, uint64_t value);
	
	void initialiseModule(uint8_t moduleId);
	
	uint16_t getBootCount();
	uint8_t getModuleId();
	uint16_t incrementBootCount();
	
	uint16_t verifyBasicEepRom();
	
	void runExtendedDumpOutput();
	void runExtendedShow100Bytes();
	
	/* Returns the number of readings read */
	uint8_t loadSensorData(SensorData* buffer, uint8_t maxNoOfReadings);
									//unsigned long* loadedUpTo); //byte* outputData, unsigned int outputDataMaxLength)

	void appendSensorEntry(SensorData* r);
	void appendDailyEntry(DailyCycleData* r);
	
	//Called at regular intervals at a fast-rate to toggle LEDs between off-on
	void flashLED();
	//Request to change the state of an LED
	void toggleLED(LED_SEL led_num, LED_STATE state);
	void reset();
	
private:
	void incrementCycleCount();
	uint16_t dataStartAddress();
	uint16_t cycleAddressIfRequired(uint16_t currFreeAddr, uint8_t dataSz);
}; //RmMemManager

#endif //__RMMEMMANAGER_H__
