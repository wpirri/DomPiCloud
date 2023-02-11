<?php
$TITLE = "Clientes";
$ONLOAD = "OnLoad();";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 0)
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->

<div id='user_list_back_btn' class='back-btn' onclick="window.location.replace('index.php');" >
	<img id='user_list_back_icon' class='icon-btn' src='../images/back.png'>&nbsp;Volver
</div>

<div id='user_list_add_btn' class='abm-add-btn' onclick="window.location.replace('abm_user_add.php');" >
	<img id='user_list_add__icon' class='icon-btn' src='../images/add.png'>&nbsp;Nuevo
</div>

<div id='user_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmList(JSON.parse(msg).response, 'user_list_table_div', '<?php echo $TITLE; ?>', 'Usuario', 'abm_user_edit.php', 'abm_user_del.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi', LoadData, false);
    }
</script>

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
