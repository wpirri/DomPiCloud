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



<div class="scroll-list" id="event-list">

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
	var j = 0;
	var json_list = JSON.parse(msg).response;
    var output = '';
    var filename = '';

    // Cabecera
    output += '<div class="list-head<?php echo $grupo; ?>" >\n';
	output += '<img id="icon-image' + i + '" class="icon-image" src="../images/lamp1.png" >&nbsp;Luces\n';
    output += '</div>\n';

	for (i = 0; i < json_list.length; i++) {
        if(json_list[i].Estado == 0)
        {
            filename = json_list[i].Icono0;
        }
        else
        {
            filename = json_list[i].Icono1;
        }

        output += '<div class="list-btn" id="luz-btn' + i + '" onClick="newAJAXCommand(\'touch.cgi?obj=salida1\');">\n';
        output += '<img id="salida1" class="icon-image" src="../images/' + filename + '">&nbsp;' + json_list[i].Objeto + '\n';
        output += '</div>\n';
	}    

    document.getElementById('event-list').innerHTML = output;

    newAJAXCommand('/cgi-bin/dompi_cloud_mobile.cgi?funcion=list&sistema=<?php echo $sistema; ?>&grupo=<?php echo $grupo; ?>', UpdateStatus, true);
}

function UpdateStatus(msg) {
    // actualizo el estado de los objetos
	
}

function InitUpdate() {
    newAJAXCommand('/cgi-bin/dompi_cloud_mobile.cgi?funcion=list&sistema=<?php echo $sistema; ?>&grupo=<?php echo $grupo; ?>', LoadData, false);
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
