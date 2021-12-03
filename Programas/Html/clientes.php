<!DOCTYPE html>
<meta charset="utf-8">
<html>
<head>
<title>Dompi Cloud</title>
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2021">
<link href="css/dompicloud.css" rel="stylesheet" type="text/css" />
<script src="js/ajax.js" type="text/javascript"></script>
<script src="js/status.js" type="text/javascript"></script>
<script src="js/jquery.min.js" type="text/javascript"></script>
</head>
<body onload='LeerClientes();'>
<div class="div-main">

  <div class="div-head">
    <p class=menu-link onclick="window.location.replace('index.php');">&nbsp;&nbsp;<< Volver >>&nbsp;&nbsp;</p>
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
		table += list[i].System_Key;
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
</body>
</html>
