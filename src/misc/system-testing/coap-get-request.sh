#!/bin/bash

server_addr="127.0.0.1"
path=".well-known/core"
number_of_get=1
method_type=get
input_data="lalalililelo"
system_test=no

if [ -z "$2" ]
  then
    echo "0.1 -- CoAP Request-er"
    echo "(c) 2017 Aldwin Hermanudin<aldwin@hermanudin.com>\n"
    echo -e "usage: $0 [-p path] [-s node-list] [-h server-address] [-n number-of-get] [-o additional-option] [-m method-type] [-i input-data] [-T int:time:time:name]\n"
    echo -e  "\t-p path\t\t\t\tCoAP resource path (default : .well-known/core)"
    echo -e  "\t-s file\t\t\t\tnode-list for multiple node"
    echo -e  "\t-h address\t\t\tCoAP server address (default: 127.0.0.1)"
    echo -e  "\t-n number\t\t\tnumber of GET request (default: 1)"
    echo -e  "\t-o option\t\t\tadditional option. eta for --eta in GNU Parallel."
    echo -e  "\t-m type\t\t\t\trequest method type. get for GET Method. put for PUT method  (default: get)"
    echo -e  "\t-i text\t\t\t\tinput text for PUT Method. (default : lalalililelo) "
    echo -e  "\t-T int:int:time:name\t\tSequentially max request, interval, interval time, logfile name.\n"
    echo "examples:"
    echo -e  "\t$0 -p ps/room20/gas -s node-list -h 192.168.1.9 -n 10 -o eta\n"
    exit 1
fi

while getopts ":p:s:h:n:o:m:i:T:" opt; do
	case $opt in
		p)
			echo -e  "Path\t\t\t: $OPTARG" >&2
			path=$OPTARG
			;;
		s)
			echo -e  "Server list file\t: $OPTARG" >&2
			multiple_server=yes;
			server_list=$OPTARG 
			;;
		h)
			echo -e  "Server address\t\t: $OPTARG" >&2
			server_addr=$OPTARG 
			;;
		n)
			echo -e  "Total number of GET\t: $OPTARG" >&2
			number_of_get=$OPTARG 
			;;
		o)
			if [ "$OPTARG" = "eta" ] ; then
				echo "ETA Option Enabled"
				eta_enable=yes
			
			else
				echo "Invalid option: -$OPTARG" >&2
				exit 1
			fi
			;;
		
		m)
			if [ "$OPTARG" = "get" ] ; then
				echo "Using GET Method"
				method_type=get
			
			
			elif [ "$OPTARG" = "put" ] ; then
				echo "Using PUT Method"
				method_type=put			
			
			else
				echo "Invalid option: -$OPTARG" >&2
				exit 1
			fi
			;;
		i)
			echo -e  "Input data\t\t: $OPTARG" >&2
			input_data=$OPTARG 
			;;
		T)			
			if [[ $OPTARG == *":"* ]]; then
				
				IFS=':' read -a myarray <<< "$OPTARG"

				if [ -z ${myarray[0]} ] || [ -z ${myarray[1]} ] || [ -z ${myarray[2]} ] || [ -z ${myarray[3]} ]
				then
					echo "Invalid option: -$OPTARG" >&2
					exit 1
				else
					  				
					max_request=${myarray[0]}
					interval=${myarray[1]}
					interval_time=${myarray[2]}
					logfile=${myarray[3]}
					system_test=yes
				fi
			else
				echo "Invalid option: -$OPTARG" >&2
				exit 1
			fi
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit 1
			;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1
			;;
	esac
done

echo ""

if [ "$multiple_server" = "yes" ]  ; then
	
	  multiple_node_arg="--sshloginfile $server_list"
fi

if [ "$eta_enable" = "yes" ]  ; then
	
	  eta_arg="--eta"
fi

if [ "$method_type" = "get" ] ; then 
	method_arg="-m get"
	
elif [ "$method_type" = "put" ] ; then 
	method_arg="-m put -e $input_data"		

fi

if [ "$system_test" = "yes" ]  ; then
  
	for (( c=0; c<=$max_request; c+=$interval ))
	do  
		if [ "$c" -ne 0 ]  ; then
	
			date_deadline=$(( $(date "+%s") + interval_time ))
			loop_counter=0
			echo | tee -a $logfile
			echo "Execute at `date`" | tee -a $logfile
			echo "Running $c Request for $interval_time seconds" | tee -a $logfile
			while [ $(date "+%s") -lt $date_deadline ]; do
				#echo "Running GNU Parallel for $c request" | tee -a $logfile
				((loop_counter++))
				delay_wo_sleep=$(( $(date "+%s") + 1 ))
				seq $c | parallel $eta_arg $multiple_node_arg -n0 coap-client $method_arg -t 0 coap://[$server_addr]/$path\;echo "Running on \`hostname\`" 
				
				#delay without sleep
				while [ $(date "+%s") -lt $delay_wo_sleep ]; do
					:
				done
			done
			echo "GNU Parallel do $c request for $loop_counter times" | tee -a $logfile
			echo "Terminate at `date`" | tee -a $logfile
		fi
	done
else
	seq $number_of_get | parallel $eta_arg $multiple_node_arg -n0 coap-client $method_arg -t 0 coap://[$server_addr]/$path\;echo "Running on \`hostname\`"
fi

exit 0

#echo all processes complete
