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

<form id="edit_form" name="edit_form" method="post">

<div id='user_edit_back_btn' class='back-btn' onclick="window.location.replace('abm_user_list.php');" >
	<img id='user_edit_back_icon' class='icon-btn' src='../images/no.png'>&nbsp;Cancelar
</div>

<div id='user_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='user_edit_save_icon' class='icon-btn' src='../images/ok.png'>&nbsp;Guardar
</div>

<div id='user_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function LoadData(msg) {
        fillAbmEdit(JSON.parse(msg).response, 'user_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('edit_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=update', null, false, kvpairs.join('&'));

        window.location.replace('abm_user_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=get&Usuario=\'<?php echo $_GET['Usuario']; ?>\'', LoadData, false);
    }
</script>

</form>

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
