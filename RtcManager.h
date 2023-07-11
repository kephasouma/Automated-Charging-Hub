/*
* RTC Manager
* Code largely taken as is from Adafruit RTClib
*/

# ifndef RtcManager
# define RtcManager

#include <RTClib.h>

RTC_DS3231 rtc;

uint32_t getUnixTimeNow(){
    return rtc.now().unixtime();
}

void rtcResetTime(){
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));

  DateTime timeNow = rtc.now();

  Serial.print("Time set: ");
  Serial.print(timeNow.year(), DEC);
  Serial.print('/');
  Serial.print(timeNow.month(), DEC); 
  Serial.print('/');
  Serial.print(timeNow.day(), DEC);
  Serial.print(' ');
  Serial.print(timeNow.hour(), DEC);
  Serial.print(':');
  Serial.print(timeNow.minute(), DEC);
  Serial.print(':');
  Serial.print(timeNow.second(), DEC);
  Serial.println();
  Serial.print("Unix time (s): ");
  Serial.print(getUnixTimeNow());
  Serial.println();
}

void rtcSetup() {
    # ifndef ESP8266
        while (!Serial);
    # endif

    if (!rtc.begin()){
        Serial.println("ERROR: Could not find RTC");
        Serial.flush();
        abort();
    }

  DateTime timeNow = rtc.now();

  Serial.print("Current time: ");
  Serial.print(timeNow.year(), DEC);
  Serial.print('/');
  Serial.print(timeNow.month(), DEC); 
  Serial.print('/');
  Serial.print(timeNow.day(), DEC);
  Serial.print(' ');
  Serial.print(timeNow.hour(), DEC);
  Serial.print(':');
  Serial.print(timeNow.minute(), DEC);
  Serial.print(':');
  Serial.print(timeNow.second(), DEC);
  Serial.println();
  Serial.print("Unix time (s): ");
  Serial.print(getUnixTimeNow());
  Serial.println();
}
# endif
