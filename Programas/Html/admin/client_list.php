<?php
$TITLE = "Clientes";
$ONLOAD = "OnLoad();";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->

<div id='client_list_back_btn' class='back-btn' onclick="window.location.replace('index.php');" >
  <img id='client_list_back_icon' class='icon-btn' src='../images/back.png'>&nbsp;Volver
</div>

<div id='client_list_add_btn' class='abm-add-btn' onclick="window.location.replace('');" >
  <img id='client_list_add_icon' class='icon-btn' src='../images/add.png'>&nbsp;Nuevo
</div>

<div id='client_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'client_list_table_div', '<?php echo $TITLE; ?>', 'System_Key', 'client_status.php', '');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi', LoadData, false);
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
