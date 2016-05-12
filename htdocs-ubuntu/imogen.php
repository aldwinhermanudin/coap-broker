<?php
$requestUri = $_SERVER["REQUEST_URI"];
$queryString = $_SERVER["QUERY_STRING"];

echo $requestUri . '<br>' . $queryString . '<br>';
?>
