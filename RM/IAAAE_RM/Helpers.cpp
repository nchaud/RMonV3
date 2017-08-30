#ifndef __RM_HELPERS_CPP__
#define __RM_HELPERS_CPP__

#include "Helpers.h"
#include <Arduino.h>


uint8_t getNumOfPadChars(unsigned int value, uint8_t padLength)
{
	uint8_t padChars = 0;
	
	if (value <10)
		padChars = max(0, padLength-1);
	else if (value < 100)
		padChars = max(0, padLength-2);
	else if (value < 1000)
		padChars = max(0, padLength-3);
	else if (value < 10000)
		padChars = max(0, padLength-4);
	else
		padChars = max(0, padLength-5);

	return padChars;	
}


byte writeCharWithPad(char* buffer, const char value, byte padLength)
{
	byte padChars = max(0, padLength-1);
	
	//First pad
	for(byte i=0;i<padChars;i++)
		*(buffer++) = '0';
	
	*(buffer++) = value;

	return padChars + 1;
}



byte writeCharArrWithPad(char* buffer, const char* value, byte padLength)
{
	byte valLen = strlen(value);
	byte padChars = max(0, padLength-valLen);
	
	//First pad
	for(byte i=0;i<padChars;i++)
		*(buffer++) = '0';
	
	for(byte i=0;i<valLen;i++)
		*(buffer++) = *(value+i);

	return padChars + valLen;
}

//byte writeStrWithPad(char* buffer, String str, byte padLength)
//{
	//const char* strRaw = str.c_str();
	//
	//volatile const char* test = strRaw;
	//
	//return writeCharArrWithPad(buffer, strRaw, padLength);
//}

byte writeByteWithPad(char* buffer, byte value, byte padLength)
{
	byte padChars =getNumOfPadChars(value, padLength);

	for(byte i=0;i<padChars;i++)
		*(buffer++) = '0';
	
	utoa(value, buffer, 10);

	byte offset = 0;
	while(*(buffer+offset)!= 0){offset++;} //However many characters it took
	
	return padChars + offset;	
}

byte writeWithPad(char* buffer, unsigned int value, byte padLength)
{
	byte padChars =getNumOfPadChars(value, padLength);

	for(byte i=0;i<padChars;i++)
		*(buffer++) = '0';
	
	utoa(value, buffer, 10);

	byte offset = 0;
	while(*(buffer+offset)!= 0){offset++;} //However many characters it took
	
	return padChars + offset;
}

//byte writeCharArrToStrWithPad(String str, uint8_t idx, const char* value, byte padLength)
//{
//}

//byte writeStrToStrWithPad(String str, uint8_t idx, String value, uint8_t padLength)
//{
	////Promote to signed before subtraction to detect negative
	//int8_t buffLength = max(0, (int8_t)padLength - (int8_t)value.length());
//
	//for(uint8_t i=0;i<buffLength;i++)
		//str[idx++] = '0';
//
//volatile char c1 = value[0];
//volatile char c2 = value[1];
//
	//for(uint8_t i=0;i<value.length();i++)
	//{
		//volatile char cc = value[i];
		//str[idx++] = value[i];
	//}
//}

//byte writeToStrWithPad(String str, uint8_t idx, unsigned int value, byte padLength)
//{
	//byte padChars = getNumOfPadChars(value, padLength);
	//
	//for(uint8_t i=0;i<padChars;i++)
		//str.setCharAt(idx++, '0');
		//
	////Assume max number of digits is 10
	//char buffer[10];
	//utoa(value, buffer, 10);
	//
	//uint8_t offset=0;
	//while(*(buffer+offset)!= 0){str.setCharAt(idx++,buffer[offset]);} //However many characters it took
	//
	//return padChars + offset;
//}


#endif