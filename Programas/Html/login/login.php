<!doctype html>
<?php
require('../config.php');

if( isset($_GET["auth_token"]) )
{
  $auth_token_cript = base64_decode($_GET["auth_token"]);
  $auth_token_decript = openssl_decrypt($auth_token_cript, $ALGO_KEY, $TOKEN_KEY, 0, $IV_KEY);
  $auth = unserialize($auth_token_decript);
  if( $auth["Remember"] != "on" )
  {
    $auth["User"] = "";
    $auth["Password"] = "";
    $auth["Remember"] = "";
    $auth["Time"] = "";
  }
  ?>

  <html>
  <head>
  <meta charset="utf-8">
  <title>DomPiCloud Login</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="author" content="Walter Pirri" >
  <meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
  <meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
  <meta name="system-build" content="2023">
  <link href="../css/login.css" rel="stylesheet" type="text/css" />
  </head>
  <body>
  <form action="go_login.php" method="post">
    <div class="imgcontainer">
      <img src="../images/home.png" class="avatar">
    </div>
  
    <div class="container">
      <label for="uname"><b>Usuario</b></label>
      <input type="text" placeholder="Ingrese su nombre de usuario" name="uname" value="<?php echo $auth["User"] ?>" required />
  
      <label for="psw"><b>Clave</b></label>
      <input type="password" placeholder="Ingrese su clave de acceso" name="psw" value="<?php echo $auth["Password"] ?>" required />
  
      <button type="submit">Ingresar</button>
      <label>
        <input type="checkbox" name="remember" <?php if( $auth["Remember"] == "on" ) echo "checked"; ?> > Recordarme
      </label>
    </div>
  </form>
  </body>
  </html>
  
  <?php
}
else
{
  ?>

  <script type="text/javascript" >
  var auth_token = localStorage.getItem('auth_token');
  window.location.replace('?auth_token=' + auth_token);
  </script>

  <?php
}

?>
