#!/bin/sh

#sleep for 10 minute
#sleep 10m

#get program location from argument
program_location="/opt/ngrok"
program_name="ngrok"
full_program_location="$program_location/$program_name"

process_count=$(pgrep -c $program_name)	

if [ "$process_count" -gt 0 ]
then
	echo "Killing all $program_name $process_count process"
    kill $(pgrep $program_name)
fi
