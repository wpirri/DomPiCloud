<!DOCTYPE html>
<html lang="es">
<?php 
    function head_link(string $filename)
    {
        if( ($fm = filemtime($filename)) == false )
        {
            $fm = 0;
        }
        echo "<link href=\"".$filename."?time=".$fm."\" rel=\"stylesheet\" type=\"text/css\" />\n";
    }

    function head_script(string $filename)
    {
        if( ($fm = filemtime($filename)) == false )
        {
            $fm = 0;
        }
        echo "<script src=\"".$filename."?time=".$fm."\" type=\"text/javascript\"></script>\n";
    }

?>
<head>
<meta charset="utf-8">
<title><?php echo $TITLE; ?></title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
<?php head_link("../css/admin.css"); ?>
<?php head_script("../js/ajax.js"); ?>
<?php head_script("../js/jquery.min.js"); ?>
<?php head_script("../js/abm.js"); ?>
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

$auth = unserialize(openssl_decrypt(base64_decode($_SESSION['auth_token']), $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY));
/*$host = $_SERVER["SERVER_NAME"];*/
$host = "127.0.0.1";
$script = "/cgi-bin/dompi_cloud_auth.cgi";
$protocol = "https";

$url = $protocol."://".$host.$script;
$result = json_decode(httpPost($url, $auth));

$resp_code = $result->response->resp_code;
$resp_msg = $result->response->resp_msg;
$sistema = $result->response->sistema;

if(isset($ONLOAD))
{
    echo "<body onload='".$ONLOAD."'>";
}
else
{
    echo "<body>";
}

/* DEBUG -->
echo "<p>Auth (decript): ".$auth_token_decript."</p>";
echo "<p>User: ".$auth["User"]."</p>";
echo "<p>Pass: ".$auth["Password"]."</p>";
echo "<p>Time: ".$auth["Time"]."</p>";
<-- DEBUG */
?>
