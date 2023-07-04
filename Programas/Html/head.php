<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<title><?php echo $TITLE; ?></title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
<link href="../css/dompicloud.css" rel="stylesheet" type="text/css" />
<script src="../js/ajax.js" type="text/javascript"></script>
<script src="../js/status.js" type="text/javascript"></script>
<script src="../js/abm.js" type="text/javascript"></script>
<script src="../js/jquery.min.js" type="text/javascript"></script>
</head>
<?php
session_start();
require('../config.php');

function httpPost($url, $data)
{
    //$url = 'http://server.com/path';
    //$data = array('key1' => 'value1', 'key2' => 'value2');
    
    // use key 'http' even if you send the request to https://...
    $options = array(
        'http' => array(
            'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
            'method'  => 'POST',
            'content' => http_build_query($data)
        ),
        "ssl"=>array(
            "verify_peer" => false,
            "verify_peer_name" => false
        )
    );
    $context  = stream_context_create($options);
    $result = file_get_contents($url, false, $context);
    return $result;
}

?>