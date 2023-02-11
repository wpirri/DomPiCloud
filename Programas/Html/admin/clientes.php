<?php
$TITLE = "Dompi Cloud - Admin - Clientes";
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
        /* ==================== CARGA AUTORIZADA ==================== */
        ?>
        <body onload='LeerClientes();'>
        <div class="div-main">

          <div class="div-head">
            <p class=menu-link onclick="window.location.replace('../index.php');">&nbsp;&nbsp;<< Volver >>&nbsp;&nbsp;</p>
          </div>

          <div class="div-content">
            <div class="div-client-list" id='client_list' name='client_list'>&nbsp;</div>
            <div class="div-title" id='client_list_title' name='client_list_title'>&nbsp;</div>
            <div class="div-status-list" id='status_list' name='status_list'>&nbsp;</div>
          </div>
          
          <div class="div-foot">
            &nbsp;&nbsp;Pie de pagina
          </div>

        </div>

        </body>
        <script type="text/javascript" >
          var sel_cliente = '';

          function CargarClientes(msg)
          {
            list = JSON.parse(msg).response
            table = '<table>\n';
            for (i = 0; i < list.length; i++) { 
              table += '<tr><td class=client-list onclick="LeerStatus(\'';
              table += list[i].System_Key;
              table += '\');">'
              if(list[i].System_Key.length > 20)
              {
                table += list[i].System_Key.substring(0,20);
                table += '...';
              }
              else
              {
                table += list[i].System_Key;
              }
              table += '</td></tr>\n';
            }
            table += '</table>\n';
            document.getElementById('client_list').innerHTML = '<p class=client-list onclick=\'LeerClientes();\'>Clientes</p>' + table;
          }

          function CargarStatus(msg)
          {
            //fillStatusTable(JSON.parse(msg).response, 'status_list', 'Estados de: ' + sel_cliente)
            document.getElementById('client_list_title').innerHTML = '<p class=abm-table-title>&nbsp;Estados de ' + sel_cliente + '</p>';
            fillStatusChangeTable(JSON.parse(msg).response, 'status_list', 'Objeto', 'control_object')
          }

          function LeerClientes()
          {
            newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi', CargarClientes, false);
          }

          function LeerStatus(cli)
          {
            sel_cliente = cli;
            newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?System_Key=' + cli, CargarStatus, false);
          }

          function control_object_on(obj)
          {
            newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=on&System_Key=' + sel_cliente + '&Objeto=' + obj, null, false);
          }

          function control_object_off(obj)
          {
            newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=off&System_Key=' + sel_cliente + '&Objeto=' + obj, null, false);
          }

          function control_object_switch(obj)
          {
            newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=switch&System_Key=' + sel_cliente + '&Objeto=' + obj, null, false);
          }
        </script>
        <?php
        /* ==================== CARGA AUTORIZADA ==================== */
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

require('../foot.php');
?>
