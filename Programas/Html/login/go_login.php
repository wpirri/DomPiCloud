<?php
$TITLE = "DomPi Cloud Login";
require('../head.php');

//syslog(LOG_DEBUG, "go_login.php");

/* Armado de un nuevo auth_token */
if( isset($_POST["uname"]) ) { $auth["User"] = htmlspecialchars(trim($_POST["uname"]), ENT_QUOTES); }
else { $auth["User"] = ""; }
if( isset($_POST["psw"]) ) { $auth["Password"] = htmlspecialchars(trim($_POST["psw"]), ENT_QUOTES); }
else { $auth["Password"] = ""; }
if( isset($_POST["remember"]) ) { $auth["Remember"] = htmlspecialchars(trim($_POST["remember"]), ENT_QUOTES); }
else { $auth["Remember"] = ""; }

//syslog(LOG_DEBUG, "FORM: ".$auth["User"]."/".$auth["Password"]."/".$auth["Remember"]);

if( !empty($_SERVER['HTTP_CLIENT_IP']) ) { $auth["ClientIP"] = $_SERVER['HTTP_CLIENT_IP']; }
elseif( !empty($_SERVER['HTTP_X_FORWARDED_FOR']) ) { $auth["ClientIP"] = $_SERVER['HTTP_X_FORWARDED_FOR']; }
else { $auth["ClientIP"] = $_SERVER['REMOTE_ADDR']; }

$auth["ClientBrowser"] = $_SERVER['HTTP_USER_AGENT'];
$auth["Time"] = time();

/* Si viene con info de auth guardada la valido */
if( isset($_SESSION['auth_token']) )
{
    //syslog(LOG_DEBUG, "auth_token en SESSION"); 
    $saved_auth = unserialize(openssl_decrypt(base64_decode($_SESSION['auth_token']), $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY));
    if( isset($saved_auth) )
    {
        //syslog(LOG_DEBUG, "SESSION User:    ".$saved_auth["User"]); 
        //syslog(LOG_DEBUG, "SESSION Pass:    ".$saved_auth["Password"]); 
        //syslog(LOG_DEBUG, "SESSION Browser: ".$saved_auth["ClientBrowser"]); 
        if( $auth["ClientBrowser"] == $saved_auth["ClientBrowser"] &&
            $auth["User"] == $saved_auth["User"] &&
            $auth["Password"] == "**********" )
        {
            /* Si el token salvado es válido sobreescribo el del formulario */
            //syslog(LOG_DEBUG, "Usando auth_token de SESSION"); 
            //$auth["User"] = $saved_auth["User"];
            $auth["Password"] = $saved_auth["Password"];
        }
    }
}
/* Autenticacion */
//syslog(LOG_DEBUG, "AUTH: ".$auth["User"]."/".$auth["Password"]);
/*$host = $_SERVER["SERVER_NAME"];*/
$host = "127.0.0.1";
$script = "/cgi-bin/dompi_cloud_auth.cgi";
$protocol = $_SERVER['REQUEST_SCHEME'];
$url = $protocol."://".$host.$script;
$result = json_decode(httpPost($url, $auth));
//syslog(LOG_DEBUG, "AUTH: resp ".$result->response->resp_code);
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
if(isset($result->response->resp_code) && isset($result->response->resp_msg) && isset($result->response->sistema))
{
    $resp_code = $result->response->resp_code;
    $resp_msg = $result->response->resp_msg;
    $sistema = $result->response->sistema;

    if($resp_code == 0)
    {
        $auth_token_cript = base64_encode(openssl_encrypt(serialize($auth), $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY));

        if( $auth["Remember"] == "on" )
        {
            echo "localStorage.setItem('auth_token', '".$auth_token_cript."');";
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
