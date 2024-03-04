#!/bin/bash

max_table_time=0;
max_table_name='';
for table in $(sqlite3 switches.db .tables);
do
	temp=$(sed -r 's/OPERATION_//g' <(echo "${table}"))
	if (( temp > max_table_time ));
	then
		max_table_time=${temp};
		max_table_name=${table};
	fi
done
echo "ID|PIN_NUMBER|PIN_INSTANCE_NAME|PIN_STATUS|STATE_CHANGE_TIME" > "${max_table_name}.csv";
sqlite3 switches.db "SELECT * from ${max_table_name};"  >> "${max_table_name}.csv";

exit 0;
