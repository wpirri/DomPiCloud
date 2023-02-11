<body onload="OnLoad();">
<?php
$TITLE = "Movile";
require('../head.php');
$auth = $_SESSION['auth_token'];
/* DEBUG --> */
echo "<p>Usuario: ".$auth->user."</p>";
echo "<p>Clave:   ".$auth->pass."</p>";
echo "<p>Time:    ".$auth->time."</p>";
/* <-- DEBUG */
?>







</body>
<script type="text/javascript" >
function AuthResponse(msg)
{
    r = JSON.parse(msg).response;
    if( r.resp_code == '0' )
    {


        alert('Movile Ok');



    }
}

function OnLoad()
{
    newAJAXCommand('/cgi-bin/auth.cgi?User=<?php echo $auth->user;?>&Password=<?php echo $auth->pass;?>&Time=<?php echo $auth->time;?>', AuthResponse, false);
}
</script>
<?php
require('../foot.php');
?>
