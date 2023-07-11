/*
    Persistant Storage Manager
    Valid for ArduinoDue, storage in flash using DueFlashStorage
    TODO: Refactor
*/

# ifndef PersistantStorageManager
# define PersistantStorageManager

#include <DueFlashStorage.h>
DueFlashStorage FlashStorage;

#include "StationConfig.h"
#include "Constants.h"

struct Locker
{
    uint8_t relayPin;
    uint8_t ID;
    char userPin[pinLength+1];
    uint32_t passwordTimeout;
    bool lastChance;
};
typedef struct Locker Locker;

//Quite an ugly solution. Perhaps just initialize everything in firstTimeSetup? Needs some general refactoring and fixing.
//Dependencies in several functions
Locker lockersConfig[numberOfLockers] = {
    {locker1RelayPin, 1, {}, 0, false},
    {locker2RelayPin, 2, {}, 0, false},
    {locker3RelayPin, 3, {}, 0, false},
    {locker4RelayPin, 4, {}, 0, false}
};

bool firstTimeFlashSetup(){
    /*
    * If first time running setup since flashing will configure flash storage and return true.
    */
    uint8_t firstTimeRun = FlashStorage.read(FIRST_TIME_SETUP_BYTE_ADDRESS); //Byte set to 255 at first run
    Serial.print("Code running for the first time: ");
    if (firstTimeRun) {
        Serial.println("Yes");
        byte bArray[sizeof(lockersConfig)]; //byte array for struct 
        memcpy(bArray, &lockersConfig, sizeof(lockersConfig));
        FlashStorage.write(LOCKERS_ADDRESS, bArray, sizeof(lockersConfig)); //store att specified address
        FlashStorage.write(FIRST_TIME_SETUP_BYTE_ADDRESS, 0); //set to 0 to indicate not first run
        FlashStorage.write(MASTER_CODE_INDEX_ADDRESS, 0); //set master code index to 0, start from top of list
        firstTimeRun = FlashStorage.read(FIRST_TIME_SETUP_BYTE_ADDRESS);
        if (firstTimeRun) {
            Serial.println("ERROR: Could not set FIRST_TIME_SETUP_BYTE");
            Serial.flush();
            abort();
        }
        return true;
   } else {
       Serial.println("No");
       return false;
   } 
}

Locker readLockerFromFlash(uint8_t lockerID){
    /*
    * Reads locker data for specified locker from flash and returns struct Locker 
    */
   byte* b = FlashStorage.readAddress(LOCKERS_ADDRESS);
   Locker lockersFromFlash[numberOfLockers] = {};
   memcpy(&lockersFromFlash, b, sizeof(lockersConfig));
   return lockersFromFlash[lockerID - 1];
}

void writeLockerToFlash(uint8_t lockerID, char userPin[pinLength+1], uint32_t passwordTimeout){
    /*
    * Updates and saves new userPin and associated timeout in flash, also resets lastChance
    */
    byte *b = FlashStorage.readAddress(LOCKERS_ADDRESS);
    Locker lockersFromFlash[numberOfLockers] = {};
    memcpy(&lockersFromFlash, b, sizeof(lockersConfig));

    strcpy(lockersFromFlash[lockerID - 1].userPin, userPin);
    lockersFromFlash[lockerID - 1].passwordTimeout = passwordTimeout;
    lockersFromFlash[lockerID - 1].lastChance = true;

    byte b2[sizeof(lockersFromFlash)];
    memcpy(b2, &lockersFromFlash, sizeof(lockersFromFlash));
    FlashStorage.write(LOCKERS_ADDRESS, b2, sizeof(lockersFromFlash));
    return;
}

void disableLastChance(uint8_t lockerID) {
    byte *b = FlashStorage.readAddress(LOCKERS_ADDRESS);
    Locker lockersFromFlash[numberOfLockers] = {};
    memcpy(&lockersFromFlash, b, sizeof(lockersConfig));

    lockersFromFlash[lockerID - 1].lastChance = false;

    byte b2[sizeof(lockersFromFlash)];
    memcpy(b2, &lockersFromFlash, sizeof(lockersFromFlash));
    FlashStorage.write(LOCKERS_ADDRESS, b2, sizeof(lockersFromFlash));
    return;
}

void incrementMasterCodeIndex (){
    uint8_t masterCodeIndex = FlashStorage.read(MASTER_CODE_INDEX_ADDRESS);
    masterCodeIndex++;
    uint8_t numberOfMasterCodes = sizeof(masterCodes) / sizeof(masterCodes[0]);
    masterCodeIndex = masterCodeIndex % numberOfMasterCodes;
    FlashStorage.write(MASTER_CODE_INDEX_ADDRESS, masterCodeIndex);
}

uint8_t readMasterCodeIndex(){
    uint8_t masterCodeIndex = FlashStorage.read(MASTER_CODE_INDEX_ADDRESS);
    return masterCodeIndex;
}

# endif
