<?php
Ming_setScale(20.0);
ming_useswfversion(7);
$movie=new SWFMovie();
$movie->setRate(31);
$movie->setDimension(400,300);
$movie->setBackground(0xCC,0xCC,0xCC);


$filesrc1="jpg.jpg";
$filesrc2="png.png";
$filesrc3="gif.gif";

$filecontents1=file_get_contents($filesrc1);
$img = new SWFBitmap($filecontents1); 
$f1=$movie->add($img);
$f1->moveto(90,50);

$filecontents2=file_get_contents($filesrc2);
$img = new SWFBitmap($filecontents2); 
$f1=$movie->add($img);
$f1->moveto(10,10);

$filecontents3=file_get_contents($filesrc3);
$img = new SWFBitmap($filecontents3); 
$f1=$movie->add($img);
$f1->moveto(170,80);

header('Content-Type: application/x-shockwave-flash');
$movie->output(0);
?>
