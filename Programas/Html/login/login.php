<!DOCTYPE html>
<html lang="en">
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
    <img src="../images/home.png" alt="Home" class="avatar">
  </div>

  <div class="container">
    <label for="uname"><b>Usuario</b></label>
    <input type="text" placeholder="Ingrese su nombre de usuario" name="uname" required>

    <label for="psw"><b>Clave</b></label>
    <input type="password" placeholder="Ingrese su clave de acceso" name="psw" required>

    <button type="submit">Login</button>
    <!--
    <label>
      <input type="checkbox" checked="checked" name="remember"> Recordar
    </label>
-->
  </div>
<!--
  <div class="container" style="background-color:#f1f1f1">
    <button type="button" class="cancelbtn">Cancelar</button>
    <span class="psw">Olvid&oacute; su <a href="#">clave de ingreso?</a></span>
  </div>
-->
</form>

</body>
</html>
