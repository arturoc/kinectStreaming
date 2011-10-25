<?php
$page = $_REQUEST['page'];
$src = fopen("http://128.2.103.219/servers.txt",'rb');
$servers_str = stream_get_contents($src);
//$servers = file($src);
fclose($src);
$servers = explode("\n",$servers_str);
$server_num = rand(0,sizeof($servers)-3);
//echo $server_num . "/" . sizeof($servers) . ": " . $servers[$server_num] .$page;
header( 'Location: ' . $servers[$server_num] . $page ) ;
?>
