/*
 *-----------INSTRUCTIONS FOR CONFIG OF STATION---------------
 * 
 *  THESE INSTRUCTIONS MAY BE OUTDATED  TODO
 * 
 *  The lockers below need to be configured correctly. 
 *  Format is: {int pin, String ID, String password, unsigned long passwordTiemout}
 *  Correct pin (as connected to relay) must be assigned.
 *  Correct ID (station+locker, 7 digits in format "A001001") needs to be assigned. The ID needs to be assigned so that it is in sync with server!! IMPORTANT!
 *  Rest should be set to empty String, 0, false
 * 
*/
# ifndef StationConfig_h
# define StationConfig_h

const char hardwareID[HARDWARE_ID_LEN+1] = "A00599999999"; //Should start with stationID, such as A001

/* Master codes must be 6 digits 0-D (supported by keypad, and not including special chars). Entering *lockerID* + *spacer* +*spacer* + *master code* opens locker.*/
const char* masterCodes[] = {"A12B67", "358C2B"};

# endif
