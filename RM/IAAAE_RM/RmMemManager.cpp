#include <Arduino.h>
#include <Wire.h>
#include "DataTypes.h"
#include "RmMemManager.h"


//Memory positions of static metadata
#define MEMLOC_START 0
//#define MEMLOC_MODULE_ID 0
//#define MEMLOC_VERSION	 4
//#define MEMLOC_BOOTCOUNT 8 /* Long - 4 bytes */
//#define MEMLOC_SENT_UPTO 12 /* The last Reading NUMBER (not idx) that was successfully sent out */

//The start and end positions in EEPROM of circular buffer reserved for Readings
//#define BAND_READING_START 100 /* Sizeof module data above + spare */
//The number of entries stored so far. 0=> no entries. -1 to get the idx of an entry.
#define MEMLOC_READING_ENTRY_COUNT 100 /* Long - 4 bytes */
#define MEMADDR_READING_DATA_START  104 /* Long - 4 bytes */
#define MEMADDR_READING_END 30L*1000 /* ROM is 32K  - but do we want to cycle back and lose that important historical data? Circular should be limited to 1K @ end?*/
//#define BAND_READING_CYCLE_START 28*1000 /* Will start cycling from here after it reaches the _END memory location above */


//Their current state - true=>high
boolean _ledBottomPinRed=false;
boolean _ledBottomPinGreen=false;
boolean _ledTopPinRed=false;
boolean _ledTopPinGreen=false;
LED_STATE _ledBottomState = All_Clear;
LED_STATE _ledTopState = All_Clear;
uint8_t _flashCallCount=0;

RmMemManager::RmMemManager(boolean isMock){
	_isMock = isMock;
	
	if (!_isMock)
		Wire.begin();
}

void internalWrite(int16_t address, uint8_t* data, uint8_t numBytes){
	
	RM_LOGMEM(F("Writing memory at address "));
	RM_LOGMEM(address);
	//RM_LOGMEM(F(" from address "));
	//RM_LOGMEMnt((uint8_t)data); //TODO
	RM_LOGMEM(F(" of size "));
	RM_LOGMEMLN(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
	
		uint16_t thisByteAddr = address+i;
	
		RM_LOGMEM(F("Writing byte at address "));
		RM_LOGMEM(thisByteAddr);
	
		RM_LOGMEM(F("...with MSB "));
		RM_LOGMEM((int) (thisByteAddr >> 8));
		RM_LOGMEM(F(" and LSB "));
		RM_LOGMEM((int) (thisByteAddr & 0xFF));
		RM_LOGMEM(F(" : "));
		RM_LOGMEMLN(*(data+i));
	
		Wire.beginTransmission(0x50);
		Wire.write((int) (thisByteAddr >> 8)); // msb
		Wire.write((int) (thisByteAddr & 0xFF)); // lsb
		Wire.write(*(data+i)); //go byte by byte
		Wire.endTransmission();
	
		//TODO: CHECK RETURN VALUE IN END-TRANSMISSION
	
		delay(15); //Spec says 5 but that causes intermittent random reads at higher temperatures
	}
}

void internalRead(int16_t address, uint8_t* data, uint8_t numBytes){
	
	RM_LOGMEM(F("Reading memory at address "));
	RM_LOGMEM(address);
	//RM_LOGMEM(F(" to address "));
	//RM_LOGMEM((uint8_t)data);//TODO
	RM_LOGMEM(F(" of size "));
	RM_LOGMEMLN(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
		
		uint16_t thisByteAddr = address+i;
		
		RM_LOGMEM(F("Reading byte at address "));
		RM_LOGMEM(thisByteAddr);
		
		RM_LOGMEM(F("...with MSB "));
		RM_LOGMEM((int) (thisByteAddr >> 8));
		RM_LOGMEM(F(" and LSB "));
		RM_LOGMEM((int) (thisByteAddr & 0xFF));
		RM_LOGMEM(F(" : "));
		
		Wire.beginTransmission(0x50);
		Wire.write((int) (thisByteAddr >> 8)); // msb
		Wire.write((int) (thisByteAddr & 0xFF)); // lsb
		Wire.endTransmission();
		
		
		//TODO: CHECK RETURN VALUE IN END-TRANSMISSION
		
		
		Wire.requestFrom(0x50,1); //Todo: can specify multiple?
		
		uint8_t readByte=0xFF;
		if (Wire.available())
			readByte = Wire.read();
		else
			RM_LOGMEMLN("WIRE NOT AVAILABLE"); //TODO: error code
		
		RM_LOGMEM(F("Raw byte read:"));
		RM_LOGMEMLN(readByte);
		
		*(data+i) = readByte;
	}
}

uint8_t RmMemManager::getUCharFromMemory(uint16_t address)
{
	uint8_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint16_t RmMemManager::getUShortFromMemory(uint16_t address)
{
	uint16_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint32_t RmMemManager::getUIntFromMemory(uint16_t address)
{
	uint32_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint64_t RmMemManager::getULongFromMemory(uint16_t address)
{
	uint64_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

void RmMemManager::setUCharToMemory(uint16_t address, uint8_t value)
{
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setUShortToMemory(uint16_t address, uint16_t value)
{
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setUIntToMemory(uint16_t address, uint32_t value)
{
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setULongToMemory(uint16_t address, uint64_t value)
{
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::initialiseModule(uint8_t moduleId){

	ModuleMeta meta;
	meta.moduleId = moduleId;
	meta.bootCount = 0;
	meta.eepromTestArea = 0;
	meta.nextFreeWriteAddr = MEMLOC_START + sizeof(ModuleMeta);
	memset(meta.spareBuffer, 0, sizeof(meta.spareBuffer));
	
	//TODO: Blank out rest of eeprom too?
	
	internalWrite(MEMLOC_START, (uint8_t*)&meta, sizeof(ModuleMeta));
}

uint16_t RmMemManager::incrementBootCount(){
	
	//TODO: Somehow verify Wire is working?
	uint16_t addr = MEMLOC_START + offsetof(ModuleMeta, bootCount);
	uint16_t currVal = getUShortFromMemory(addr);
	++currVal;
	setUShortToMemory(addr, currVal);
	return currVal;
}

uint16_t RmMemManager::verifyBasicEepRom(){
	
	//TODO: Verify of spill over 64-bit boundary what to do
	//TODO: Roll-over verification
	//TODO: Don't have strings of messages, return code which translate in a #if NOT_ENCODED_ON_DEVICE block
	
	uint16_t MEM_TEST_LOC = offsetof(ModuleMeta, eepromTestArea);
	
	//Repeat test multiple times to check for intermittent possible errors
	uint16_t numFailures;
	
	for(uint8_t i=0;i<30;i++)
	{
		//No intersection of values so we'll know straight away if wrong bits picked
		this->setULongToMemory(MEM_TEST_LOC, 0xA946F7D8C941F9A8);
		uint64_t val8 = this->getULongFromMemory(MEM_TEST_LOC);
		if (val8 != 0xA946F7D8C941F9A8)
		{
			RM_LOGLN("FAILED LONG");
			++numFailures;
		}
		
		this->setUIntToMemory(MEM_TEST_LOC, 0xC911F948);
		uint32_t val4 = this->getUIntFromMemory(MEM_TEST_LOC);
		if (val4 != 0xC911F948)
		{
			RM_LOGLN("FAILED INT");
			++numFailures;
		}
		
		this->setUShortToMemory(MEM_TEST_LOC, 0x5C3A);
		uint16_t val2 = this->getUShortFromMemory(MEM_TEST_LOC);
		if (val2 != 0x5C3A)
		{
			RM_LOGLN("FAILED SHORT");
			++numFailures;
		}
		
		this->setUCharToMemory(MEM_TEST_LOC, 0xE1);
		uint8_t val1 = this->getUCharFromMemory(MEM_TEST_LOC);
		if (val1 != 0xE1)
		{
			RM_LOGLN("FAILED CHAR");
			++numFailures;
		}
		
		RM_LOG(F("EEPROM Test: Long="));
		RM_LOGLONG(val8, HEX);
		RM_LOG(F(", Int="));
		RM_LOGFMT(val4, HEX);
		RM_LOG(F(", Short="));
		RM_LOGFMT(val2, HEX);
		RM_LOG(F(", Char="));
		RM_LOGLNFMT(val1, HEX);
	}
	
	return numFailures;
}

//TODO: Only in PC_BEHAVIOUR 
void RmMemManager::printData(){

	//Get last reading
	ModuleMeta meta;
	internalRead(MEMLOC_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	
	RM_LOG2(F("Module #"), meta.moduleId);
	RM_LOG2(F("# Boots"), meta.bootCount);
	RM_LOG2(F("Next Addr"), meta.nextFreeWriteAddr);
	//
	//for(uint8_t i=0;i<meta.numReadings;i++){
	//
	//RM_LOG(F("Reading #"));
	//RM_LOGLN(i);
	//
	//uint16_t readingAddr = getReadingAddress(i);
	//SingleSession session;
	//readMem(readingAddr, (uint8_t*)&session, sizeof(SingleSession));
	//
	//RM_LOG(F("Gsm-Status: "));
	//RM_LOG(session.gsmInfo.networkStatus);
	//RM_LOG(F(", Gsm-RSSI: "));
	//RM_LOG(session.gsmInfo.rssi);
	//RM_LOG(F(", Gsm-Error Code: "));
	//RM_LOG(session.gsmInfo.errorCode);
	//
	//RM_LOG(F(", Gps-Status: "));
	//RM_LOG(session.gpsInfo.gpsStatus);
	//RM_LOG(F(", Gps-Error Code: "));
	//RM_LOG(session.gpsInfo.errorCode);
	//RM_LOG(F(", Gps-Lat: "));
	//RM_LOG(session.gpsInfo.lat);
	//RM_LOG(F(", Gps-Lon: "));
	//RM_LOG(session.gpsInfo.lon);
	//RM_LOG(F(", Gps-Date: "));
	//RM_LOG(session.gpsInfo.date);
	//RM_LOG(F(", Gps-Heading: "));
	//RM_LOG(session.gpsInfo.heading);
	//RM_LOG(F(", Gps-Speed: "));
	//RM_LOGLN(session.gpsInfo.speed_kph);
	//}
}

/* Returns the number of readings read */
unsigned long RmMemManager::loadSensorData(SensorData* buffer, unsigned int maxNoOfReadings,
										  unsigned long* loadedUpTo)
{
	return 1;
	
	//uint8_t readingSz = sizeof(SensorData);
	//
	////TODO: This only takes the last few, change to average/compress?
	//
	//volatile unsigned long entryCount = getLongFromMemory(MEMLOC_READING_ENTRY_COUNT);
	//volatile unsigned long alreadySentTo = getLongFromMemory(MEMLOC_SENT_UPTO);
	//
	////We need the last {<maxNoOfReadings}. This may mean we skip from {alreadySentTo-x} onwards.
	//volatile unsigned long numOfLastReadings = min(entryCount-alreadySentTo, maxNoOfReadings); //Take last n readings
	//if (numOfLastReadings == 0)
	//{
		//*loadedUpTo = alreadySentTo; /* Nothing more */
		//return 0UL; //Nothing to send
	//}
	//
	////Get read idx => 2 readings if 10 entry count => @ idx 8 offset
	//volatile unsigned int startReadOffset = (entryCount-numOfLastReadings) * readingSz;
	//volatile unsigned int startReadAddress = MEMADDR_READING_DATA_START + startReadOffset;
//
	//byte* rPtr = (byte*)buffer;//&r2;
	//
	//for(volatile int readingNo = 0; readingNo < numOfLastReadings; readingNo++)
		//for(volatile int byteIdx=0;byteIdx<readingSz;byteIdx++)
		//{
			//unsigned int currBufferOffset = (readingNo*readingSz)+byteIdx;//curr buffer offset
			//unsigned int currReadAddress = startReadAddress + currBufferOffset;
			//unsigned volatile int stopme=currBufferOffset;
			//unsigned volatile int stopme2=currReadAddress;
			//volatile byte stopx = (byte)EEPROM.read(currReadAddress);
			//*(rPtr+currBufferOffset) = EEPROM.read(currReadAddress);
		//}
	//
	//
	//*loadedUpTo = entryCount; //Gets passed to markDataSent() if sent successfully
	//return numOfLastReadings;
}

void RmMemManager::markDataSent(uint64_t sentUpTo)
{
	//this->setULongToMemory(MEMLOC_SENT_UPTO, sentUpTo);
}

void RmMemManager::appendDailyEntry(DailyCycleData* r)
{
	//TODO
}

void internalWriteEntryAtAddress(SensorData* r, unsigned long address){
	
	//byte* rPtr = (byte*)r;
//
	//for(int i=0;i<sizeof(SensorData);i++)
		//EEPROM.write(address+i, *(rPtr+i));
}

void RmMemManager::replaceLastSensorEntry(SensorData* r)
{
	//Read where last entry is
	//volatile unsigned long entryCount = this->getULongFromMemory(MEMLOC_READING_ENTRY_COUNT);
	//volatile unsigned long lastEntryOffset = max(0,entryCount-1) * sizeof(SensorData);
	//volatile unsigned long lastEntryAddress = MEMADDR_READING_DATA_START + lastEntryOffset;
	//
	//internalWriteEntryAtAddress(r, lastEntryAddress);
}

void RmMemManager::appendSensorEntry(SensorData* r)
{
	//volatile unsigned int readingSz = sizeof(SensorData); //const
	//
	////Read where next entry is free
	//volatile unsigned long entryCount = this->getULongFromMemory(MEMLOC_READING_ENTRY_COUNT);
	//volatile unsigned long nextFreeOffset = entryCount * sizeof(SensorData);
	//volatile unsigned long nextFreeAddress = MEMADDR_READING_DATA_START + nextFreeOffset;

	//TODO: modulo both free address (AND entry count?)

	//internalWriteEntryAtAddress(r, nextFreeAddress);

	//Update entry count
	//this->setULongToMemory(MEMLOC_READING_ENTRY_COUNT, entryCount+1);
	
	/* Can we do this even with writing page limitations ? */
	
	//If it goes beyond the end, restart from beginning
	//uint32_t structureSize = sizeof(Reading);
	//if (nextFreeMemAddr+structureSize > BAND_READING_END)
	//	nextFreeMemAddr = BAND_READING_START;
	
	//unsigned int nextFreeAddr = writeEEPROM(nextFreeMemAddr, (byte*)&r, sizeof(Reading));
	
	//IF success, update the location at which to write the next Reading
	//writeEEPROM(MEMLOC_READING_DATA_START, (byte*)&nextFreeAddr, 2);
}






/* LED mgmt - Not strictly memory related */

void RmMemManager::reset(){
	_flashCallCount=0;
	toggleLED(Bottom, All_Clear);
	toggleLED(Top, All_Clear);
}

/* Flashes for a single LED */
void internalFlash(
	boolean& greenPinVal, boolean& redPinVal,
	LED_STATE currLedState, boolean atSlowInterval
	)
{
	//Do green LED first
	//boolean nextState = greenPinVal;//-1=no action, 0=false, 1=true
	
	if (currLedState == Green_Fast) {
		greenPinVal = !greenPinVal;// currPinValue==-1?1:!currLedState; //toggle
	}
	else if (currLedState == Green_Slow) {
		greenPinVal = atSlowInterval;//!greenPinVal;// currPinValue==-1?1:!currPinValue; //toggle
	}
	else if (currLedState == Green_Solid) {
		greenPinVal = true;// currPinValue==1?-1:1; //Must be solid if not already
	}
	else if (currLedState == All_Clear) {
		greenPinVal = false;//currPinValue==0?-1:0; //Must be clear if not already
	}
	
	//newGreenPinVal = nextState;
	//if (nextState != -1 && nextState != greenPinVal)
	//	digitalWrite(greenPinNo, nextState);	
		
	//Do red LED next
	//nextState = redPinVal;// -1;//-1=no action, 0=false, 1=true
	
	if (currLedState == Red_Fast) {
		redPinVal = !redPinVal;// currPinValue==-1?1:!currLedState; //toggle
	}
	else if (currLedState == Red_Slow) {
		redPinVal = atSlowInterval;//!greenPinVal;// currPinValue==-1?1:!currPinValue; //toggle
	}
	else if (currLedState == Red_Solid) {
		redPinVal = true;// currPinValue==1?-1:1; //Must be solid if not already
	}
	else if (currLedState == All_Clear) {
		redPinVal = false;//currPinValue==0?-1:0; //Must be clear if not already
	}
	
	//newRedPinVal &= redPinVal;
	//if (nextState != -1 && nextState != redPinVal)
	//	digitalWrite(redPinNo, nextState);
}


//Called at regular intervals at a fast-rate to toggle LEDs between off-on
void RmMemManager::flashLED()
{
	//Every 3 flashes, do a slow blink
	_flashCallCount = ++_flashCallCount%3;

	//Flash Bottom LED	
	internalFlash(
				_ledBottomPinRed,_ledBottomPinGreen, _ledBottomState,
				_flashCallCount==0
				);
	digitalWrite(PIN_LED_BOTTOM_GREEN, _ledBottomPinGreen);
	digitalWrite(PIN_LED_BOTTOM_RED, _ledBottomPinRed);
				
	//Flash Top LED
	internalFlash(
				_ledTopPinGreen, _ledTopPinRed, _ledTopState,
				_flashCallCount==0
				);
	digitalWrite(PIN_LED_TOP_GREEN, _ledTopPinGreen);
	digitalWrite(PIN_LED_TOP_RED, _ledTopPinRed);
}

//Request to change the state of an LED
void RmMemManager::toggleLED(LED_SEL led_num, LED_STATE state)
{
	if (led_num == Bottom)
		_ledBottomState = state; //(?) / Clearing? / (state&=IsTemporary)==1?
	else if (led_num == Top)
		_ledTopState = state;

	//TODO: prioritise instead?
	//TODO: Could have a hierarchy where if solid is cleared, maybe slow-flashing
	//still required?
}
