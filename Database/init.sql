\u DB_DOMPICLOUD

CREATE USER 'dompi_cloud'@'%' IDENTIFIED BY 'dompi_cloud'; 
GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPICLOUD.* TO 'dompi_cloud'@'%' WITH GRANT OPTION;

INSERT INTO TB_DOMCLOUD_USER (Usuario, Clave, Id_Sistema, Estado) VALUES ("Admin", "Admin", "ADMIN", 1);
