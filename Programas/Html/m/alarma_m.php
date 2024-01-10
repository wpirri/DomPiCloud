<?php
$TITLE='Alarma'; 
include("head_m.php");
?>

<?php
    $part = $_GET['part'];
?>

<body onload="OnLoad()">

<div class="desktop-group" id="desktop">

<div class="scroll-list" id="event-list">
&nbsp;
</div>

<div class="list-back-btn" id="movil-return" onclick="window.location.replace('particiones_m.php');">
	<img id="back-icon" class="icon-image" src="../images/back.png">&nbsp;Volver
</div>

</div>

<script type="text/javascript">

// Parses the xmlResponse from status.xml and updates the status box
function LoadData(msg) {
    try {
        // Armo el listado de objetos
        var i = 0;
        var part = JSON.parse(msg).response;
        if(!part) return;
        var zonas = part.Zonas;
        var salidas = part.Salidas;
        var output = '';

        // Cabecera
        if(document.getElementById('icon-alarma'))
        {
            if(part.Estado_Activacion > 0) {
                document.getElementById('icon-alarma').src = '../images/lock1.png';
            } else {
                document.getElementById('icon-alarma').src = '../images/lock0.png';
            }
            
            for (i = 0; i < zonas.length; i++) {
                if(zonas[i].Activa > 0) {
                    if (zonas[i].Estado > 0) {
                        document.getElementById('icon-zona' + i).src = '../images/led_rojo1.png';
                    } else {
                        document.getElementById('icon-zona' + i).src = '../images/led_rojo0.png';
                    }
                } else {
                    document.getElementById('icon-zona' + i).src = '../images/no.png';
                }
            }    

            for (i = 0; i < salidas.length; i++) {
                if (salidas[i].Estado > 0) {
                    document.getElementById('icon-salida' + i).src = '../images/sirena_mini1.png';
                } else {
                    document.getElementById('icon-salida' + i).src = '../images/sirena_mini0.png';
                }
            }    

        }
        else
        {
            output += '<div class="list-head" id="list-head1" onClick="ChangeParticion(\'' + part.Nombre + '\');" >\n';
            output += '<img id="icon-alarma" class="icon-image" ';
            if(part.Estado_Activacion > 0) {
                output += 'src="../images/lock1.png"';
            } else {
                output += 'src="../images/lock0.png"';
            }
            output += ' />&nbsp;' + part.Nombre + '\n';
            output += '</div>\n';

            for (i = 0; i < zonas.length; i++) {
                output += '<div class="list-btn-group1" ';
                output += 'onClick="ChangeZona(\'' + part.Nombre + '\',\'' + zonas[i].Objeto + '\');">\n';
                output += '<img id="icon-zona' + i + '" ';
                if(zonas[i].Activa > 0) {
                    if (zonas[i].Estado > 0) {
                        output += 'src="../images/led_rojo1.png"';
                    } else {
                        output += 'src="../images/led_rojo0.png"';
                    }
                } else {
                    output += 'src="../images/no.png"';
                }
                output += ' >&nbsp;' + zonas[i].Objeto + '\n';
                output += '</div>\n';
            }    

            for (i = 0; i < salidas.length; i++) {
                output += '<div class="list-btn-group1" ';
                output += 'onClick="ChangeSalida(\'' + part.Nombre + '\',\'' + salidas[i].Objeto + '\');">\n';
                output += '<img id="icon-salida' + i + '"'; 
                if (salidas[i].Estado > 0) {
                    output += 'src="../images/sirena_mini1.png"';
                } else {
                    output += 'src="../images/sirena_mini0.png"';
                }
                output += ' >&nbsp;' + salidas[i].Objeto + '\n';
                output += '</div>\n';
            }    

            document.getElementById('event-list').innerHTML = output;
        }
    } catch (e) { }
}

function OnLoad() {
    newAJAXCommand('/cgi-bin/dompi_cloud_alarma.cgi?funcion=status_part&sistema=<?php echo $sistema; ?>&part=<?php echo $part; ?>', LoadData, true);
}

function ChangeParticion(part) {
    newAJAXCommand('/cgi-bin/dompi_cloud_alarma.cgi?funcion=switch_part&sistema=<?php echo $sistema; ?>&part=' + part);
}

function ChangeZona(part, zona) {
    newAJAXCommand('/cgi-bin/dompi_cloud_alarma.cgi?funcion=switch_zona&sistema=<?php echo $sistema; ?>&part=' + part + '&zona=' + zona);
}

function ChangeSalida(part, salida) {
    newAJAXCommand('/cgi-bin/dompi_cloud_alarma.cgi?funcion=pulse_salida&sistema=<?php echo $sistema; ?>&part=' + part + '&salida=' + salida);
}

</script>

</body>
<?php
include("foot_m.php");
?>
