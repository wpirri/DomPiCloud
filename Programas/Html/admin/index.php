<?php
$TITLE = "Dompi Cloud - Admin";
require('../head.php');
$auth_token_cript = $_SESSION['auth_token'];
$auth_token_decript = openssl_decrypt($auth_token_cript, $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY);
$auth = unserialize($auth_token_decript);
$host = $_SERVER["SERVER_NAME"];
$script = "/cgi-bin/auth.cgi";
if(isset($_SERVER["HTTPS"]))
{
    $protocol = "https";
}
else
{
    $protocol = "http";
}

$url = $protocol."://".$host.$script;
$result = json_decode(httpPost($url, $auth));

$resp_code = $result->response->resp_code;
$resp_msg = $result->response->resp_msg;
$sistema = $result->response->resp_code;
?>

<body>

<?php
/* DEBUG -->
echo "<p>Auth (decript): ".$auth_token_decript."</p>";
echo "<p>User: ".$auth["User"]."</p>";
echo "<p>Pass: ".$auth["Password"]."</p>";
echo "<p>Time: ".$auth["Time"]."</p>";
<-- DEBUG */
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 0)
    {
        ?>
        <h1 align="center">DomPi Cloud</h1>
        <table>
        <tr>
            <td class=menu-link onclick="window.location.replace('');">&nbsp;&nbsp;<< Configuraciones >></td>
        </tr>
        <tr>
            <td class=menu-link onclick="window.location.replace('clientes.php');">&nbsp;&nbsp;<< Listado de Clientes >></td>
        </tr>
        </table>
        <?php
    }
    else
    {
        ?>
        <script type="text/javascript" >
        window.location.replace('login.php?msg=Error');
        </script>
        <?php
    }
}
else
{
    ?>
    <script type="text/javascript" >
    window.location.replace('login.php?msg=Error');
    </script>
    <?php
}
?>
</body>
<script type="text/javascript" >


</script>
<?php
require('../foot.php');
?>
