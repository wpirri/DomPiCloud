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

<div class="desktop-group" id="desktop">

<!-- Grupos -->
<div class="menu-btn" id="menu1" onclick="window.location.replace('objetos_m.php?grupo=1');">
	<img id="alarm1-icon" class="icon-image" src="../images/lock.png" >&nbsp;Alarma
	<div class="status-text" id="alarm1_status_arm">&nbsp;* ????</div>
	<div class="status-text" id="alarm1_status_rdy">&nbsp;* ????</div>
</div>

<div class="menu-btn" id="menu2" onclick="window.location.replace('objetos_m.php?grupo=2');">
	<img id="home-icon" class="icon-image" src="../images/lamp1.png">&nbsp;Luces
</div>

<div class="menu-btn" id="menu3" onclick="window.location.replace('objetos_m.php?grupo=3');">
	<img id="zone-icon" class="icon-image" src="../images/door1.png">&nbsp;Puertas
</div>

<div class="menu-btn" id="menu4" onclick="window.location.replace('objetos_m.php?grupo=4');">
	<img id="zone-icon" class="icon-image" src="../images/calef1.png">&nbsp;Clima
</div>

<div class="menu-btn" id="menu5" onclick="window.location.replace('objetos_m.php?grupo=5');">
	<img id="zone-icon" class="icon-image" src="../images/camara.png">&nbsp;C&aacute;maras
</div>

</div>
&nbsp;
</div>

<div class="app-btn" id="menu-return" onclick="window.location.replace('../index.php');">
	<img id="back-icon" class="icon-image" src="../images/back.png">&nbsp;Volver
</div>

</div>

<script type="text/javascript">

// Parses the xmlResponse from status.xml and updates the status box
function updateStatus(xmlData) {

}

function InitUpdate() {
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
