#ifndef DATABASE_H
#define DATABASE_H

void logTimeForStateChange(unsigned long timestamp, char state, char PinNumber);

char getCurrentStateOfPinFromDb(char pinNumber);

void storeInputDataForPin(sqlite3 *db, char* tableName, char inputData, char count);

#endif
