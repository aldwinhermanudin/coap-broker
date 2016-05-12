<?php
	$key = "Last Chance to See";
	$input = base64_decode("KiL2IfHUkEZE7H753kKICiG66O6qQqir2dIxItALEP4HeP5qVx6CIxYAGH6iWQe9fhhxeN6wNNs=");

	echo mcrypt_ecb (MCRYPT_3DES, $key, $input, MCRYPT_DECRYPT);
?>
