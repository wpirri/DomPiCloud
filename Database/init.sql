CREATE ROLE dompi_cloud LOGIN
  NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION;

ALTER ROLE dompi_cloud
  SET password_encryption = 'scram-sha-256';

alter user dompi_cloud with password 'dompi_cloud';

GRANT SELECT, INSERT, UPDATE, DELETE ON TB_DOMCLOUD_USER TO dompi_cloud;
GRANT SELECT, INSERT, UPDATE, DELETE ON TB_DOMCLOUD_ASSIGN TO dompi_cloud;
GRANT SELECT, INSERT, UPDATE, DELETE ON TB_DOMCLOUD_NOTIF TO dompi_cloud;

INSERT INTO TB_DOMCLOUD_USER (Usuario, Clave, Id_Sistema, Estado) VALUES ("Admin", "Admin", "ADMIN", 1);
