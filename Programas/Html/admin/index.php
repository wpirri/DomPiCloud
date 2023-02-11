<?php
$TITLE = "Dompi Cloud - Admin";
$ONLOAD = "";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 0)
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->


<div class="back-btn" id="back-from-config" onclick="window.location.replace('../index.php');">
    <img id="back-icon" class="icon-btn" src="../images/back.png">&nbsp;Volver
</div>


<div class="normal-btn" id="btn-config-sistema" onclick="window.location.replace('config.php');">
    <img id="upload-icon" class="icon-btn" src="../images/gear.png">&nbsp;Sistema
</div>

<div class="normal-btn" id="btn-config-usuarios" onclick="window.location.replace('abm_user_list.php');">
    <img id="user-icon" class="icon-btn" src="../images/access.png">&nbsp;Usuarios
</div>

<div class="normal-btn" id="btn-config-planta" onclick="window.location.replace('client_list.php');">
    <img id="home-icon" class="icon-btn" src="../images/home.png">&nbsp;Clientes
</div>


<!--
<div class="normal-btn" id="btn-config-hard" onclick="window.location.replace('hw_list.php');">
    <img id="disp-icon" class="icon-btn" src="images/hard.png">&nbsp;Dispositivos
</div>
-->

<!--
<div class="normal-btn" id="btn-config-assign" onclick="window.location.replace('ass_list.php');">
    <img id="net-icon" class="icon-btn" src="images/lamp1.png">&nbsp;Objetos
</div>
-->

<!--
<div class="normal-btn" id="btn-config-groups" onclick="window.location.replace('group_list.php');">
    <img id="group-icon" class="icon-btn" src="images/group.png">&nbsp;Grupos
</div>
-->

<div class="normal-btn" id="btn-config-events" onclick="window.location.replace('eventos.php');">
    <img id="event-icon" class="icon-btn" src="../images/event.png">&nbsp;Eventos
</div>


<!--
<div class="normal-btn" id="btn-config-flags" onclick="window.location.replace('flag_list.php');">
    <img id="flags-icon" class="icon-btn" src="images/var.png">&nbsp;Variables
</div>
-->

<!--
<div class="normal-btn" id="btn-config-crono" onclick="window.location.replace('task.php');">
    <img id="crono-icon" class="icon-btn" src="images/cron.png">&nbsp;Programas
</div>
-->
<!--
<div class="normal-btn" id="btn-config-alarm" onclick="window.location.replace('alarm.php');">
    <img id="alarm-icon" class="icon-btn" src="images/lock1.png">&nbsp;Alarma
</div>
-->
<!--
<div class="normal-btn" id="btn-config-cameras" onclick="window.location.replace('working_list.php');">
    <img id="camera-icon" class="icon-btn" src="images/camara.png">&nbsp;C&aacute;maras
</div>
-->

<div class="normal-btn" id="btn-config-upload" onclick="window.location.replace('working_list.php');">
    <img id="upload-icon" class="icon-btn" src="../images/upload.png">&nbsp;Actualizaci&oacute;n
</div>

<div class="normal-btn" id="btn-config-download" onclick="window.location.replace('working_list.php');">
    <img id="logs-icon" class="icon-btn" src="../images/download.png">&nbsp;Descargas
</div>


<div class="normal-btn" id="btn-config-dummy2" onclick="window.location.replace('working_list.php');">
    <!-- <img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;xxxxxxxx -->
</div>

<div class="normal-btn" id="btn-config-dummy3" onclick="window.location.replace('working_list.php');">
    <!-- <img id="logs-icon" class="icon-btn" src="images/download.png">&nbsp;xxxxxxxx -->
</div>

</div>

<!-- ==================== CARGA AUTORIZADA ==================== -->
<?php
    }
    else
    {
        ?>
        <script type="text/javascript" >
        window.location.replace('../login.php?msg=Error');
        </script>
        <?php
    }
}
else
{
    ?>
    <script type="text/javascript" >
    window.location.replace('../login.php?msg=Error');
    </script>
    <?php
}
require('foot_admin.php');
?>
