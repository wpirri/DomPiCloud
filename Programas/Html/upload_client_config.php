<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<title>Upload</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="author" content="Walter Pirri" >
<meta name="keywords" content="SMART HOME, SYSHOME, DOMOTIC, SECURITY SYSTEM, IOT">
<meta name="description" content="Sistema integrado de monitoreo, alarma y domotica">
<meta name="system-build" content="2023">
</head>
<body>

<?php

// php.ini:
//  file_uploads = On
//  upload_max_filesize=10M
//  post_max_size=11M
if( isset($_FILES['uploadedfile']['name']) )
{
    if ( move_uploaded_file($_FILES['uploadedfile']['tmp_name'], "clients_uploads/".$_FILES['uploadedfile']['name']) )
    { 
        ?><p>Upload Ok</p><?php
    }
    else
    { 
        ?><p>Upload Error</p><?php
    }
}

?>

</body>
</html>
