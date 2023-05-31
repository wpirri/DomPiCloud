<?php
$TITLE = "DomPi Cloud Login";
require('../head.php');

$_SESSION['auth_token'] = "";

$auth["User"] = htmlspecialchars(trim($_POST["uname"]), ENT_QUOTES);
$auth["Password"] = htmlspecialchars(trim($_POST["psw"]), ENT_QUOTES);
$auth["Remember"] = htmlspecialchars(trim($_POST["remember"]), ENT_QUOTES);
$auth["Time"] = time();
/*$host = $_SERVER["SERVER_NAME"];*/
$host = "127.0.0.1";
$script = "/cgi-bin/dompi_cloud_auth.cgi";
/*
if(isset($_SERVER["HTTPS"]))
{
    $protocol = "https";
}
else
{
    $protocol = "http";
}
*/
$protocol = "http";
$url = $protocol."://".$host.$script;
$result = json_decode(httpPost($url, $auth));

$resp_code = $result->response->resp_code;
$resp_msg = $result->response->resp_msg;
$sistema = $result->response->sistema;

?>
<body>

<?php
/* DEBUG -->
echo "<p> print_r: ".print_r($result)."</p>";
echo "<p>resp_code: ".$result->response->resp_code."</p>";
echo "<p>resp_msg: ".$result->response->resp_msg."</p>";
echo "<p>sistema: ".$result->response->sistema."</p>";
$auth_token = serialize($auth);
echo "<p>Auth: ".$auth_token."</p>";
$auth_token_cript = openssl_encrypt($auth_token, $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY);
echo "<p>Auth (cript): ".$auth_token_cript."</p>";
$auth_token_decript = openssl_decrypt($auth_token_cript, $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY);
echo "<p>Auth (decript): ".$auth_token_decript."</p>";
$auth_decript = unserialize($auth_token_decript);
echo "<p>User: ".$auth_decript["user"]."</p>";
echo "<p>Pass: ".$auth_decript["pass"]."</p>";
echo "<p>Time: ".$auth_decript["time"]."</p>";
<-- DEBUG */
?>
</body>

<script type="text/javascript" >
<?php
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0)
    {
        $auth_token = serialize($auth);
        $auth_token_cript = openssl_encrypt($auth_token, $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY);

        if( $auth["Remember"] == "on" )
        {
            echo "localStorage.setItem('auth_token', '".base64_encode($auth_token_cript)."');";
        }
        else
        {
            echo "localStorage.setItem('auth_token', '');";
        }

        $_SESSION['auth_token'] = $auth_token_cript;
        if($sistema == 'ADMIN')
        {
            echo "window.location.replace('../admin/index.php');";
        }
        else
        {
            echo "window.location.replace('../m/index.php?sistema=".$sistema."');";
        }
    }
    else
    {
        echo "localStorage.setItem('auth_token', '');";
        echo "window.location.replace('../index.php?msg=Error');";
    }
}
else
{
    echo "localStorage.setItem('auth_token', '');";
    echo "window.location.replace('../index.php?msg=Error');";
}
?>
</script>
<?php
require('../foot.php');
?>
