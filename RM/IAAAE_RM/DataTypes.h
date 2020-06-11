#ifndef __DATATYPES_H__
#define __DATATYPES_H__  


//Configuration
#define OUTPUT_DEBUG		//For logging to Serial
//#define OUTPUT_DEBUG_MEM	//For logging detailed EEPROM messages to Serial
#define DEBUG				//Prints output stmts - ONLY WITH LAPTOP - to print output statements whilst all below are running
#ifdef DEBUG
	//Set FONA module to be debug too
	#define ADAFRUIT_FONA_DEBUG
#endif

//Testing - TODO: should be derived based on below values? TODO: Make behaviour here a bitwise enum?
#define IS_GSM_MOCK					true	//Without connecting GSM shield
#define IS_GPS_MOCK					true	//Without connecting GPS shield

#define INITIALISE_MODULE_ID		0		//0 implies don't initialise it - modules start with id 1
#define IS_BASIC_MEM_TEST			false   //Smoke test new module's EEPROM is physically present and working basics

//All Extended* tests define conditional code only compiled in when set - i.e. only run on the PC
#define IS_EXTENDED_SHOW_100_BYTES	false	//Prints first 100 bytes
#define IS_EXTENDED_SHOW_MEM		false	//Prints everything on this module nicely for review - TODO: A Basic?
#define IS_EXTENDED_MEM_TEST		false	//Test reading signals and reading/writing to memory
#define IS_EXTENDED_GSM_TEST		true	//Test for gprs and sending over web
#define IS_EXTENDED_TYPES_TEST		false	//Test for RMv3 types are good, esp flags

//Fona Pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

//Digital Pins
#define PIN_SHUTDOWN 5
#define PIN_LED_BOTTOM_RED 8
#define PIN_LED_BOTTOM_GREEN 9
#define PIN_LED_TOP_RED 10
#define PIN_LED_TOP_GREEN 11

//Analog Pins
#define PIN_BATT_VOLTAGE A1
#define PIN_PV_VOLTAGE A0
#define PIN_TEMP A2
#define PIN_CURRENT A3

//Logic constants
#define GPRS_MAX_ENABLE_TIME 60	//Max time in seconds to try to enable GPRS

//Error constants
#define ERR_GPS_NO_FIX 10
#define ERR_GPS_NO_RESPONSE 11
#define ERR_GPS_BAD_FIELD 12

enum FAILURE {
	FAILURE_NONE=0,
	FAILURE_WARN_NO_DATA_TO_SEND=1
};

enum MEM_SLOT_TYPE : uint8_t {
	NoMem		= 0,
	SensorMem	= 1,
	SentMem		= 2
};

/* What the system should do on this start-up */
enum SYS_BEHAVIOUR : uint8_t {
	DoNothing	 = 0,
	TakeReadings = 1 << 0,
	SendData	 = 1 << 1
};

enum SYS_STATE {
	SysState_None				=0,
	SysState_Initialising		=1<<14,
	SysState_InTest				=1<<1,
	
	SysState_GPRSToggleError	=1<<2,
	SysState_GPRSIsOn			=1<<3,
	SysState_GPRSSendingViaNet	=1<<4,
	SysState_GPRSSendViaNetError=1<<5,
	SysState_GPRSSendViaNetSuccess=1<<6,
	
	SysState_GPRSSendingViaSms	=1<<7,
	SysState_GPRSSendViaSmsError=1<<8,
	SysState_GPRSSendViaSmsSuccess=1<<9,
	
	SysState_ShuttingDown		=1<<10, //Known shut down, not abrupt
	SysState_TakingReadings		=1<<11,
	SysState_IsCharging			=1<<12,

	SysState_OneTimeInit		=1<<13 //One time initialisation of module-ID etc.
};

enum LED_SEL/*:byte*/ {
	Top,
	Bottom
};

enum LED_STATE : uint8_t  { 
	Red_Solid   = 1,	 
	Red_Fast    = 2,	 
	Red_Slow    = 4, 
	//Red_Clear   = 8, 
	Green_Solid = 16, 
	Green_Fast  = 32, 
	Green_Slow  = 64, 
	//Green_Clear = 128,
	
	All_Clear	= 128
	
	//IsTemp		= 256
};

enum FONA_STATUS_INIT : uint8_t  {
	
	SUCCESS_FSI=1,
	WARN_ATEO_FAIL=2,
	ERR_SERIAL_FAIL=3,
	ERR_FONA_SIM_MODULE=4
	
	//Some can be mutually exclusive, some flag'd
};
#define IS_ERR_FSI(x) \
		(x == FONA_STATUS_INIT::ERR_SERIAL_FAIL || \
		 x == FONA_STATUS_INIT::ERR_FONA_SIM_MODULE)

enum FONA_STATUS_GPRS_INIT : uint8_t  {
	
	SUCCESS_FSGI=1,
	WARN_CIPSHUT_BEFORE_OPENING=2,
	ERR_CGATT_ATTACH=3,
	ERR_SAPBR_SET_CONN_TYPE=4,
	ERR_SAPBR_SET_APN=5,
	ERR_SAPBR_START_APN_TASK=6,
	ERR_SAPBR_SET_USER=7,
	ERR_SAPBR_SET_PWD=8,
	ERR_SAPBR_WHEN_OPENING=9,
	ERR_CIICR_WHEN_ON_WIRELESS=10,
	ERR_CIPSHUT_CLOSE_SOCKET=11,
	ERR_SAPBR_CLOSE_GPRS=12,
	ERR_CGATT_DETACH=13,
};
#define IS_ERR_FSGI(x) \
		(x != FONA_STATUS_GPRS_INIT::SUCCESS_FSGI && \
		 x != FONA_STATUS_GPRS_INIT::WARN_CIPSHUT_BEFORE_OPENING)

enum FONA_STATUS_GSM_SEND {
	
	INIT_SUCCESS = 1
};

//Lowest 2 bytes for result code, next 3 bytes for netstat, next byte for error
enum FONA_GET_NETREG : uint8_t {
	RESULT_CODE_0 = 0,
	RESULT_CODE_1 = 1,
	RESULT_CODE_2 = 2,
	NETSTAT_0	  = 0 << 2,
	NETSTAT_1	  = 1 << 2,
	NETSTAT_2	  = 2 << 2,
	NETSTAT_3	  = 3 << 2,
	NETSTAT_4	  = 4 << 2,
	NETSTAT_5	  = 5 << 2,
	IS_ERROR	  = 1 << 5
};
//Macros to clear other bytes - nb: cast is critical for later equality checks
#define NETREG_ONLY_RESULT_CODE(val) \
	(FONA_GET_NETREG)(val & B00000011)
#define NETREG_ONLY_NETSTAT(val) \
	(FONA_GET_NETREG)(val & B00011100)
#define NETREG_ONLY_ERROR(val) \
	(FONA_GET_NETREG)(val & B00100000)
//Macros to deshift (eg. so netstat_5 comes out as number 5) - nb: no casting as no longer enum
#define NETREG_ACTUALVAL_RESULT_CODE(val) \
	NETREG_ONLY_RESULT_CODE(val)
#define NETREG_ACTUALVAL_NETSTAT(val) \
	NETREG_ONLY_NETSTAT(val)>>2
#define NETREG_ACTUALVAL_ERROR(val) \
	NETREG_ONLY_ERROR(val)>>5

struct FONA_GET_RSSI {
	
	uint8_t rssi	= 0;
	uint8_t ber		= 0; //bit error rate
	uint8_t rssiErr	= 0;
	
	FONA_GET_NETREG netReg;
};

struct FONA_INIT_RESULT {

		
};

//struct TransmitReadingsResult{
	//boolean HasDataToSend=false;
	//uint16_t GsmResultCode=0;
	//uint16_t SmsResultCode=0;
//}

//TODO: Kill below, mixes gps and gsm data
#ifdef GPS
struct GpsData{
	boolean success=false;
	boolean is3DFix=false;
	uint8_t rawSz = 0;
	char* raw = NULL;      //Must be set before read
	char* gsmLoc = NULL;
	uint8_t gsmLocSz = 0;
	uint16_t gsmErrCode;
};
#endif

#ifdef OUTPUT_DEBUG_MEM
	#define RM_LOGMEM(x) \
	Serial.print(x);
	
	#define RM_LOGMEMLN(x) \
	Serial.println(x);
#else
	#define RM_LOGMEM(x)
	#define RM_LOGMEMLN(x)
#endif

#ifdef OUTPUT_DEBUG
	#define RM_LOG2(x,y)  \
	Serial.print (x);\
	Serial.print (":");\
	Serial.println (y);

	#define RM_LOG(x) \
	Serial.print(x);

	#define RM_LOGLONG(x,fmt) \
	uint64_t lower = x>>32;\
	Serial.print((uint32_t)lower,HEX);\
	Serial.print((uint32_t)x,HEX);

	#define RM_LOGFMT(x,fmt) \
	Serial.print(x,fmt);
	
	#define RM_LOGLN(x) \
	Serial.println(x);
	
	#define RM_LOGLNFMT(x,fmt) \
	Serial.println(x,fmt);
	
#else
	#define RM_LOG2(x,y)
	#define RM_LOG(x)
	#define RM_LOGLONG(x,fmt) 
	#define RM_LOGFMT(x,fmt)
	#define RM_LOGLN(x)
	#define RM_LOGLNFMT(x,fmt)
#endif


//struct GsmInfo{
//
//
	////TODO: IS THAT IT?
	//
	//
	//uint8_t errorCode;
	//uint8_t rssi;
	//uint8_t networkStatus;
//};

struct GpsInfo{

	uint8_t errorCode=0; //0 implies no error
	int8_t gpsStatus=0;
	
	float lat=0;
	float lon=0;
	float speed_kph=0;
	float heading=0;
	float altitude=0;
	char date[15]; //Format yyyyMMddHHmmss with \0
};

struct SingleSession{
	
	//GsmInfo gsmInfo;
	GpsInfo gpsInfo;
};

/* Stored at start of ROM. */
struct ModuleMeta{

	uint8_t  moduleId = 0;
	
	/* No of times module has booted up incase numReadings cycles/fails */
	uint16_t bootCount = 0;
	
	/* Where the next sensor data or daily cycle data can go */
	uint16_t nextFreeWriteAddr = 0;
	
	/* Dedicated area where testing eeprom will write and read to */
	uint64_t eepromTestArea = 0;
	
	uint8_t spareBuffer[16] = {0}; //Sets all elems to 0
};


struct SensorData {
	
	MEM_SLOT_TYPE  dataType = MEM_SLOT_TYPE::SensorMem;
	uint16_t battVoltage	= 0;
	uint16_t current		= 0;
	uint16_t pVVoltage		= 0;
	uint16_t temperature	= 0;
	uint8_t  errorChar		= 0;/* TODO: uint16_32 for bitwise errs?*/ /* not unsigned else will get treated like int by string ctor */
	//bool           HasBeenSent	= false;
};

//Populate a struct with data which will be compacted with Base64 and sent via gprs
//struct SendViaGprsData {
//
	//uint8_t moduleId		= 0;
	//uint16_t thisBootNumber = 0;
	//FONA_GET_RSSI rssi;
	////uint8_t content_flags;
	//
	//uint8_t hasGpsInfo		= 0;
	//GpsInfo* gpsInfo;
	//
	//uint8_t numOfReadings	= 0;
	//SensorData* data;
	//
	//SensorData arrData[0]; //Size=0, we don't want pointer as it takes up size
//};

//What was the result of trying to send via GPRS, incl. error codes. Store in EEPROM.
struct SendViaGprsResult {
	
	uint8_t status;
};

/* Stored in ROM and attempted to be sent every day along with readings */
struct DailyCycleData {
	
	//TODO: BITWISE OF ALL FAILURE CODE, INCLUDING IN FONA?
	
	MEM_SLOT_TYPE DataType		  = MEM_SLOT_TYPE::SentMem;
	//TODO: AttemptedSend ?
	unsigned long BootNo		  = 0;
	boolean GPRSToggleFailure	  = false;
	boolean GetBatteryFailure	  = false;
	uint8_t NoOfReadings		  = 0; /* NoOfReadings in this transmission */
	uint16_t GsmMessageLength	  = 0; /* Length of string that was attempted to send */
	uint16_t GsmFailureCode		  = 0;
	uint8_t SmsFailureCode		  = 0;
	uint8_t	BattPct				  = 0;
	uint8_t NetworkStatus		  = 0;
	uint8_t RSSI				  = 0;
	SYS_STATE SystemState	      = SysState_Initialising; //Bitwise combination of sys state
	
};

//typedef struct GsmTransmitData{
	//
	//unsigned int NoOfReadings = 0;
	//unsigned byte* Readings   = NULL;
	//unsigned int ReadingsLength = 0;
	//unsigned int ErrorCodes = 0;
//} GsmTransmitData;

//typedef struct SmsTransmitData{
//} SmsTransmitData;



#endif //__DATATYPES_H__