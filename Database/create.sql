create database DB_DOMPICLOUD;
USE DB_DOMPICLOUD;

DROP TABLE IF EXISTS TB_DOMCLOUD_USER;
DROP TABLE IF EXISTS TB_DOMCLOUD_NOTIF;
DROP TABLE IF EXISTS TB_DOMCLOUD_ASSIGN;
DROP TABLE IF EXISTS TB_DOMCLOUD_ALARM;
DROP TABLE IF EXISTS TB_DOMCLOUD_ALARM_ENTRADA;
DROP TABLE IF EXISTS TB_DOMCLOUD_ALARM_SALIDA;

CREATE TABLE TB_DOMCLOUD_USER (
Usuario varchar(256),
Clave varchar(256),
Id_Sistema varchar(256),
Amazon_Key varchar(256),
Google_Key varchar(256),
Apple_Key varchar(256),
Other_Key varchar(256),
Errores integer DEFAULT 0,
Ultima_Conexion bigint DEFAULT 0,
Estado integer DEFAULT 0,     -- 0 Disable, 1 Enable
PRIMARY KEY (Usuario)
);

CREATE TABLE TB_DOMCLOUD_ASSIGN (
System_Key varchar(256),
Id integer,
Objeto varchar(128),
Tipo integer DEFAULT 0,                 -- 0=Output, 1=Input, 2=Analog, 3=Output Alarma, 4=Input Alarma, 5=Output Pulse/Analog_Mult_Div_Valor=Pulse Param, 6=Periferico, 1x=Automatizacion (3 estados)
Estado integer DEFAULT 0,               -- 0=Apagado, 1=Encendido, 2=Automático
Icono_Apagado varchar(32),
Icono_Encendido varchar(32),
Icono_Auto varchar(32),
Grupo_Visual integer DEFAULT 0,         -- 0=Ninguno 1=Alarma 2=Iluminación 3=Puertas 4=Climatización 5=Cámaras 6=Riego
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

CREATE TABLE TB_DOMCLOUD_ALARM (
System_Key varchar(256),
Id integer,
Particion varchar(128),
Estado_Activacion integer,
Estado_Memoria integer,
Estado_Alarma integer,
Ultimo_Update varchar(32),
PRIMARY KEY (System_Key, Id)
);

CREATE TABLE TB_DOMCLOUD_ALARM_ENTRADA (
System_Key varchar(256),
Id integer,
Particion integer, 
Entrada varchar(128),
Tipo_Entrada integer,
Grupo integer,
Activa integer,
Estado_Entrada integer,
Ultimo_Update varchar(32),
PRIMARY KEY (System_Key, Id, Particion)
);

CREATE TABLE TB_DOMCLOUD_ALARM_SALIDA (
System_Key varchar(256),
Id integer,
Particion integer, 
Salida varchar(128),
Tipo_Salida integer,
Estado_Salida integer,
Ultimo_Update varchar(32),
PRIMARY KEY (System_Key, Id, Particion)
);
