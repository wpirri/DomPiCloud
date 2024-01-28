<?php
$TITLE = "Dompi Cloud - Admin";
//$ONLOAD = "";
require('head_admin.php');
if(isset($resp_code) && isset($resp_msg) && isset($sistema))
{
    if($resp_code == 0 && $sistema == 'ADMIN')
    {
?>
<!-- ==================== CARGA AUTORIZADA ==================== -->


<div class="main-head">
    <a class="menu-link" onclick="window.location.replace('../index.php');">Desloguearse</a>
    &nbsp;&nbsp;&nbsp;&nbsp;
</div>

<div class="tabs">
    <div class="tab-container">
        <div id="tab4" class="tab"> 
            <a href="#tab4">Actualizar</a>
            <div class="tab-content">
                <h2>Actualizacion del sistema</h2>
                <div class="tab-content-form" id="upgrade-form">
                <form enctype="multipart/form-data" action="" method="post" id="update_form" name="update_form" method="post">

                    <div class="abm-save-cancell">
                        <a class="menu-link" onclick="Upgrade();">Subir</a>
                        &nbsp;&nbsp;&nbsp;&nbsp;
                    </div>
                    <br />
                    <br />
                    <br />
                    &nbsp;&nbsp;&nbsp;&nbsp;<input type="file" size="35" name="uploadedfile" />
                    <br />
                    <br />
                    <br />
                    <br />
                    <br />
                    <div id='update_result_div' class='abm-result-message'>&nbsp;</div>
                    <br />
                </form>
                </div>
            </div>
        </div>
        <div id="tab3" class="tab" OnClick="GetUserList();"> 
            <a href="#tab3">Usuarios</a>
            <div class="tab-content">
                <form method="post"  id="user-form">
                <div class="tab-content-form" id="user-data">
                    <div class="abm-save-cancell">
                        <a class="menu-link" onclick="GetUserDataNew();">Nuevo</a>
                        &nbsp;&nbsp;&nbsp;&nbsp;
                    </div>
                </div>
                </form>
                <div class="tab-content-list" id="user-list">&nbsp</div>
            </div>
        </div>
        <div id="tab2" class="tab" OnClick="GetClientList();">
            <a href="#tab2">Clientes</a>
            <div class="tab-content">
                <div class="tab-content-form" id="client-data">&nbsp</div>
                <div class="tab-content-list" id="client-list">&nbsp</div>
            </div>
        </div> 
        <div id="tab1" class="tab">
            <a href="#tab1">Sistema</a>
            <div class="tab-content">
                <div class="tab-content-list" id="system-list">
                    <h2>Nube de Domotica</h2>
                    <pre><?php system("/usr/local/sbin/gmon_status all"); ?></pre>
                    <br />
                    <h2>Espacio en Disco</h2>
                    <pre><?php system("df -h"); ?></pre>
                    <br />
                    <h2>Sistema Operativo</h2>
                    <pre><?php system("top -b -n 1 -c -1 -w 512"); ?></pre>
                    <br />
                    <br />
                </div>
            </div> 
        </div> 
    </div>
</div>

<script type="text/javascript" >
    var status_system_key = '';
    function LoadClientList(msg) { fillClientList(JSON.parse(msg).response, 'client-list', 'System_Key', 'GetClientStatus', 'DelClientData'); }
    function GetClientList() { newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi', LoadClientList, false); }
    function LoadClientStatus(msg) { fillClientStatus(JSON.parse(msg).response, 'client-data', 'Cliente: ' + status_system_key, 'CancelClient'); }
    function GetClientStatus(system_key) { 
        status_system_key = system_key;
        newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?System_Key=' + system_key, LoadClientStatus, false); 
    }
    function CancelClient() { document.getElementById('client-data').innerHTML = '&nbsp;'; }
    function DelClientData(system_key) { 
        newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Admin=delcli&System_Key=' + system_key, null, false); 
        CancelClient();
    }

    function LoadUserList(msg) { fillUserList(JSON.parse(msg).response, 'user-list', 'Usuario', 'GetUserDataEdit', 'GetUserDataDel'); }
    function GetUserList() { newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi', LoadUserList, false); }
    function LoadUserDataNew(msg) { fillUserNew(JSON.parse(msg).response, 'user-data', 'AddUser', 'CancelUser'); }
    function GetUserDataNew() { newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=get&Usuario=Admin' , LoadUserDataNew, false); }
    function LoadUserDataEdit(msg) { fillUserEdit(JSON.parse(msg).response, 'user-data', 'SaveUser', 'CancelUser'); }
    function GetUserDataEdit(usuario) { newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=get&Usuario=' + usuario, LoadUserDataEdit, false); }
    function LoadUserDataDel(msg) { fillUserDelete(JSON.parse(msg).response, 'user-data', 'DeleteUser', 'CancelUser'); }
    function GetUserDataDel(usuario) { newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=get&Usuario=' + usuario, LoadUserDataDel, false); }
    function CancelUser() { document.getElementById('user-data').innerHTML = '<div class="abm-save-cancell"><a class="menu-link" onclick="GetUserDataNew();">Nuevo</a>&nbsp;&nbsp;&nbsp;&nbsp;</div>'; }
    function AddUser() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */
        var kvpairs = [];
        var form = document.getElementById('user-form');
        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=add', null, false, kvpairs.join('&'));
        CancelUser();
    }
    function SaveUser() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */
        var kvpairs = [];
        var form = document.getElementById('user-form');
        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=update', null, false, kvpairs.join('&'));
        CancelUser();
    }
    function DeleteUser(usuario) {
        newAJAXCommand('/cgi-bin/dompi_cloud_abmuser.cgi?funcion=delete&Usuario=' + usuario, null, false);
        CancelUser();
    }

    //function control_object_on(obj) { newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=on&System_Key=      &Objeto=' + obj, null, false); }
    //function control_object_off(obj) { newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=off&System_Key=       &Objeto=' + obj, null, false); }
    //function control_object_switch(obj) { newAJAXCommand('/cgi-bin/dompi_cloud_status.cgi?Accion=switch&System_Key=       &Objeto=' + obj, null, false); }

    function Upgrade() {
        document.getElementById('update_result_div').innerHTML = 'Actualizando...';
        document.update_form.submit();
    }
    
    <?php
        // php.ini:
        //  file_uploads = On
        //  upload_max_filesize=10M
        //  post_max_size=11M
        if( isset($_FILES['uploadedfile']['name']) )
        {
            if ( move_uploaded_file($_FILES['uploadedfile']['tmp_name'], "upload/".$_FILES['uploadedfile']['name']) )
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
