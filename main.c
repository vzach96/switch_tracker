#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "common.h"
#include "database.h"

static volatile char KEEP_RUNNING = 1;

#define NUMBER_OF_MUX_INPUTS 64
#define MAX_NAME_LENGTH 64
#define DELIMITER ','

void intHandler(int dummy)
{
	KEEP_RUNNING = 0;
}

static int callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	//fprintf(stderr, "%s\n", (const char*)data);
	for (i = 0; i < argc; i++)
	{
		if (strcmp(azColName[i], "PIN_STATUS") && argv[i])
		{
			int val = atoi(argv[i]);
			printf("PIN_STATUS: %d\n", val);
		}
		else
			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (getuid())
	{
		fprintf(stderr, "Application must be run as root!\n");
		exit(1);
	}	

	FILE *file_ptr;
	char name_mapping[MAX_NAME_LENGTH][NUMBER_OF_MUX_INPUTS];
	char *mapping_file = argv[1];
	file_ptr = fopen(mapping_file, "r");

	if (!file_ptr)
	{
		printf("File %s cannot be opened.\n", mapping_file);
		return 1;
	}

	unsigned line_number = 0;
	char c;
	char record = 0;
	unsigned index = 0;
	do 
	{
		c = getc(file_ptr);
		switch (c)
		{
			case '\n':
				name_mapping[line_number][index] = 0;
				record = 0; 
				line_number++;
				index = 0;
				break;
			case DELIMITER:
				record = 1;	      
				break;
			default:
				if (record)
				{
					if (c > 31 && c < 126) 
					{
						name_mapping[line_number][index] = c;
						index++;
					}
				}
				break;
		}
		
	} while (line_number < NUMBER_OF_MUX_INPUTS && c != EOF);

	fclose(file_ptr);

	printf("Use CTRL+C to terminate Operation.\n");
	signal(SIGINT, intHandler);
	int retc = initGpio();
	if (retc < 0)
	{
		fprintf(stderr, "GPIO Failed to initialize.\n");
		exit(1);
	}
	sqlite3 *db;
	char *zErrMsg = 0;
	
	int sqlrc = sqlite3_open("switches.db", &db);

	if (sqlrc)
	{
		fprintf(stderr, "Can't open database %s\n", sqlite3_errmsg(db));
		return(1);
	}
	else
	{
		fprintf(stderr, "Opened database successfully\n");
	}

	
	char table_name[64];
	sprintf(table_name, "OPERATION_%lu", time(NULL));
	char create_table_sql[256];
	sprintf(create_table_sql, "CREATE TABLE %s(" \
			    "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
			    "PIN_NUMBER INTEGER NOT NULL," \
			    "PIN_INSTANCE_NAME TEXT," \
			    "PIN_STATUS INTEGER NOT NULL," \
			    "STATE_CHANGE_TIME INTEGER NOT NULL);", table_name);

	printf("Name of table: %s\n", table_name);
	
	sqlrc = sqlite3_exec(db, create_table_sql, callback, 0, &zErrMsg);

	if (sqlrc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		fprintf(stdout, "Table created successfully\n");
	}
	
	clock_t number_of_ticks_in_2_microseconds = CLOCKS_PER_SEC * 2 * pow(10,-6);
	unsigned count = 0;
	setCountToOutputPins(count);
	clock_t start_t = clock();	


	while(KEEP_RUNNING)
	{
		if ((clock() - start_t) > number_of_ticks_in_2_microseconds)
		{
			//printf("Pin number: %d\n",count);
			int pin_reading = readInputPins();	
			int db_pin_reading = 0;
			storeInputDataForPin(db, table_name, pin_reading, count);

			//read DB for pin to see current PIN_STATUS
			char read_pin_sql[256];
			const char* last_db_reading_for_pin = "CALLBACK CALLED";
			sqlite3_stmt *stmt;
			sprintf(read_pin_sql, "SELECT PIN_STATUS FROM %s WHERE PIN_NUMBER=%d ORDER BY ID DESC LIMIT 1;",
					table_name, count);


			if (sqlite3_prepare_v2(db, read_pin_sql, -1, &stmt, NULL))
			{
				printf("FAILED TO PREPARE STATEMENT: %s\n", sqlite3_errmsg(db));
				return -1;
			}

			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				db_pin_reading = sqlite3_column_int(stmt, 0);
			}

			//printf("Pin Status: %d\n", db_pin_reading);
			//printf("GPIO pin reading: %d\n", pin_reading);

			char pin_row_exists_query[256];
			int pin_row_exists = 0;
			sqlite3_stmt *exists_stmt;
			sprintf(pin_row_exists_query, "SELECT EXISTS(SELECT 1 FROM %s WHERE PIN_NUMBER=%d LIMIT 1);", 
					table_name, count);
			if (sqlite3_prepare_v2(db, pin_row_exists_query, -1, &exists_stmt, NULL))
			{
				printf("FAILED TO PREPARE STATEMENT: %s\n", sqlite3_errmsg(db));
				return -1;
			}
			if(sqlite3_step(exists_stmt) == SQLITE_ROW)
			{
				pin_row_exists = sqlite3_column_int(exists_stmt, 0);
			}


			if (pin_reading != db_pin_reading || !pin_row_exists)
			{
			
				printf("Pin number: %d\n",count);
				printf("Prev Pin Status: %d\n", db_pin_reading);
				printf("Current GPIO pin reading: %d\n", pin_reading);
				char pin_name[NUMBER_OF_MUX_INPUTS];
				sprintf(pin_name, "%s", name_mapping[(unsigned)count]);
				char insert_sql[256];
				sprintf(insert_sql, "INSERT INTO %s (PIN_NUMBER,PIN_INSTANCE_NAME,PIN_STATUS,STATE_CHANGE_TIME)"\
					"VALUES (%d,'%s',%d,%ld);", table_name, count, pin_name, pin_reading, time(NULL));
				sqlrc = sqlite3_exec(db, insert_sql, callback, (void*)last_db_reading_for_pin, &zErrMsg);
				if (sqlrc != SQLITE_OK)
				{
					fprintf(stderr, "SQL error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				}
			}

			count = (count + 1) & (NUMBER_OF_MUX_INPUTS - 1);
			setCountToOutputPins(count);
			start_t = clock();
		}
	}
	printf("\nKeyboard Interupt recieved, Operation Terminating\n");

	sqlite3_close(db);
	return retc;
}
