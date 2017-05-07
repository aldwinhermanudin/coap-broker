#!/bin/sh

server_addr="127.0.0.1"
path=".well-known/core"
method_type=get
subscribe_arg="-s 3600"
subscribe_keyword="lala"
delay_arg=1

if [ -z "$2" ]
  then
    echo "0.1 -- CoAP PubSub Request-er"
    echo "(c) 2017 Aldwin Hermanudin<aldwin@hermanudin.com>\n"
    echo "usage: $0 [-p path] [-h server-address][-m method-type] [-t subscribe-time] [-d delay-time] [-k subscribe-keyword]\n"
    echo "\t-p path\t\tCoAP resource path (default : .well-known/core)"
    echo "\t-h address\tCoAP server address (default: 127.0.0.1)"
    echo "\t-t time\t\tSubscribe time (default: 3600s)"
    echo "\t-k word\t\tSubscribe keyword (default: lala)"
    echo "\t-d time\t\tDelay time (default: 1s)"
    echo "\t-m type\t\trequest method type. get for GET Method. put for PUT method  (default: get)"
    echo "examples:"
    echo "\t$0 -p ps/room20/gas -h 192.168.1.9 \n"
    exit 1
fi

while getopts ":p:h:m:t:d:k:" opt; do
	case $opt in
		p)
			echo "Path\t\t\t: $OPTARG" >&2
			path=$OPTARG
			;;
		h)
			echo "Server address\t\t: $OPTARG" >&2
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
			echo "Subscribe time\t\t: $OPTARG" >&2
			subscribe_arg="-s $OPTARG"
			;;
		d)
			echo "Delay time\t\t: $OPTARG" >&2
			delay_arg="$OPTARG"
			;;
		k)
			echo "Subscribe Keyword\t\t: $OPTARG" >&2
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

echo "\n"

if [ "$method_type" = "get" ] ; then 
	method_arg="-m get"
fi

coap-client $method_arg $subscribe_arg -t 0 coap://[$server_addr]/$path | while read LOGLINE
do
	if [ "$LOGLINE" = "$subscribe_keyword" ] ; then 
		echo $subscribe_keyword | coap-client -N -m put -t 0 -f - coap://[$server_addr]/$path
		sleep $delay_arg
	fi 
done


exit 0

