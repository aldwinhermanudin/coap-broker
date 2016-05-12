<?php
	echo "aaa";
	$output = shell_exec('./client-tes -m get coap://[::1]/sensor');
	echo "<pre>$output</pre>";
	$res = substr($output, 6);
	echo "<pre>$res</pre>";
	$pieces = explode("-", $res);
	echo "1. ". $pieces[0] . "<br>";
	echo "1. ". $pieces[1] . "<br>";
	echo $_SERVER["QUERY_STRING"];
	//echo exec("./coap-client -m get coap://[::1]/sensor");
?>
