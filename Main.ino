/*
   MbeguSolar
   Version 1.0
   2021/01/25

*/


#include <Keypad.h>

#include "Constants.h"
#include "StationConfig.h"  
#include "PinConfig.h"
#include "PersistentStorageManager.h"
#include "RtcManager.h"
#include "AccessManager.h"
#include "LEDManager.h"

char entryBuffer[entryBufferSize+1] = {}; //Stores entered key presses, global
uint32_t timeNow = 0; //global
uint32_t lastTimeKeyPressed = 0; //global

void setup(){
  Serial.begin(9600);
  delay(setupDelay);
  keypad.setDebounceTime(debounceTime);
  rtcSetup(); //verify RTC connected
  bool firstTime = firstTimeFlashSetup(); //Set up persistant storage in flash after reprogramming
  if (firstTime){
    rtcResetTime(); //RTC should be set to compilation time after reprogramming
  }

  /*  Pin config  */
  for(int i=0;i<4;i++){
    pinMode(lockersConfig[i].relayPin, OUTPUT);
    digitalWrite(lockersConfig[i].relayPin, HIGH);
  }
  pinMode(LEDRed, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  pinMode(LEDBlue, OUTPUT);


  Serial.println("Welcome");
  signalStartup();
}

void loop(){
  char pressedKey = keypad.getKey(); //Get keypress, does not block
  //TODO: Clean up master code detect
  //TODO: Validation of key presses should be done in one step
  if (pressedKey != NO_KEY){
    if (pressedKey != entryBufferResetKey){ //entry buffer cleared when '*' (default) is pressed
      recordKeyPress(&pressedKey);
      // TODO: Optimize this. Buffer should clear on invalid entries.
      if (strlen(entryBuffer) == pinLength + 2 && entryBuffer[1] == pinSpacerKey && entryBuffer[2] != pinSpacerKey){ //PIN ENTRY TODO: length should not be hardcoded
        /* userPin entry 
         * Pin has form 'X#YYYY' for locker X pin YYYY
         */
        handlePinEntry();
        memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
      }
      else if (strlen(entryBuffer) == 11 && entryBuffer[accessKeyFullLength]){ //TODO: remove hardcoded 11
        /* accessKey entry 
         * accessKey has form 'XXXXXY#ZZZZ' for lockerTag Y and chosen userPin ZZZZ
         */
        handleAccessKeyEntry();
        memset(entryBuffer, 0, strlen(entryBuffer));
      } 
      else if (strlen(entryBuffer) == masterCodeLen + 3 && entryBuffer[1] == pinSpacerKey && entryBuffer[2] == pinSpacerKey) { //TODO: removed hardcoded len
        /* masterCode entry 
         * code has form 'X##YYYYYY' for locker X masterCode YYYYYY
         */
        handleMasterCodeEntry();
        memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
      }
    }
    else{
      Serial.println("Clearing Keypad (clear key used)");
      memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
      signalClearBuffer();
    }
  }
  else{
    /*Clear entered keys after certain delay if buffer not empty*/
    uint32_t timeTemp = millis();
    if(timeTemp - lastTimeKeyPressed >= keypadClearTimeout){ 
      if (entryBuffer[0] != '\0'){
        Serial.println("Keypad has timed out. Buffer cleared.");
        memset(entryBuffer,0,strlen(entryBuffer)); //clear buffer
        signalClearBuffer();
      }
    }
  }
}

void handleAccessKeyEntry() {
  /*
  * Validates accessKey and opens locker and saves chosen pin if accepted 
  */
  /*Access key entry has form "XXXXXZ#YYYY" for access key XXXXX, chosen pin YYYY and locker Z where 
    0<=Z<=2 => locker 1
    3<=Z<=5 => locker 2
    6<=Z<=8 => locker 3
    9<=Z<=B => locker 4 
  */
  const int lockerRequested = getRequestedLocker(entryBuffer[lockerTagIndex]); //Get lockerID from lockerTag
  Serial.print("Access key entry detected - ");
  if (1 <= lockerRequested && lockerRequested <= numberOfLockers) //accept only valid lockers
  {
    Serial.print("Locker: ");
    Serial.println(lockerRequested); 
    char accessKeyPass[accessKeyFullLength];  // Holds accessKeyPass, accessKeyFull without lockerTag (5 digits)
    strncpy(accessKeyPass, entryBuffer, accessKeyFullLength - LOCKER_ID_LEN); //extract accessKeyPass from full entry (remove lockerTag)
    char pin[pinLength + 1];
    for (int i = 0; i < pinLength; i++){
      pin[i] = entryBuffer[accessKeyFullLength + 1 + i]; //extract user selected pin from entry, from index accessKeyLength + "#"
      if (!(isdigit(pin[i]) || pin[i] == 'A' || pin[i] == 'B' || pin[i] == 'C' || pin[i] == 'D')){ //accept only numerical digits and A-D
        Serial.println("Chosen pin rejected - only digits 0-9 and A-D allowed");
        signalRejectedPassword();
        memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
        return;
      }
    }
    pin[pinLength] = '\0';
    uint32_t accessKeyTimestamp = 0;
    accessKeyTimestamp = validateAccessKey(lockerRequested, accessKeyPass); //returns the timestamp of when the access key was generated or 0 if invalid
    if (accessKeyTimestamp)
    {
      updateLocker(lockerRequested, pin, accessKeyTimestamp); //save chosen userPin, expiry time and update lastChance to memory
      performUnlockingProcedure(lockersConfig[lockerRequested - 1]);
    }
    else
    {
      signalRejectedPassword();
    }
  }
  else
  {
    Serial.print("Requested locker is invalid");
    signalRejectedPassword();
  }
  memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
}

void handleMasterCodeEntry(){
  /*
  * Validates master code and opens locker if correct
  */
  Serial.print("Master code entry detected - ");
  const int lockerRequested = entryBuffer[0] - '0'; //Char to int
  if (1 <= lockerRequested && lockerRequested <= numberOfLockers) {
    Serial.print("locker: ");
    Serial.println(lockerRequested);
    char code[masterCodeLen + 1];
    for (int i = 0; i < masterCodeLen; i++) {
      code[i] = entryBuffer[i + 3]; //copy masterCode from buffer
    }
    code[masterCodeLen] = '\0';
    bool codeValid = validateMasterCode(code); //Validate code
    if (codeValid){
      acceptMasterCode(lockerRequested); //Open locker and increment masterCode index
    } else {
      signalRejectedPassword();
      Serial.println("Master code is incorrect");
    }
  } else {
    signalRejectedPassword();
    Serial.println("Locker requested invalid");
  }
}

void handlePinEntry(){
  /*
  * Validates pin entry and opens locker if correct
  */
  Serial.print("Pin entry detected - ");
  const int lockerRequested = entryBuffer[0] - '0'; //Char to Int
  if (1 <= lockerRequested && lockerRequested <= numberOfLockers) { //Valid locker entered
    Serial.print("Locker: ");
    Serial.println(lockerRequested);
    char pin[pinLength + 1];
    for (int i = 0; i < pinLength; i++) {
      pin[i] = entryBuffer[i + 2]; //copy pin from entryBuffer
    }
    pin[pinLength] = '\0';
    bool pinValid = validatePin(lockerRequested, pin); //checks pin against pin stored
    pinValid ? performUnlockingProcedure(lockersConfig[lockerRequested - 1]) : signalRejectedPassword();
  }
  else
  {
    Serial.print("Requested locker: ");
    Serial.print(lockerRequested);
    Serial.println(" is invalid");
    signalRejectedPassword();
  }
  memset(entryBuffer, 0, strlen(entryBuffer)); //clear buffer
}

void updateLocker(int lockerToUpdate, char pin[], uint32_t accessKeyTimestamp)
{
  /*
  This function updates the locker data in flash. New pin, expiry time, and lastChance updated.
  */
  uint32_t unixTimePinExpiry = accessKeyTimestamp + lockerUseTime;
  Serial.print("Updating locker: ");
  Serial.print(lockerToUpdate);
  Serial.print(", Pin: ");
  Serial.print(pin);
  Serial.print(", unixTimePinExpiry: ");
  Serial.println(unixTimePinExpiry);
  writeLockerToFlash(lockerToUpdate, pin, unixTimePinExpiry);
}

void recordKeyPress(const char* pressedKey){
    /*
    * Records the key pressed into the global entryBuffer
    */
  timeNow = millis();
  lastTimeKeyPressed = timeNow;
  signalKeyPressed();
  strncat(entryBuffer, pressedKey, 1);
  Serial.print("Entry buffer: ");
  Serial.println(entryBuffer);

}
