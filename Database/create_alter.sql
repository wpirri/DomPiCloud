# 12/07/2023 - Creación de ìndices
alter table TB_DOMCLOUD_ASSIGN add unique index idx_system_key_id (System_Key,Id);
alter table TB_DOMCLOUD_ASSIGN add unique index idx_system_key_objeto_id (System_Key,Objeto,Id);

# 31/01/2025 - IDs alfanumericos
drop table TB_DOMCLOUD_ASSIGN;
CREATE TABLE TB_DOMCLOUD_ASSIGN (
System_Key varchar(256),
Id varchar(16),                         -- Permito Ids alfanumericos
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
Time_Stamp bigint,
Flags integer DEFAULT 0,
PRIMARY KEY (System_Key, Id)
);

