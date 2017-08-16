#!/bin/bash

server_addr="127.0.0.1"
path=".well-known/core"
method_type=get
subscribe_time="3600"
subscribe_keyword="lala"
delay_arg=1

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c() {
        echo "** Trapped CTRL-C"
        pkill -P $$ aiocoap-client
}


if [ -z "$2" ]
  then
    echo "0.1 -- CoAP PubSub Request-er"
    echo "(c) 2017 Aldwin Hermanudin<aldwin@hermanudin.com>\n"
    echo "usage: $0 [-p path] [-h server-address][-m method-type] [-t subscribe-time] [-d delay-time] [-k subscribe-keyword]\n"
    echo -e "\t-p path\t\tCoAP resource path (default : .well-known/core)"
    echo -e "\t-h address\tCoAP server address (default: 127.0.0.1)"
    echo -e "\t-t time\t\tSubscribe time (default: 3600s)"
    echo -e  "\t-k word\t\tSubscribe keyword (default: lala)"
    echo -e "\t-d time\t\tDelay time (default: 1s)"
    echo -e "\t-m type\t\trequest method type. get for GET Method. put for PUT method  (default: get)"
    echo -e "examples:"
    echo -e "\t$0 -p ps/room20/gas -h 192.168.1.9 \n"
    exit 1
fi

while getopts ":p:h:m:t:d:k:" opt; do
	case $opt in
		p)
			echo -e "Path\t\t\t: $OPTARG" >&2
			path=$OPTARG
			;;
		h)
			echo -e "Server address\t\t: $OPTARG" >&2
			server_addr=$OPTARG 
			;;
		m)
			if [ "$OPTARG" = "get" ] ; then
				echo "Using GET Method"
				method_type=get			
			else
				echo "Invalid option: -$OPTARG" >&2
				exit 1
			fi
			;;
		t)
			echo -e "Subscribe time\t\t: $OPTARG" >&2
			subscribe_time="$OPTARG"
			;;
		d)
			echo -e "Delay time\t\t: $OPTARG" >&2
			delay_arg="$OPTARG"
			;;
		k)
			echo -e "Subscribe Keyword\t\t: $OPTARG" >&2
			subscribe_keyword="$OPTARG"
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

echo 

if [ "$method_type" = "get" ] ; then 
	method_arg="-m get"
fi

date_deadline=$(( $(date "+%s") + $subscribe_time ))


#coap-client $method_arg $subscribe_arg -t 0 coap://[$server_addr]/$path | while read LOGLINE

echo $subscribe_keyword | coap-client -N -m put -t 0 -f - coap://[$server_addr]/$path
aiocoap-client -m get -q --observe --content-format 0 coap://[$server_addr]/$path | while read LOGLINE
do
	if [ "$LOGLINE" = "$subscribe_keyword" ] ; then 
		
		echo $subscribe_keyword | coap-client -m put -t 0 -f - coap://[$server_addr]/$path
		sleep $delay_arg
	fi 
	
	if [ $(date "+%s") -ge $date_deadline ]; then
		pkill -P $$ aiocoap-client
		exit 0
	fi
done


exit 0

