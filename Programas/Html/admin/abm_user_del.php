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

<div id='user_delete_back_btn' class='back-btn' onclick="window.location.replace('abm_user_list.php');" >
	<img id='user_delete_back_icon' class='icon-btn' src='../images/no.png'>&nbsp;Cancelar
</div>

<div id='user_delete_save_btn' class='submit-btn' onclick="DeleteData();" >
	<img id='user_delete_save_icon' class='icon-btn' src='../images/ok.png'>&nbsp;Borrar
</div>

<div id='user_delete_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmDelete(JSON.parse(msg).response, 'user_delete_div', '<?php echo $TITLE; ?>');
    }

    function DeleteData() {
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=delete&Usuario=<?php echo $_GET['Usuario']; ?>', null, false);
        window.location.replace('abm_user_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=get&Usuario=\'<?php echo $_GET['Usuario']; ?>\'', LoadData, false);
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
