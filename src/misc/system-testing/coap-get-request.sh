#!/bin/sh

server_addr="127.0.0.1"
path=".well-known/core"
number_of_get=1
method_type=get
input_data="lalalililelo"

if [ -z "$2" ]
  then
    echo "0.1 -- CoAP Request-er"
    echo "(c) 2017 Aldwin Hermanudin<aldwin@hermanudin.com>\n"
    echo "usage: $0 [-p path] [-s node-list] [-h server-address] [-n number-of-get] [-o additional-option] [-m method-type] [-i input-data]\n"
    echo "\t-p path\t\tCoAP resource path (default : .well-known/core)"
    echo "\t-s file\t\tnode-list for multiple node"
    echo "\t-h address\tCoAP server address (default: 127.0.0.1)"
    echo "\t-n number\tnumber of GET request (default: 1)"
    echo "\t-o option\tadditional option. eta for --eta in GNU Parallel."
    echo "\t-m type\t\trequest method type. get for GET Method. put for PUT method  (default: get)"
    echo "\t-i text\t\tinput text for PUT Method. (default : lalalililelo) \n"
    echo "examples:"
    echo "\t$0 -p ps/room20/gas -s node-list -h 192.168.1.9 -n 10 -o eta\n"
    exit 1
fi

while getopts ":p:s:h:n:o:m:i:" opt; do
	case $opt in
		p)
			echo "Path\t\t\t: $OPTARG" >&2
			path=$OPTARG
			;;
		s)
			echo "Server list file\t: $OPTARG" >&2
			multiple_server=yes;
			server_list=$OPTARG 
			;;
		h)
			echo "Server address\t\t: $OPTARG" >&2
			server_addr=$OPTARG 
			;;
		n)
			echo "Total number of GET\t: $OPTARG" >&2
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
			echo "Input data\t\t: $OPTARG" >&2
			input_data=$OPTARG 
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

seq $number_of_get | parallel $eta_arg $multiple_node_arg -n0 coap-client $method_arg -t 0 coap://[$server_addr]/$path\;echo "Running on \`hostname\`"

exit 0

#echo all processes complete
