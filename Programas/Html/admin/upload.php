<?php
$TITLE = "Actualizar";
$ONLOAD = "OnLoad();";
$UPLOAD_FOLDER='upload';

require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->

<form enctype="multipart/form-data" action="upload.php" method="post" id="update_form" name="update_form" method="post">

<div id='user_list_back_btn' class='back-btn' onclick="window.location.replace('index.php');" >
	<img id='user_list_back_icon' class='icon-btn' src='../images/back.png'>&nbsp;Volver
</div>

<div id='update_save_btn' class='submit-btn' onclick="Upload();" >
	<img id='update_save_icon' class='icon-btn' src='../images/ok.png'>&nbsp;Actualizar
</div>

<div id='update_div' class='abm-div'>
    <p class=abm-table-title>&nbsp;Actualizar</p>
    <br />
    &nbsp;<input type="file" size="35" name="uploadedfile" />
    <br />
    <br />
    <br />
    <div id='update_result_div' class='abm-result-message'>&nbsp;</div>
</div>

<script type="text/javascript" >
    <?php

    // php.ini:
    //  file_uploads = On
    //  upload_max_filesize=10M
    //  post_max_size=11M
    if( isset($_FILES['uploadedfile']['name']) )
    {
        if ( move_uploaded_file($_FILES['uploadedfile']['tmp_name'], $UPLOAD_FOLDER."/".$_FILES['uploadedfile']['name']) )
        { 
            ?>
            document.getElementById('update_result_div').innerHTML = 'Actualizacion ok, el sistema se va a reiniciar.';
            <?php
        }
        else
        { 
            ?>
            document.getElementById('update_result_div').innerHTML = 'Error en actualizacion.';
            <?php
        }
    }

    ?>
    function OnLoad() {

    }

    function Upload() {
        document.getElementById('update_result_div').innerHTML = 'Actualizando...';
        document.update_form.submit();
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
