/*
// TODO: Do below APN settings !!
Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
// TODO: Keep on for charging battery at times
*/

#include <Arduino.h>
#include "Adafruit_FONA.h"
#include "DataTypes.h"
#include "Helpers.h"
#include "Timing.h"
//#include "GsmManager.h"
//#include "GpsManager.h"
#include "RmMemManager.h"
#include "SensorManager.h"
#include "ExtendedTests.h"


//C++ instances
Adafruit_FONA __fona = Adafruit_FONA(FONA_RST, IS_GSM_MOCK);
RmMemManager mem = RmMemManager(false);
//GpsManager gps = GpsManager(IS_GPS_MOCK);
SensorManager sensorMgr = SensorManager(true);

void switchOffSystem();
void on3MinutesElapsed(bool doWrite);
void printData();
Adafruit_FONA* ensureFonaInitialised(boolean forDataSend, boolean* isPending);

uint8_t _behaviour = SYS_BEHAVIOUR::DoNothing;

void setup() {
	
	//unsigned char ResetSrc = MCUSR;   // TODO: save reset reason if not 0
	//unsigned volatile int px=9;
	//MCUSR = 0x00;  // cleared for next reset detection

	// Optionally configure a GPRS APN, username, and password.
	// You might need to do this to access your network's GPRS/data
	// network.  Contact your provider for the exact APN, username,
	// and password values.  Username and password are optional and
	// can be removed, but APN is required.
	//fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));

	// Optionally configure HTTP gets to follow redirects over SSL.
	// Default is not to follow SSL redirects, however if you uncomment
	// the following line then redirects over SSL will be followed.
	//fona.setHTTPSRedirect(true);


	//Must immediately run as this pin in LOW switches off the system
	pinMode(PIN_SHUTDOWN, OUTPUT);
	digitalWrite(PIN_SHUTDOWN, HIGH);



	delay(3000); //time for above to happen + hardware peripherals to warm up + for user's serial monitor to connect
	
	
	
	//Turn off redundant Arduino board notification LED controlled by pin 13
	pinMode(13, OUTPUT);
	
	#ifdef OUTPUT_DEBUG
		Serial.begin(9600); //Writes to Serial output
	#endif
	
	RM_LOGLN(F("Starting..."));
		
	if (IS_BASIC_MEM_TEST) {
		
		mem.verifyBasicEepRom();
		
		switchOffSystem();
		return;
	}

	if (IS_EXTENDED_TYPES_TEST) {
	
		ExtendedTests::runExtendedTypesTest();
	
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_MEM_TEST) {
		
		ExtendedTests::runExtendedMemTest(mem, sensorMgr);
		
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_SHOW_100_BYTES) {
		
		mem.runExtendedShow100Bytes();
		
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_SHOW_MEM) {
		
		mem.runExtendedDumpOutput();
		
		switchOffSystem();
		return;
	}

	if (IS_EXTENDED_GSM_TEST) {
	
		_behaviour |= SYS_BEHAVIOUR::ExtendedGsmTest;
		return;
	}

	if (INITIALISE_MODULE_ID) {
		
		mem.initialiseModule(INITIALISE_MODULE_ID);
		
		switchOffSystem();
		return;
	}
		
	uint16_t currBootCount = mem.incrementBootCount();
	RM_LOG2(F("Boot Count"), currBootCount);
	
	//Take reading every few hours - not a factor of 24 - so it's a scattered time reading throughout the week
	_behaviour |= SYS_BEHAVIOUR::TakeReadings;
	
	//Send to HQ every ~20 hours
	if (currBootCount > 0 && currBootCount%4 == 0) { //TODO: Overflow?
		
		_behaviour |= SYS_BEHAVIOUR::SendData;
	}
}

uint8_t _fonaStatusInit=0;
uint8_t _gprsStatusInit=0;
FONA_GET_RSSI _rssiStatusInit;
uint16_t _initFonaLoopCount = 0;
uint16_t _gprsSignalLoopCount = 0;
Adafruit_FONA* ensureFonaInitialised(boolean forDataSend, boolean* isComplete) {

	//boolean isFirstLoop = _initFonaLoopCount == 0;
	++_initFonaLoopCount;
	
	*isComplete = true;
	
	if (_fonaStatusInit==0) {
		
		RM_LOGLN(F("Initialising fona..."));
		
		FONA_STATUS_INIT ret = __fona.begin(FONA_TX, FONA_RX);
		_fonaStatusInit = ret;
		
		//TODO: TEST ! with both single-digit module IDs and double digit
		uint8_t moduleId = mem.getModuleId();
		String userAgentStr = "IAAAE_RMonV3_";
		userAgentStr += moduleId;
		__fona.setUserAgent(userAgentStr);
	}

	if (IS_ERR_FSI(_fonaStatusInit)) {
	
		RM_LOG2(F("Error initialising fona..."), _fonaStatusInit);
	
		//FONA library did not begin - store err in ROM, terminate and don't consume power
		//(TODO + Why would this ever happen?)
		//But don't log it in eeprom if running a test? Basic=non-writing test vs Extended tests?
	
		return NULL;
	}	

	
	if (forDataSend) {
		
		//The first enable attempt will happen after GPRS_ENABLE_INTERVAL
		
		if (_gprsStatusInit != 0) {
			
			//No-op, just return what was calculated the last time
		}
		else if (_initFonaLoopCount % GPRS_ENABLE_INTERVAL != 0) {
			
			//Try to enable it every x seconds for a period
			*isComplete = false;
		}
		else {
			
			RM_LOGLN(F("Attempting to enable GPRS..."));
		
			FONA_STATUS_GPRS_INIT gprsRet = __fona.enableGPRS(true);
		
			if (IS_ERR_FSGI(gprsRet)) {
			
				//TODO: Log this
				//But don't log it in eeprom if running a test? Basic=non-writing test vs Extended tests?
			
				RM_LOG2(F("Error initialising GPRS"), gprsRet);
				if (_initFonaLoopCount < GPRS_MAX_ENABLE_TIME) {
				
					*isComplete = false;
					RM_LOGLN(F("Will try to enable GPRS again shortly"));
				}
				else {
				
					//We've hit the last interval of trying, so all done trying
					_gprsStatusInit = gprsRet;
					RM_LOGLN(F("All attempts to enable GPRS failed"));
				}
			}
			else {
			
				//Success, we're done initialising GPRS
				_gprsStatusInit = gprsRet;
				RM_LOGLN(F("GPRS initialised successfully !"));
			}
		}
		
		if (_gprsStatusInit != 0) {
			if (IS_ERR_FSGI(_gprsStatusInit)) {
				return NULL;
			}
			else { //Initialised successfully, now ensure good signal
				
				++_gprsSignalLoopCount;
				
				if (Helpers::isSignalGood(&_rssiStatusInit)) {
					
					//Previously checked - it's fine
				}
				else if (_gprsSignalLoopCount % GPRS_SIGNAL_CHECK_INTERVAL != 0) {
			
					//Try to enable it every x seconds for a period
					*isComplete = false;
				}
				else {

					RM_LOG(F("Checking RSSI - currently:"));
					FONA_GET_RSSI rssi = __fona.getRSSI();
					Helpers::printRSSI(&rssi);					
					
					if (!Helpers::isSignalGood(&rssi)) {
						
						if (_gprsSignalLoopCount < GPRS_MAX_SIGNAL_WAIT_TIME) {
							
							*isComplete = false;
							RM_LOGLN(F("\t (Bad-RSSI - will check again after interval)"));
						}						
						else {
							
							//Wait-time over for signal, continue regardless of signal value, it may work
							RM_LOGLN(F("\t (Waiting For Good-RSSI Timed Out - will continue now)"));
							_rssiStatusInit = rssi;
						}
					}
					else {
						
						//All done, signal is good now
						RM_LOGLN(F("\t (Good-RSSI - successfull, all done)"));
						_rssiStatusInit = rssi;
					}
				}
				
				//Even if we have a bad signal at timeout,
				//	we let the code flow to the end and return __fona so they can try sending data anyway
			}
		}
		else {
			return NULL;
		}
	}
	
	return &__fona;
}


void switchOffSystem() {
	
	RM_LOGLN("Switching off...");
	
	digitalWrite(PIN_SHUTDOWN, LOW);
	
	delay(3000); //To allow serial to purge the shutdown message
}

void on3MinutesElapsed(bool doWrite) {

	//RM_LOGLN(F("3 minutes elapsed - logging..."));
	//
	//byte META_SZ = sizeof(ModuleMeta);
	//byte SESSION_SZ = sizeof(SingleSession);
	//
	////Get last reading
	//ModuleMeta meta;
	//readMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	//
	//RM_LOG(F("Module #"));
	//RM_LOG(meta.moduleId);
	//RM_LOG(F(", Current #Readings: "));
	//RM_LOGLN(meta.numReadings);
//
	//if (!doWrite)
		//return;
//
	////Update the number of readings in metadata first so no matter what happens, existing data isnt overwritten
	//meta.numReadings++;
	//writeMem(TART, (uint8_t*)&meta, sizeof(ModuleMeta));
	//
	//SingleSession session;
	//gps.getGpsInfo(session.gpsInfo);
	//gsm.getGsmInfo(session.gsmInfo);
//
	//
	//RM_LOG(F("Got GPS info, lat="));
	//RM_LOG(session.gpsInfo.lat);
	//RM_LOG(F(" lon="));
	//RM_LOG(session.gpsInfo.lon);
	//RM_LOG(F(" date="));
	//RM_LOG(session.gpsInfo.date);
	//RM_LOG(F(" status="));
	//RM_LOG(session.gpsInfo.gpsStatus);
	//RM_LOG(F(" errCode="));
	//RM_LOGLN(session.gpsInfo.errorCode);
	//
	//uint16_t writeAddress = getReadingAddress(meta.numReadings);
////	Serial.print(F("Calculated next address for data to be written to: "));
////	Serial.println(writeAddress);
	//
	////Run 2 tests
	////gsm.setGPRSNetworkSettings
	//String sm = "";//"Module ID:"+ModuleMeta.moduleId+" transmitting.";
	//
	////gsm.sendViaSms(sm.c_str()); //TO: local number !
	////gsm.sendViaGprs(sm.c_str());
	//
	//writeMem(writeAddress, (uint8_t*)&session, SESSION_SZ);
}

boolean takeReadings() {
	
	RM_LOGLN(F("Taking readings..."));
	
	SensorData sd;
	sensorMgr.readData(&sd);
	
	return true;
}

void createEncodedData(Adafruit_FONA* fona, char* encodedOutput, uint8_t maxReadings, DailyCycleData* cycleData) {
	
	//This will likely be peak of stack usage so warn if low memory !
	int8_t freeRAM = Helpers::freeMemory();
	int8_t minRAM = (sizeof(SensorData)*maxReadings)
					+sizeof(FONA_GET_RSSI)
					+sizeof(GsmPayload)
					+100; //Buffer
					
	if (freeRAM < minRAM)
		RM_LOG2(F("**** Too little RAM before payload creation ***"), freeRAM);
	
	FONA_GET_RSSI rssi = fona->getRSSI();
	
	SensorData sData[maxReadings];
	uint8_t numLoaded = mem.loadSensorData((SensorData*)&sData, maxReadings);//, countToSend, &loadedTo);
	
	GsmPayload payload;
	payload.setModuleId(999);
	payload.setBootNumber(33);
	payload.setSensorData((SensorData*)&sData, numLoaded);
	payload.setRSSI(rssi);
	payload.createEncodedPayload(encodedOutput);
	
	cycleData->BootNo = payload.getBootNumber();
	cycleData->NoOfReadings = numLoaded;
	cycleData->RSSI = rssi;
	
	uint16_t battPct;
	if (!fona->getBattPercent(&battPct))
		cycleData->BattPct = -1;
	else
		cycleData->BattPct = battPct;
}

uint16_t _sendDataLoopCount = 0;
boolean sendData() {
	
	//Increment before doing any work so doesn't get stuck continuously initialising
	//(by being called from 'loop') due to a loop-resetting error raised by FONA
	++_sendDataLoopCount;
	
	if (_sendDataLoopCount == 1)
		RM_LOGLN(F("Initialising Fona to send data"));
	
	boolean isComplete;
	Adafruit_FONA* sendDataFona = ensureFonaInitialised(true, &isComplete);
	
	if (!isComplete) {
		RM_LOGLN(F("\t(Fona Init Pending...)"));
		return false; //Still waiting to initialise
	}
		
	if (sendDataFona == NULL) {
		RM_LOGLN(F("\t(Fona Init ERROR)"));
		
		
		DailyCycleData errorSendData;
		errorSendData.GPRSToggleFailure = true;
		//Save to ROM
		
		
		
		return true; //Error initialising
	}
	
	//TODO: Max number of readings to send vs when eeprom rolls over and start from beginning
		 
	uint16_t encodedSz = GsmPayload::getEncodedPayloadSize_S(GPRS_MAX_READINGS_FOR_SEND);
	char encodedData[encodedSz];
		
	//Encode in another method to free up RAM on return for the sending (just in case)
	DailyCycleData sendData;
	createEncodedData(sendDataFona, encodedData, GPRS_MAX_READINGS_FOR_SEND, &sendData);

	RM_LOGLN(F("Encoded data created and ready for send:"));
	RM_LOGLN(encodedData);

	uint16_t statuscode=0;
	FONA_STATUS_GPRS_SEND status = 
		sendDataFona->sendDataOverGprs((uint8_t*)encodedData, encodedSz, &statuscode);
	
	sendData.SendStatus = status;
	sendData.HTMLStatusCode = statuscode;

	//If all done - reset (even though board will be reset - but for tests)
	//_sendDataLoopCount = 0;
		
	
		
		
	//TODO: Save send-status
		
		
		
		
	return true;
}

//Loop-scoped variables
uint16_t _timerCounter = 0;
void loop() {

	delay(1000);
	++_timerCounter;

	RM_LOG2(F("Behaviour"), _behaviour);
	
	if((_behaviour&SYS_BEHAVIOUR::TakeReadings) != 0) {
		
		if (takeReadings())
			_behaviour &= ~SYS_BEHAVIOUR::TakeReadings;
	}
	
	if ((_behaviour&SYS_BEHAVIOUR::SendData) != 0) {
		
		if (sendData())
			_behaviour &= ~SYS_BEHAVIOUR::SendData;
	}

	if ((_behaviour&SYS_BEHAVIOUR::ExtendedGsmTest) != 0) {
	
		if (_timerCounter == 1)
			ExtendedTests::startExtendedGsmTest(&__fona, &mem);
	
		if (sendData()) {
			
			_behaviour &= ~SYS_BEHAVIOUR::ExtendedGsmTest;
			ExtendedTests::endExtendedGsmTest();
		}
	}

	if (_behaviour == SYS_BEHAVIOUR::DoNothing) {
		
		switchOffSystem();
	}
	
	//if (DIAGNOSTIC_TEST) {
	//
		////Write and print every second
		//on3MinutesElapsed(true);
		//printData();
		//return;
	//}

}




//TODO: A good test is putting
//char forWeb[1000]; 
//in the stack, causing a restart and we should get a restart code
//=> to be stored in memory and then module shutdown?






/************************************************************************/
/*             Timer/power-on testing at intervals                      */
/************************************************************************/
/*
void setup() {

	//Must immediately run - reset pin for timer
	pinMode(5, OUTPUT);
	digitalWrite(5, HIGH);

	pinMode(4, OUTPUT); //LED

	pinMode(9, OUTPUT); //buzzer
	
}

int counter = 0;

void loop() {

	if (counter <= 3)
	{
		tone(9, 1000);
		delay(1000);
		noTone(9);
	}

	digitalWrite(4, HIGH);
	delay(1000);
	digitalWrite(4, LOW);

	if (counter==5)
		digitalWrite(5, LOW);

	delay(1000);
	++counter;
}

*/
