#ifndef ADAFRUIT_FONA_H
#define ADAFRUIT_FONA_H

#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "DataTypes.h"
#include "Helpers.h"
//#include "includes/platform/FONAPlatform.h"

#define FONA808_V2 3

#define FONA_HEADSETAUDIO 0
#define FONA_EXTAUDIO 1

#define FONA_DEFAULT_TIMEOUT_MS 500

#define FONA_HTTP_GET   0
#define FONA_HTTP_POST  1
#define FONA_HTTP_HEAD  2

#define FONA_CALL_READY 0
#define FONA_CALL_FAILED 1
#define FONA_CALL_UNKNOWN 2
#define FONA_CALL_RINGING 3
#define FONA_CALL_INPROGRESS 4





#define DebugStream		Serial

#ifdef ADAFRUIT_FONA_DEBUG
// need to do some debugging...
#define DEBUG_PRINT(...)		DebugStream.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)		DebugStream.println(__VA_ARGS__)
#endif

#ifndef DEBUG_PRINT
// debug is disabled
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

// a few typedefs to keep things portable
typedef	Stream 						FONAStreamType;
typedef const __FlashStringHelper *	FONAFlashStringPtr;

#define prog_char  					char PROGMEM

#define prog_char_strcmp(a, b)					strcmp_P((a), (b))
// define prog_char_strncmp(a, b, c)				strncmp_P((a), (b), (c))
#define prog_char_strstr(a, b)					strstr_P((a), (b))
#define prog_char_strlen(a)						strlen_P((a))
#define prog_char_strcpy(to, fromprogmem)		strcpy_P((to), (fromprogmem))




class Adafruit_FONA : public FONAStreamType {
	
 public:
  Adafruit_FONA(int8_t r, boolean isMock);
  FONA_STATUS_INIT begin(uint8_t tx, uint8_t rx);
  uint8_t type();

  // Stream
  int available(void);
  size_t write(uint8_t x);
  int read(void);
  int peek(void);
  void flush();

  //Power
  boolean powerOff();
  boolean toggleCharging(boolean onOff);

  // Battery and ADC
  boolean getADCVoltage(uint16_t *v);
  boolean getBattPercent(uint16_t *p);
  boolean getBattVoltage(uint16_t *v);

  // SIM query
  uint8_t unlockSIM(char *pin);
  uint8_t getSIMCCID(char *ccid);
  FONA_GET_RSSI getRSSI(void);

  // IMEI
  uint8_t getIMEI(char *imei);

  // SMS handling
  int8_t getNumSMS(void);
  boolean readSMS(uint8_t i, char *smsbuff, uint16_t max, uint16_t *readsize);
  boolean sendSMS(char *smsaddr, char *smsmsg);
  boolean deleteSMS(uint8_t i);
  boolean getSMSSender(uint8_t i, char *sender, int senderlen);
  
  boolean sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen);

  // GPRS handling
  FONA_STATUS_GPRS_INIT enableGPRS(boolean onoff);
  uint8_t GPRSstate(void);
  boolean getGSMLoc(uint16_t *replycode, char *buff, uint16_t maxlen);
  boolean getGSMLoc(float *lat, float *lon);
  void setGPRSNetworkSettings(FONAFlashStringPtr apn, FONAFlashStringPtr username=0, FONAFlashStringPtr password=0);

  // GPS handling
  boolean enableGPS(boolean onoff);
  int8_t GPSstatus(void);
  uint8_t getGPS(uint8_t arg, char *buffer, uint8_t maxbuff);
  boolean getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude, char *date);
  boolean enableGPSNMEA(uint8_t nmea);

  // Send data over GPRS
  FONA_STATUS_GPRS_SEND sendDataOverGprs(
						  uint8_t* sendData, uint16_t sendDataLength,
						  char* response, uint16_t maxResponseLength, uint16_t* actualResponseLength,
						  uint16_t*statuscode);

  // HTTP high level interface (easier to use, less flexible).
  boolean HTTP_GET_start(FONAFlashStringPtr url, uint16_t *status, uint16_t *datalen);
  void HTTP_GET_end(void);
  
  FONA_STATUS_GPRS_SEND HTTP_POST_start(
							FONAFlashStringPtr url, FONAFlashStringPtr contenttype,
							const uint8_t *postdata, uint16_t postdatalen, uint16_t *status,
							uint16_t maxResponseLen, uint16_t *actualResponselen, char* response);
  
  void HTTP_POST_end(void);
  void setUserAgent(String useragent);

  // PWM (buzzer)
  boolean setPWM(uint16_t period, uint8_t duty = 50);

  // Helper functions to verify responses.
  boolean expectReply(FONAFlashStringPtr reply, uint16_t timeout = 10000);
  boolean sendCheckReply(char *send, char *reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr send, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(char* send, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);


 protected:
  int8_t _rstpin;
  boolean _isMock;
  uint8_t _type = FONA808_V2;

  char replybuffer[255];
  FONAFlashStringPtr apn;
  FONAFlashStringPtr apnusername;
  FONAFlashStringPtr apnpassword;
  boolean httpsredirect;
  String useragent;
  FONAFlashStringPtr ok_reply;

  // HTTP helpers
  FONA_STATUS_GPRS_SEND HTTP_setup(FONAFlashStringPtr url);
  
  //FONA_STATUS_GPRS_INIT Adafruit_FONA::internalEnableGPRS(boolean onoff);
  void internalReadRawData(char* response, uint16_t lenToRead);
  
  void flushInput();
  uint16_t readRaw(uint16_t b);
  uint8_t readline(uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS, boolean multiline = false);
  uint8_t getReply(char *send, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr send, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, char *suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, int32_t suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout); // Don't set default value or else function call is ambiguous.
  uint8_t getReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);

  boolean sendCheckReply(FONAFlashStringPtr prefix, char *suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, int32_t suffix2, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);

	boolean HTTP_init();
	boolean HTTP_term();
	void HTTP_para_start(FONAFlashStringPtr parameter, boolean quoted = true);
	boolean HTTP_para_end(boolean quoted = true);
	boolean HTTP_para(FONAFlashStringPtr parameter, const char *value);
	boolean HTTP_para(FONAFlashStringPtr parameter, FONAFlashStringPtr value);
	boolean HTTP_para(FONAFlashStringPtr parameter, int32_t value);
	boolean HTTP_para(FONAFlashStringPtr parameter, const String value);
	boolean HTTP_data(uint32_t size, uint32_t maxTime=10000);
	FONA_STATUS_GPRS_SEND HTTP_action(uint8_t method, uint16_t *status, uint16_t *datalen, int32_t timeout = 10000);
	boolean HTTP_readall(uint16_t maxReadSz, char *response);
	boolean HTTP_ssl(boolean onoff);

  boolean parseReply(FONAFlashStringPtr toreply,
          uint16_t *v, char divider  = ',', uint8_t index=0);
  boolean parseReply(FONAFlashStringPtr toreply,
          char *v, char divider  = ',', uint8_t index=0);
  boolean parseReplyQuoted(FONAFlashStringPtr toreply,
          char *v, int maxlen, char divider, uint8_t index);

  boolean sendParseReply(FONAFlashStringPtr tosend,
       FONAFlashStringPtr toreply,
       uint16_t *v, char divider = ',', uint8_t index=0);

  static boolean _incomingCall;
  static void onIncomingCall();

  SoftwareSerial *mySerial;
  //SoftwareSerial *fonaSerial;
};

#endif

/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
