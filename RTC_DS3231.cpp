/*
  RTC_DS3231.cpp - library for DS3231 RTC
*/
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Wire.h>
#include "RTC_DS3231.h"

RTC_DS3231::RTC_DS3231()
{
  Wire.begin();
}
  
// PUBLIC FUNCTIONS

bool RTC_DS3231::lostPower(void) {
  return (read_i2c_register(DS3231_ADDRESS, DS3231_STATUSREG) >> 7);
}

time_t RTC_DS3231::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  read(tm);
  return(makeTime(tm));
}

void  RTC_DS3231::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  tm.Second |= 0x80;  // stop the clock
  write(tm); 
  tm.Second &= 0x7f;  // start the clock
  write(tm); 
}

// Aquire data from the RTC chip in BCD format
void RTC_DS3231::read( tmElements_t &tm)
{
  Wire.beginTransmission(DS3231_CTRL_ID);
  Wire.write((uint8_t)0);
  Wire.endTransmission();

  // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
  Wire.requestFrom(DS3231_CTRL_ID, tmNbrFields);
  
  tm.Second = bcd2dec(Wire.read() & 0x7f);   
  tm.Minute = bcd2dec(Wire.read() );
  tm.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  tm.Wday = bcd2dec(Wire.read() );
  tm.Day = bcd2dec(Wire.read() );
  tm.Month = bcd2dec(Wire.read() );
  tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
}

void RTC_DS3231::write(tmElements_t &tm)
{
  Wire.beginTransmission(DS3231_CTRL_ID);
  Wire.write((uint8_t)0); // reset register pointer
  
  Wire.write(dec2bcd(tm.Second)) ;   
  Wire.write(dec2bcd(tm.Minute));
  Wire.write(dec2bcd(tm.Hour));      // sets 24 hour format
  Wire.write(dec2bcd(tm.Wday));   
  Wire.write(dec2bcd(tm.Day));
  Wire.write(dec2bcd(tm.Month));
  Wire.write(dec2bcd(tmYearToY2k(tm.Year)));   

  Wire.endTransmission();  
}

float RTC_DS3231::getTemp()
{
  byte tMSB, tLSB;
  float temp3231;

  //temp registers (11h-12h) get updated automatically every 64s
  Wire.beginTransmission(DS3231_CTRL_ID);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_CTRL_ID, 2);
  
  if(Wire.available()) {
    tMSB = Wire.read(); //2's complement int portion
    tLSB = Wire.read(); //fraction portion
    
    temp3231 = (tMSB & B01111111); //do 2's math on Tmsb
    temp3231 += ( (tLSB >> 6) * 0.25 ); //only care about bits 7 & 8
	//temp3231 = ((((short)tMSB << 8) | (short)tLSB) >> 6) / 4.0;
    //temp3231 = (temp3231 * 1.8) + 32.0;
    return temp3231;
  }
  else {
    //oh noes, no data!
  }
    
  return 0;
}
// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t RTC_DS3231::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t RTC_DS3231::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

// PROTECTED FUNCTIONS

void set_sreg(uint8_t val) {
	Wire.beginTransmission(DS3231_CTRL_ID);
	Wire.write(DS3231_STATUS_ADDR);
	Wire.write(val);
	Wire.endTransmission();
}

uint8_t get_sreg() {
	uint8_t rv;

	Wire.beginTransmission(DS3231_CTRL_ID);
	Wire.write(DS3231_STATUS_ADDR);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_CTRL_ID, 1);
	rv = Wire.read();

	return rv;
}

void set_creg(uint8_t val) {
	Wire.beginTransmission(DS3231_CTRL_ID);
	Wire.write(DS3231_CONTROL_ADDR);
	Wire.write(val);
	Wire.endTransmission();
}

uint8_t get_creg() {
	uint8_t rv;

	Wire.beginTransmission(DS3231_CTRL_ID);
	Wire.write(DS3231_CONTROL_ADDR);
	Wire.endTransmission();

	Wire.requestFrom(DS3231_CTRL_ID, 1);
	rv = Wire.read();

	return rv;
}

RTC_DS3231 RTC = RTC_DS3231(); // create an instance for the user
