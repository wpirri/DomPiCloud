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
// curl --insecure -F "file=@${FILENAME};filename=${FILE}" "${HOSTNAME}/${UPLOAD_FORM}"
if( isset($_FILES['file']['name']) )
{
    if ( move_uploaded_file($_FILES['file']['tmp_name'], "clients_uploads/".$_FILES['file']['name']) )
    {
        ?><p>Upload Ok</p><?php
    }
    else
    {
        ?><p>Upload Error</p><?php
    }
}

// var_dump($_FILES);

?>

</body>
</html>
