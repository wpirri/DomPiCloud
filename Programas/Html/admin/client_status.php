<?php
$TITLE = "Estado";
$ONLOAD = "OnLoad();";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->

<div id='client_status_back_btn' class='back-btn' onclick="window.location.replace('client_list.php');" >
  <img id='client_status_back_icon' class='icon-btn' src='../images/back.png'>&nbsp;Volver
</div>

<div id='client_status_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'client_status_table_div', '<?php echo $TITLE; ?>', 'Id', 'client_edit.php', 'client_delete.php');
    }

    function OnLoad() {
      newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?System_Key=<?php echo $_GET['System_Key']; ?>', LoadData, false);
    }

    function control_object_on(obj)
    {
      newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=on&System_Key=<?php echo $_GET['System_Key']; ?>&Objeto=' + obj, null, false);
    }

    function control_object_off(obj)
    {
      newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=off&System_Key=<?php echo $_GET['System_Key']; ?>&Objeto=' + obj, null, false);
    }

    function control_object_switch(obj)
    {
      newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=switch&System_Key=<?php echo $_GET['System_Key']; ?>&Objeto=' + obj, null, false);
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
require('foot_admin.php');
?>
