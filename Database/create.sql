\u DB_DOMPICLOUD

DROP TABLE IF EXISTS TB_DOMCLOUD_USER;
DROP TABLE IF EXISTS TB_DOMCLOUD_NOTIF;
DROP TABLE IF EXISTS TB_DOMCLOUD_ASSIGN;

CREATE TABLE IF NOT EXISTS TB_DOMCLOUD_USER (
Usuario varchar(256),
Clave varchar(256),
Id_Sistema varchar(256),
Errores integer DEFAULT 0,
Ultima_Conexion bigint DEFAULT 0,
Estado integer DEFAULT 0,     -- 0 Disable, 1 Enable
PRIMARY KEY (Usuario)
);


CREATE TABLE IF NOT EXISTS TB_DOMCLOUD_ASSIGN (
System_Key varchar(256),
Id integer,
Objeto varchar(128),
Tipo integer DEFAULT 0,
Estado integer DEFAULT 0,
Icono0 varchar(32),
Icono1 varchar(32),
Grupo_Visual integer DEFAULT 0,
Planta integer DEFAULT 0,
Cord_x integer DEFAULT 0,
Cord_y integer DEFAULT 0,
Coeficiente integer DEFAULT 0,
Analog_Mult_Div integer DEFAULT 0,
Analog_Mult_Div_Valor integer DEFAULT 1,
Ultimo_Update varchar(32),
Flags integer DEFAULT 0,
PRIMARY KEY (System_Key, Id)
);

CREATE TABLE TB_DOMCLOUD_NOTIF (
System_Key varchar(256),
Time_Stamp bigint,
Objeto varchar(128),
Accion varchar(16),
PRIMARY KEY (System_Key, Time_Stamp)
);