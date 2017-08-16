<?php

$json = file_get_contents('http://localhost/tes/c-json.cgi');
$array = json_decode($json, true);
var_dump($array);
//echo "<br>";
//echo "array[sensor] = " . $array["sensor"] . "<br>";
?>
