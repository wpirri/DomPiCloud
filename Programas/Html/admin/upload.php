<?php
$TITLE = "Upload";
$ONLOAD = "OnLoad();";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->

<div id='user_list_back_btn' class='back-btn' onclick="window.location.replace('index.php');" >
	<img id='user_list_back_icon' class='icon-btn' src='../images/back.png'>&nbsp;Volver
</div>


<script type="text/javascript" >
    function LoadData(msg) {

    }

    function OnLoad() {

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
