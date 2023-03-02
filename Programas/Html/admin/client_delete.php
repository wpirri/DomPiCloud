<?php
$TITLE = "Borrando...";
$ONLOAD = "OnLoad();";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->
<script type="text/javascript" >
    function OnLoad() {
        newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Admin=delcli&System_Key=<?php echo $_GET['System_Key']; ?>', null, false);
        setTimeout("window.location.replace('client_list.php');", 5000);
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
