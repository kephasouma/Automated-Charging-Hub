# ifndef Constants_h
# define Constants_h 
//TODO: inconsistent mix of const and define, perhaps best to only use const

const uint16_t lockerOpenTime = 10000; //TODO: ARBITRARY, SHOULD BE REVISED
const uint16_t setupDelay = 500; //TODO: ARBITRARY, SHOULD BE REVISED
const uint32_t lockerUseTime = 140 * 60; //Time allocated to user in seconds
const uint16_t keypadClearTimeout = 8000; //entryBuffer should be cleared after this time without new keypress
const uint8_t numberOfLockers = 4; //Hardcoded to 4 in code, must be edited if this is changed
const uint8_t accessKeyFullLength = 6; //length of access key entered by user, can be max 8 due to overflow, including lockerlockerTag digit 
const uint8_t pinLength = 4; //Length of user chosen pin.
const char entryBufferResetKey = '*';
const char pinSpacerKey = '#';
const uint8_t entryBufferSize = accessKeyFullLength + pinLength + 1; // +1 comes from pinSpacerKey
const uint8_t debounceTime = 100; //Keypad debounce config
const uint8_t lockerTagIndex = 5; //Index of lockerID in accessKey, for example index 5 -> accessKey == XXXXXY with Y ~= locker 
const uint16_t timeStampIncrement = 10*60; //Access key valid for timestamp with increment timeStampIncrement
const uint8_t masterCodeLen = 6; //Default 6

const uint8_t HASH_SIZE = ceil(accessKeyFullLength / 2); //TODO: weird def. should be fixed..

# define FIRST_TIME_SETUP_BYTE_ADDRESS 0 //Address of FIRST_TIME_SETUP_BYTE which is 0 if flash has been configured.
# define MASTER_CODE_INDEX_ADDRESS 4 //Address of master code index in flash
# define LOCKERS_ADDRESS 8 //Address of lockers struct in flash memory
# define HARDWARE_ID_LEN 12 
# define TIME_STAMP_LEN 10 //Unix time has 10 digits in decimal
# define LOCKER_ID_LEN 1 
# define HASH_DATA_LEN LOCKER_ID_LEN + TIME_STAMP_LEN + HARDWARE_ID_LEN


# endif
