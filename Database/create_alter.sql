# 12/07/2023 - Creación de ìndices
alter table TB_DOMCLOUD_ASSIGN add unique index idx_system_key_id (System_Key,Id);
alter table TB_DOMCLOUD_ASSIGN add unique index idx_system_key_objeto_id (System_Key,Objeto,Id);
