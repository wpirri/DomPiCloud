CREATE USER 'dompi_cloud'@'%' IDENTIFIED BY 'dompi_cloud'; 
GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPICLOUD.* TO 'dompi_cloud'@'%' WITH GRANT OPTION;