<?php
$TITLE = "Dompi Cloud App";
$ONLOAD = "InitUpdate();";
require('head_m.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema != 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->
<?php
    $grupo = $_GET['grupo'];
?>

<div class="desktop-group" id="desktop">

<div class="scroll-list" id="event-list">
&nbsp;
</div>

<div class="list-back-btn" id="movil-return" onclick="window.location.replace('index.php');">
	<img id="back-icon" class="icon-image" src="../images/back.png">&nbsp;Volver
</div>

</div>

<script type="text/javascript">

// Parses the xmlResponse from status.xml and updates the status box
function LoadData(msg) {
    // Armo el listado de objetos
	var i = 0;
	var json_list = JSON.parse(msg).response;
    var output = '';
    var filename = '';

    if(!json_list) return;

    // Cabecera
    <?php if($grupo == 1) { ?>
    output += '<div class="list-head" id="list-head1"  >\n';
	output += '<img id="icon-image1" class="icon-image" src="../images/lock.png" >&nbsp;Alarma\n';
    output += '</div>\n';
    <?php } else if($grupo == 2) { ?>
    output += '<div class="list-head" id="list-head2" >\n';
	output += '<img id="icon-image2" class="icon-image" src="../images/lamp1.png" >&nbsp;Luces\n';
    output += '</div>\n';
    <?php } else if($grupo == 3) { ?>
    output += '<div class="list-head"  id="list-head3" >\n';
	output += '<img id="icon-image3" class="icon-image" src="../images/door1.png" >&nbsp;Puertas\n';
    output += '</div>\n';
    <?php } else if($grupo == 4) { ?>
    output += '<div class="list-head"  id="list-head4" >\n';
	output += '<img id="icon-image4" class="icon-image" src="../images/calef1.png" >&nbsp;Clima\n';
    output += '</div>\n';
    <?php } else if($grupo == 5) { ?>
    output += '<div class="list-head"  id="list-head5" >\n';
	output += '<img id="icon-image5" class="icon-image" src="../images/camara.png" >&nbsp;C&aacute;maras\n';
    output += '</div>\n';
    <?php } ?>

	for (i = 0; i < json_list.length; i++) {
        if(json_list[i].Estado == 0)
        {
            filename = json_list[i].Icono0;
        }
        else
        {
            filename = json_list[i].Icono1;
        }

        output += '<div class="list-btn-group<?php echo $grupo; ?>" id="list-btn' + i + '" onClick="ChangeStatus(\'<?php echo $sistema; ?>\', \'' + json_list[i].Objeto + '\');">\n';
        output += '<img id="boton' + i + '" class="icon-image" src="../images/' + filename + '">&nbsp;' + json_list[i].Objeto + '\n';
        output += '</div>\n';
	}    

    document.getElementById('event-list').innerHTML = output;

    newAJAXCommand('/cgi-bin/dompi_cloud_mobile.cgi?funcion=list&sistema=<?php echo $sistema; ?>&grupo=<?php echo $grupo; ?>', UpdateStatus, true);
}

function UpdateStatus(msg) {
    // actualizo el estado de los objetos
	var i = 0;
	var json_list = JSON.parse(msg).response;
    var filename = '';

    if(!json_list) return;

	for (i = 0; i < json_list.length; i++) {
        if(json_list[i].Estado == 0)
        {
            filename = json_list[i].Icono0;
        }
        else
        {
            filename = json_list[i].Icono1;
        }
        document.getElementById('boton' + i).src = '../images/' + filename;
    }

}

function InitUpdate() {
    newAJAXCommand('/cgi-bin/dompi_cloud_mobile.cgi?funcion=list&sistema=<?php echo $sistema; ?>&grupo=<?php echo $grupo; ?>', LoadData, false);
}

function ChangeStatus(sistema, objeto) {
    newAJAXCommand('/cgi-bin/dompi_cloud_mobile.cgi?funcion=touch&sistema=' + sistema + '&objeto=' + objeto, null, false);
}
</script>

<!-- ==================== CARGA AUTORIZADA ==================== -->
<?php
    }
    else
    {
        ?>
        <script type="text/javascript" >
        window.location.replace('../index.php?msg=Error');
        </script>
        <?php
    }
}
else
{
    ?>
    <script type="text/javascript" >
    window.location.replace('../index.php?msg=Error');
    </script>
    <?php
}
require('foot_m.php');
?>
