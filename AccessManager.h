# ifndef AccessManager
# define AccessManager

#include <Crypto.h>
#include <SHA1.h>

#include "Constants.h"
#include "StationConfig.h"
#include "PinConfig.h"
#include "PersistentStorageManager.h"
#include "RtcManager.h"
#include "LEDManager.h"

SHA1 sha1;

void performUnlockingProcedure(Locker locker);
bool validateHash(Hash *hash, char *data, char *accessKeyPass);
bool validateMasterCode(char *masterCode);
void acceptMasterCode(int lockerRequested);
bool validatePin(int lockerRequested, char pin[pinLength + 1]);
uint32_t validateAccessKey(int lockerRequested, char accessKeyPass[]);
uint8_t getRequestedLocker(char lockerTag);
void signalRejectedPassword();


void performUnlockingProcedure(Locker locker)
{
    //TODO: Check timings
    digitalWrite(locker.relayPin, LOW);
    signalEntryAccepted();
    Serial.print("Locker open: ");
    Serial.println(locker.ID);
    delay(lockerOpenTime);
    Serial.print("Locker closed: ");
    Serial.println(locker.ID);
    digitalWrite(locker.relayPin, HIGH);
}

bool validateMasterCode(char* masterCodeEntered) {
    /*
    * Validates master code based on saved rotating master codes 
    * Gets index from flash and selects corresponding code
    */
   uint8_t masterCodeIndex = readMasterCodeIndex(); 
   const char* currentMasterCode = masterCodes[masterCodeIndex]; 
   if (strncmp(masterCodeEntered, currentMasterCode, masterCodeLen) == 0) {
       return true;
   }
   return false;
}

void acceptMasterCode(int lockerRequested) {
    incrementMasterCodeIndex();
    performUnlockingProcedure(lockersConfig[lockerRequested - 1]);
}

bool validatePin(int lockerRequested, char pin[pinLength + 1]) {
    /*
    * Compares entered pin with pin saved in flash
    */
    Locker lockerData = readLockerFromFlash(lockerRequested); // Get locker data from flash and read in
    uint32_t pinTimeoutFromFlash = lockerData.passwordTimeout;
    bool lastChanceActive = lockerData.lastChance;
    char pinFromFlash[pinLength + 1];
    strcpy(pinFromFlash, lockerData.userPin);
    
    Serial.print("pinFromFlash: "); //TODO: Remove
    Serial.println(pinFromFlash); 
    Serial.print("lockerData.userPin: ");
    Serial.println(lockerData.userPin);
    Serial.print("pinTimeoutFromFlash: ");
    Serial.println(pinTimeoutFromFlash);
    Serial.print("lastChanceActive: ");
    Serial.println(lastChanceActive);
    

    uint32_t unixTimeNow = getUnixTimeNow();
    Serial.print("Pin: ");
    Serial.println(pin); 

    if (strcmp(pin, pinFromFlash) == 0)
    {
        Serial.print("Accepted");
        if (unixTimeNow < pinTimeoutFromFlash)
        {
            Serial.println(" and still valid");
            return true;
        }
        else
        {
            Serial.print(" but expired");
            if (lastChanceActive)
            {
                Serial.println(" - Last chance valid, using last chance");
                disableLastChance(lockerRequested);
                return true;
            }
        }
    }
    Serial.println(" - Entry Rejected");
    return false;
}


uint32_t validateAccessKey(int lockerRequested, char accessKeyPass[]) {
    /*
  * Calculates hash from stationID, lockerID, and current timestamp and compares with entered accessKey
  * Also accepts key from previous timestamp
  */
    uint32_t timeNow = getUnixTimeNow();
    char data[HASH_DATA_LEN + 1]; //data to hash is conc string: hardwareID + lockerID + timestamp
    char lockerString[2];
    itoa(lockerRequested, lockerString, 10);
    timeNow -= timeNow % timeStampIncrement; // Round to nearest timeStampIncrement below
    char timeString[TIME_STAMP_LEN + 1];
    itoa(timeNow, timeString, 10);

    strcpy(data, hardwareID);
    strcat(data, lockerString);
    strcat(data, timeString);

    Serial.print("Validating accessKey: ");
    Serial.print(accessKeyPass);
    Serial.print(" with hardwareID: "); //TODO: Remove for production
    Serial.print(hardwareID);
    Serial.print(", lockerID: ");
    Serial.print(lockerString);
    Serial.print(", timeStamp: ");
    Serial.println(timeString);
  
    bool keyValid = validateHash(&sha1, data, accessKeyPass); //calculates hash and compares to entered key
    if (keyValid)
    { // accessKey valid for current timestamp
        Serial.println("Access key accepted");
        return timeNow;
    }
    else
    { // accessKey also accepted for previous and next timestamp. Previous to allow longer validity period in case of sms delay to user.
        timeNow -= timeStampIncrement; //Get previous valid timestamp
        itoa(timeNow, timeString, 10);
        strcpy(data + HARDWARE_ID_LEN + 1, timeString); //replace time with new timestamp
        Serial.print("Trying timeStamp: ");
        Serial.println(timeString);
        keyValid = validateHash(&sha1, data, accessKeyPass); //calculates hash and compares to entered key
        if (keyValid)
        {
            Serial.println("Access key accepted for previous time timestamp");
            return timeNow;
        } 
        else { 
            timeNow += 2*timeStampIncrement; //next timestamp
            itoa(timeNow, timeString, 10);
            strcpy(data + HARDWARE_ID_LEN + 1, timeString); //replace time with new timestamp
            Serial.print("Trying timeStamp: ");
            Serial.println(timeString);
            keyValid = validateHash(&sha1, data, accessKeyPass); //calculates hash and compares to entered key
            if (keyValid) {// next timestamp also accepted. Due to sync issues.
                Serial.println("Access key accepted for next timestamp");
                return timeNow;
            }
        }
    }
    Serial.println("Access key rejected");
    return 0;
}

bool validateHash(Hash *hash, char *data, char *accessKeyPass)
{
    /*
    * Calculates hash from given data and compares to accessKeyPass
    * Uses given hash function
    * TODO: generalize and refactorize
    */
    uint8_t value[HASH_SIZE]; //stores hash
    hash->reset();            //hash needs to be reset every time it is used
    hash->update(data, HASH_DATA_LEN);
    hash->finalize(value, sizeof(value)); //get hash and truncate to fit value[]
    uint32_t numBuffer = 0;
    for (int i = 0; i < HASH_SIZE; i++)
    {
        numBuffer += value[i] << 8 * (HASH_SIZE - i - 1); //combine hash, goes from value[] == {0xab, 0xcd, 0xef} to buffer == 0xabcdef
    }
    //TODO: REMOVE ALL FOR PRODUCTION
    Serial.print("Truncated hash (HEX): ");
    Serial.println(numBuffer, HEX);
    char keyCalculated[HASH_SIZE * 2 + 1]; //Hex to base 12, length icreases max by 1 digit
    itoa(numBuffer, keyCalculated, 12);
    Serial.print("Truncated hash (base-12): ");
    Serial.println(keyCalculated);
    Serial.print("Comparing first: ");
    Serial.print(strlen(accessKeyPass));
    Serial.println(" digits");
    char c;
    int i = 0;
    //capitalize all
    while (keyCalculated[i]){
        c = keyCalculated[i];
        c = toupper(c);
        keyCalculated[i] = c;
        i++;
    }
    if (strncmp(keyCalculated, accessKeyPass, strlen(accessKeyPass)) == 0)
    {
        return true;
    }
    return false;
}

uint8_t getRequestedLocker(char lockerTag) {
    /* 
  Converts lockerTag to lockerID
  Access key entry has form "XXXXXZ#YYYY" for access key XXXXX, chosen pin YYYY and locker Y where 
          0<=Y<=2 => locker 1
          3<=Y<=5 => locker 2
          6<=Y<=8 => locker 3
  */
    if (lockerTag == '0' || lockerTag == '1' || lockerTag == '2')
    {
        return 1;
    }
    else if (lockerTag == '3' || lockerTag == '4' || lockerTag == '5')
    {
        return 2;
    }
    else if (lockerTag == '6' || lockerTag == '7' || lockerTag == '8')
    {
        return 3;
    }
    else if (lockerTag == '9' || lockerTag == 'A' || lockerTag == 'B')
    {
        return 4;
    }
    else
    {
        return false;
    }
}



# endif
