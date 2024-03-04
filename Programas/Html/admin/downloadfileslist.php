<?php
    $DOWNLOAD_FOLDER="../clients_uploads";

    echo "<table class=abm-list-table>";
    echo "<tr><th>Archivo</th><th>Bajar</th></tr>";

    $dir = scandir($DOWNLOAD_FOLDER);
    if($dir != false)
    {
        foreach($dir as $value)
        {
            if(is_file($DOWNLOAD_FOLDER."/".$value))
            {
                echo "<tr><td>".$value."</td>";
                echo "<td><a target=_blank href=\"".$DOWNLOAD_FOLDER."/".$value."\"><img src=\"../images/download.png\"></a></td></tr>";
            }
        }
    }
    echo "</table>";
?>
