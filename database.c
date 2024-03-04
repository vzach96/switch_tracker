#include <stdio.h>
#include <sqlite3.h>
#include "database.h"

void logTimeForStateChange(unsigned long timestamp, char state, char pinNumber)
{

}


char getCurrentStateOfPinFromDb(char pinNumber)
{
	//SELECT PIN_STATUS FOR ${TABLE_NAME} WHERE PIN_NUMBER = ${PIN_NUMBER} 
	return 0;
}


void storeInputDataForPin(sqlite3 *db, char* tableName, char inputData, char count)
{

}

