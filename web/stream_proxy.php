<?php
header("Content-type: video/ogg");
$stream = "http://giss.tv:".$_REQUEST['port']."/".$_REQUEST['mount'];
$src = fopen($stream,'r');
while(!feof($src)){
        echo fread($src,8192);
}

