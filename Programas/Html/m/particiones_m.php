<?php
$TITLE='Particiones'; 
include("head_m.php");
?>

<body onload="OnLoad()">

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
    try {
        // Armo el listado de particiones
        var i = 0;
        var json_list = JSON.parse(msg).response;
        var output = '';
        var filename = '';

        if(!json_list) return;

        if(document.getElementById('icon-head'))
        {
            for (i = 0; i < json_list.length; i++) {
                if( json_list[i].Id > 0 )
                {
                    if( json_list[i].Activada > 0) {
                        document.getElementById('icon-part' + i).src = '../images/lock1.png';
                    } else {
                        document.getElementById('icon-part' + i).src = '../images/lock0.png';
                    }
                }
            }    
        }
        else
        {
            // Cabecera
            output += '<div class="list-head" id="list-head1"  >\n';
            output += '<img id="icon-head" id="icon-image1" class="icon-image" src="../images/lock0.png" />&nbsp;Particiones\n';
            output += '</div>\n';

            for (i = 0; i < json_list.length; i++) {
                if( json_list[i].Id > 0 )
                {
                    output += '<div class="list-btn-group1" onClick="LoadAlarma(\'' + json_list[i].Nombre + '\');" >\n';
                    output += '<img id="icon-part' + i + '" class="icon-image" src="../images/';
                    if( json_list[i].Activada > 0) {
                        output += 'lock1.png';
                    } else {
                        output += 'lock0.png';
                    }
                    output += '" />&nbsp;&nbsp;';
                    output += json_list[i].Nombre;
                    output += '\n</div>\n';
                }
            }    

            document.getElementById('event-list').innerHTML = output;
        }
	} catch (e) { }
}

function LoadAlarma(part) {
    window.location.replace('alarma_m.php?part=' + part);
}

function OnLoad() {
    newAJAXCommand('/cgi-bin/dompi_cloud_alarma.cgi?funcion=list_part&sistema=<?php echo $sistema; ?>', LoadData, false);
}

</script>

</body>
<?php
include("foot_m.php");
?>
