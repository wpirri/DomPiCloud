/***************************************************************************
    Copyright (C) 2021   Walter Pirri

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/
/*
Informaciòn de desarrollo sobre Raspberry Pi

https://pinout.xyz/
http://diymakers.es/usando-el-puerto-gpio/
http://wiringpi.com/reference/


instalar libcjson-dev (https://github.com/DaveGamble/cJSON)
#include <cjson/cJSON.h>
-lcjson

char *p;
cJSON *obj;
cJSON *str;
cJSON *num;
cJSON *arr;
obj = cJSON_CreateObject();


-- String ---------------------------------------------------------------------
obj = cJSON_Parse(const char *value);

-- String ---------------------------------------------------------------------
str = cJSON_CreateString("un string");
cJSON_AddItemToObject(obj, "nombre", str);
o
cJSON_AddStringToObject(obj, "nombre", "un string")

-- Number ---------------------------------------------------------------------
num = cJSON_CreateNumber(50);
cJSON_AddItemToObject(obj, "edad", num);
o
cJSON_AddNumberToObject(obj, edad, 50);

-- Array ----------------------------------------------------------------------
arr = cJSON_CreateArray();
cJSON_AddItemToArray(arr, obj);
cJSON_AddItemToObject(obj, "nombre_array", arr);
o
arr = cJSON_AddArrayToObject(obj, "nombre_arr");

-- Generado -------------------------------------------------------------------
p = cJSON_Print(obj);
p = cJSON_PrintUnformatted(obj)
cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format);

-- Obtener --------------------------------------------------------------------
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "nombre");
un_obj = cJSON_GetObjectItemCaseSensitive(obj, "edad");
    cJSON_IsString(un_obj)
    name->valuestring

    cJSON_IsNumber
    width->valuedouble

-- Free -----------------------------------------------------------------------
cJSON_Delete(obj);


#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

*/

#include <gmonitor/gmerror.h>
#include <gmonitor/gmontdb.h>
/*#include <gmonitor/gmstring.h>*/
#include <gmonitor/gmswaited.h>

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

#include "config.h"
#include "strfunc.h"
//#include "cpgdb.h"
#include "cmydb.h"

#define MAX_BUFFER_LEN 32767

CGMServerWait *m_pServer;
DPConfig *pConfig;

const char *user_columns[] = {
	"Usuario",
	"Clave",
	"Id_Sistema",
	"Errores",
	"Ultima_Conexion",
	"Estado",
	0};

const char *assign_columns[] = {
	"System_Key",
	"Id",
	"ASS_Id",
	"Objeto",
	"Tipo",
	"Tipo_ASS",
	"Estado",
	"Icono0",
	"Icono1",
	"Grupo_Visual",
	"Planta",
	"Cord_x",
	"Cord_y",
	"Coeficiente",
	"Analog_Mult_Div",
	"Analog_Mult_Div_Valor",
	"Ultimo_Update",
	"Flags",
	0};

int power2(int exp)
{
	switch(exp)
	{
		case 0x00: return 0x01;
		case 0x01: return 0x02;
		case 0x02: return 0x04;
		case 0x03: return 0x08;
		case 0x04: return 0x10;
		case 0x05: return 0x20;
		case 0x06: return 0x40;
		case 0x07: return 0x80;
		default:   return 0x00;
	}
}

void OnClose(int sig);

int QueryForNews(CMyDB* db, cJSON *response_array, const char *client_key)
{
	int rc;
	char query[4096];

	m_pServer->m_pLog->Add(100, "[QueryForNews] System_Key = [%s].", client_key);
	sprintf(query, "SELECT * "
					"FROM TB_DOMCLOUD_NOTIF "
					"WHERE System_Key = \'%s\';", client_key);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	rc = db->Query(response_array, query);
					
	return rc;
}

void DeleteNotify(CMyDB *db, const char *client_key, time_t t)
{
	char query[4096];
	m_pServer->m_pLog->Add(100, "[DeleteNotify] System_Key = [%s].", client_key);
	sprintf(query, "DELETE FROM TB_DOMCLOUD_NOTIF "
					"WHERE System_Key = \'%s\' AND Time_Stamp <= %lu;", client_key, t);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	db->Query(NULL, query);
}

int ExisteColumna(const char* columna, const char** lista)
{
	int i = 0;

	if(!columna || !lista) return 0;
	while(lista[i])
	{
		if( !strcmp(columna, lista[i])) return 1;
		i++;
	}
	return 0;
}

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	unsigned long message_len;

	char db_host[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	char query_into[2048];
	char query_values[2048];
	char query_set[2048];
	char query_where[512];

	time_t t;
	struct tm *p_tm;
	char save_client_key[256];

	STRFunc Strf;
	//CPgDB *pDB;
	CMyDB *pDB;

    cJSON *json_obj;
    cJSON *json_arr;
    cJSON *json_un_obj;
    cJSON *json_Id;
    cJSON *json_System_Key;
	cJSON *json_Objeto;
	cJSON *json_Estado;
	cJSON *json_Time_Stamp;
	cJSON *json_User;
	cJSON *json_Password;
	cJSON *json_Time;
	cJSON *json_Id_Sistema;
	cJSON *json_sistema;
	cJSON *json_grupo;

    cJSON *json_Accion;
    cJSON *json_Admin;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, OnClose);
	signal(SIGTERM, OnClose);
	signal(SIGSTOP, OnClose);
	signal(SIGABRT, OnClose);
	signal(SIGQUIT, OnClose);
	signal(SIGINT,  OnClose);
	signal(SIGILL,  OnClose);
	signal(SIGFPE,  OnClose);
	signal(SIGSEGV, OnClose);
	signal(SIGBUS,  OnClose);

	m_pServer = new CGMServerWait;
	m_pServer->Init("dompi_cloud_server");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de Nube de Domotica...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompicloud.config");
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

	m_pServer->Suscribe("dompi_web_notif", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_status", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_check_user", GM_MSG_TYPE_CR);

	m_pServer->Suscribe("dompi_cloud_user_list", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_user_list_all", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_user_get", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_user_add", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_user_delete", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_user_update", GM_MSG_TYPE_CR);

	m_pServer->Suscribe("dompi_cloud_list_objects", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("dompi_cloud_touch_object", GM_MSG_TYPE_CR);
	

	m_pServer->m_pLog->Add(10, "Conectado a la base de datos...");
	pDB = new CMyDB(db_host, "DB_DOMPICLOUD", db_user, db_password);
	if(pDB == NULL)
	{
		m_pServer->m_pLog->Add(1, "ERROR: Al conectarse a la base de datos.");
	}
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR: En Open a la base de datos.");
	}

	m_pServer->m_pLog->Add(1, "Servicios de Domotica en la nube inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 1000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(100, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_web_notif - 
			**************************************************************** */
			if( !strcmp(fn, "dompi_web_notif"))
			{
				t = time(&t);
				p_tm = localtime(&t);

				json_obj = cJSON_Parse(message);

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "System_Key");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(!json_Id)
				{
					json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "ASS_Id");
				}

				if(json_System_Key)
				{
					if(json_Id)
					{
						query[0] = 0;
						query_into[0] = 0;
						query_values[0] = 0;
						query_set[0] = 0;
						query_where[0] = 0;

						m_pServer->m_pLog->Add(100, "[*** Update ***] System_Key: %s", json_System_Key->valuestring);

						json_un_obj = json_obj;
						while( json_un_obj )
						{
							/* Voy hasta el elemento con datos */
							if(json_un_obj->type == cJSON_Object)
							{
								json_un_obj = json_un_obj->child;
							}
							else
							{
								if(json_un_obj->type == cJSON_String)
								{
									if(json_un_obj->string && json_un_obj->valuestring)
									{
										if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
										{
											if(ExisteColumna(json_un_obj->string, assign_columns))
											{
												/* Armo las sentencias del UPDATE */
												if( !strcmp(json_un_obj->string, "System_Key") ||
													!strcmp(json_un_obj->string, "ASS_Id") ||
													!strcmp(json_un_obj->string, "Id"))
												{
													if(strlen(query_where) > 0)
													{
														strcat(query_where, " AND ");
													}
													if( !strcmp(json_un_obj->string, "ASS_Id"))
													{
														strcat(query_where, "Id");
													}
													else
													{
														strcat(query_where, json_un_obj->string);
													}
													strcat(query_where, "='");
													strcat(query_where, json_un_obj->valuestring);
													strcat(query_where, "'");
												}
												else
												{
													/* Dato = Valor */
													if(strlen(query_set) > 0)
													{
														strcat(query_set, ",");
													}
													if( !strcmp(json_un_obj->string, "Tipo_ASS"))
													{
														strcat(query_set, "Tipo");
													}
													else
													{
														strcat(query_set, json_un_obj->string);
													}
													strcat(query_set, "='");
													strcat(query_set, json_un_obj->valuestring);
													strcat(query_set, "'");
												}
												/* Armo las sentencias del insert */
												/* Dato */
												if(strlen(query_into) == 0)
												{
													strcpy(query_into, "(");
												}
												else
												{
													strcat(query_into, ",");
												}
												strcat(query_into, json_un_obj->string);
												/* Valor */
												if(strlen(query_values) == 0)
												{
													strcpy(query_values, "(");
												}
												else
												{
													strcat(query_values, ",");
												}
												strcat(query_values, "'");
												strcat(query_values, json_un_obj->valuestring);
												strcat(query_values, "'");
											}
										}
									}
								}
								json_un_obj = json_un_obj->next;
							}
						}

						if(strlen(query_where))
						{
							/* agrego Ultimo_Update */
							if(strlen(query_set) > 0)
							{
								strcat(query_set, ",");
							}
							strcat(query_set, "Ultimo_Update=");
							sprintf(&query_set[strlen(query_set)], "\'%04i-%02i-%02i %02i:%02i:%02i\'",
									p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
									p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

							/* Trato de hacer un update */
							sprintf(query, "UPDATE TB_DOMCLOUD_ASSIGN SET %s WHERE %s;", query_set, query_where);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							if(rc == 0)
							{
								/* Si no actualiza nada hago un insert */
								if(strlen(query_into) && strlen(query_values))
								{
									/* Dato */
									strcat(query_into, ",");
									strcat(query_into, "Ultimo_Update");
									/* Valor */
									strcat(query_values, ",");
									sprintf(&query_values[strlen(query_values)], "\'%04i-%02i-%02i %02i:%02i:%02i\'",
											p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
											p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
									/* Final */
									strcat(query_into, ")");
									strcat(query_values, ")");

									sprintf(query, "INSERT INTO TB_DOMCLOUD_ASSIGN %s VALUES %s;", query_into, query_values);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(NULL, query);
								}
							}
						}
					}
					else /* Solo vino la Key */
					{
						m_pServer->m_pLog->Add(100, "[*** Keep Alive ***] System_Key: %s", json_System_Key->valuestring);
						sprintf(query, "UPDATE TB_DOMCLOUD_ASSIGN "
													"SET Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\' "
													"WHERE System_Key = \'%s\' AND Id = 0;",
												p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
												p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
												json_System_Key->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						if(rc == 0)
						{
							sprintf(query, "INSERT INTO TB_DOMCLOUD_ASSIGN (System_Key, Id, Ultimo_Update) "
														"VALUES (\'%s\', 0, \'%04i-%02i-%02i %02i:%02i:%02i\');",
													json_System_Key->valuestring,
													p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
													p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							if(rc < 0)
							{
								m_pServer->m_pLog->Add(1, "ERROR: Al agregar [%s]", json_System_Key->valuestring);
							}
						}
						else if(rc < 0)
						{
							m_pServer->m_pLog->Add(10, "ERROR: Al actualizar [%s]", json_System_Key->valuestring);
						}
					}

					/* Me fijo si hay algo para notificar */
					json_arr = cJSON_CreateArray();
					strcpy(save_client_key, json_System_Key->valuestring);
					if(QueryForNews(pDB, json_arr, save_client_key))
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddStringToObject(json_obj, "resp_code", "0");
						cJSON_AddStringToObject(json_obj, "resp_msg", "Ok");
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);

						t = 0;
						cJSON_ArrayForEach(json_un_obj, json_arr)
						{
							json_Time_Stamp = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Time_Stamp");
							if(atol(json_Time_Stamp->valuestring) > t)
							{
								t = atol(json_Time_Stamp->valuestring);
							}
						}
						if(t > 0)
						{
							DeleteNotify(pDB, save_client_key, t);
						}
					}
					else
					{
						cJSON_Delete(json_arr);
						/* Siempre responder OK para evitar skimming */
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
				}
				else
				{
					/* Siempre responder OK para evitar skimming */
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [dompi_infoio]");
				}
				cJSON_Delete(json_obj);
			}
			/* ****************************************************************
			*		dompi_cloud_status - 
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_status"))
			{
				json_obj = cJSON_Parse(message);

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "System_Key");
				json_Accion = cJSON_GetObjectItemCaseSensitive(json_obj, "Accion");
				json_Admin = cJSON_GetObjectItemCaseSensitive(json_obj, "Admin");
				if(json_Accion)
				{
					json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
					if(json_Objeto)
					{
						t = time(&t);
						m_pServer->m_pLog->Add(50, "Establecer / Cambiar estado de Objeto: %s de %s",
							json_Objeto->valuestring,
							json_System_Key->valuestring);
						sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
											"VALUES (\'%s\', %lu, \'%s\', \'%s\') ",
											json_System_Key->valuestring,
											t,
											json_Objeto->valuestring,
											json_Accion->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto\"}}");
					}
				}
				else if(json_Admin)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					if( !strcmp(json_Admin->valuestring, "delcli"))
					{
						m_pServer->m_pLog->Add(50, "Borrar registros de: [%s]", json_System_Key->valuestring);

						sprintf(query, "DELETE FROM TB_DOMCLOUD_ASSIGN "
											"WHERE System_Key = \'%s\';", json_System_Key->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						if(rc < 0)
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto\"}}");
						}
					}
				}
				else if(json_System_Key)
				{
					m_pServer->m_pLog->Add(50, "Estado de Objetos: [%s]", json_System_Key->valuestring);
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT Objeto, Estado, Ultimo_Update "
											"FROM TB_DOMCLOUD_ASSIGN "
											"WHERE System_Key = \'%s\' AND Id > 0;", json_System_Key->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc > 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddStringToObject(json_obj, "resp_code", "0");
						cJSON_AddStringToObject(json_obj, "resp_msg", "Ok");
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Listado vacio\"}}");
						cJSON_Delete(json_arr);
					}
				}
				else
				{
					m_pServer->m_pLog->Add(50, "Listado de clientes");
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT System_Key "
											"FROM TB_DOMCLOUD_ASSIGN "
											"WHERE Id = 0;");
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc > 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddStringToObject(json_obj, "resp_code", "0");
						cJSON_AddStringToObject(json_obj, "resp_msg", "Ok");
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Listado vacio\"}}");
						cJSON_Delete(json_arr);
					}
				}
				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [dompi_cloud_status]");
				}
				cJSON_Delete(json_obj);
			}
			/* ****************************************************************
			*		dompi_cloud_check_user - 
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_check_user"))
			{
				json_obj = cJSON_Parse(message);

				json_User = cJSON_GetObjectItemCaseSensitive(json_obj, "User");
				json_Password = cJSON_GetObjectItemCaseSensitive(json_obj, "Password");
				json_Time = cJSON_GetObjectItemCaseSensitive(json_obj, "Time");
				if(json_User && json_Password && json_Time)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT Id_Sistema, Errores, Ultima_Conexion, Estado "
											"FROM TB_DOMCLOUD_USER "
											"WHERE UPPER(Usuario) = UPPER(\'%s\') AND Clave = \'%s\';",
											json_User->valuestring,
											json_Password->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc > 0)
					{
						cJSON_ArrayForEach(json_un_obj, json_arr)
						{
							json_Id_Sistema = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Id_Sistema");
							//json_Errores = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Errores");
							//json_Ultima_Conexion = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Ultima_Conexion");
							json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado");

							if( atoi(json_Estado->valuestring) == 1)
							{
								//cJSON_Delete(json_obj);
								//json_obj = cJSON_CreateObject();
								//cJSON_AddStringToObject(json_obj, "resp_code", "0");
								//cJSON_AddStringToObject(json_obj, "resp_msg", "Ok");
								//cJSON_AddItemToObject(json_obj, "response", json_arr);
								//cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
								sprintf(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\", \"sistema\":\"%s\"}}", json_Id_Sistema->valuestring);
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"9999\", \"resp_msg\":\"Auth Error\"}}");
							}
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"9999\", \"resp_msg\":\"Auth Error\"}}");
					}
					cJSON_Delete(json_arr);
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"9999\", \"resp_msg\":\"Auth error\"}}");
				}

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
				cJSON_Delete(json_obj);
			}
			/* ****************************************************************
			*		dompi_cloud_user_list
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_list"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT Usuario, Id_Sistema, Estado "
								"FROM TB_DOMCLOUD_USER;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc > 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_user_list_all
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_list_all"))
			{
				message[0] = 0;

				json_arr = cJSON_CreateArray();
				strcpy(query, "SELECT * FROM TB_DOMCLOUD_USER;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				if(rc > 0)
				{
					json_obj = cJSON_CreateObject();
					cJSON_AddItemToObject(json_obj, "response", json_arr);
					cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					cJSON_Delete(json_obj);
				}
				else
				{
					cJSON_Delete(json_arr);
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_user_get
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_get"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Usuario");
				if(json_un_obj)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT * FROM TB_DOMCLOUD_USER WHERE Usuario = %s;", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc > 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_user_add
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_add"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;

				json_un_obj = json_obj;

				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExisteColumna(json_un_obj->string, user_columns))
									{
										/* Dato */
										if(strlen(query_into) == 0)
										{
											strcpy(query_into, "(");
										}
										else
										{
											strcat(query_into, ",");
										}
										strcat(query_into, json_un_obj->string);
										/* Valor */
										if(strlen(query_values) == 0)
										{
											strcpy(query_values, "(");
										}
										else
										{
											strcat(query_values, ",");
										}
										strcat(query_values, "'");
										strcat(query_values, json_un_obj->valuestring);
										strcat(query_values, "'");
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);

				strcat(query_into, ")");
				strcat(query_values, ")");
				sprintf(query, "INSERT INTO TB_DOMCLOUD_USER %s VALUES %s;", query_into, query_values);
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);

				rc = pDB->Query(NULL, query);
				if(rc != 0)
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_user_delete
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_delete"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				json_un_obj = cJSON_GetObjectItemCaseSensitive(json_obj, "Usuario");
				if(json_un_obj)
				{
					sprintf(query, "DELETE FROM TB_DOMCLOUD_USER WHERE Usuario = \'%s\';", json_un_obj->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc != 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_user_update
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_user_update"))
			{
				json_obj = cJSON_Parse(message);
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				query[0] = 0;
				query_into[0] = 0;
				query_values[0] = 0;
				query_where[0] = 0;
// cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
				json_un_obj = json_obj;
				while( json_un_obj )
				{
					/* Voy hasta el elemento con datos */
					if(json_un_obj->type == cJSON_Object)
					{
						json_un_obj = json_un_obj->child;
					}
					else
					{
						if(json_un_obj->type == cJSON_String)
						{
							if(json_un_obj->string && json_un_obj->valuestring)
							{
								if(strlen(json_un_obj->string) && strlen(json_un_obj->valuestring))
								{
									if(ExisteColumna(json_un_obj->string, user_columns))
									{
										if( !strcmp(json_un_obj->string, "Usuario") )
										{
											strcpy(query_where, json_un_obj->string);
											strcat(query_where, "='");
											strcat(query_where, json_un_obj->valuestring);
											strcat(query_where, "'");
										}
										else
										{
											/* Dato = Valor */
											if(strlen(query_values) > 0)
											{
												strcat(query_values, ",");
											}
											strcat(query_values, json_un_obj->string);
											strcat(query_values, "='");
											strcat(query_values, json_un_obj->valuestring);
											strcat(query_values, "'");
										}
									}
								}
							}
						}
						json_un_obj = json_un_obj->next;
					}
				}
				cJSON_Delete(json_obj);
				if(strlen(query_where))
				{
					sprintf(query, "UPDATE TB_DOMCLOUD_USER SET %s WHERE %s;", query_values, query_where);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					if(rc == 0)
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"1\", \"resp_msg\":\"Database Error\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Form Data Error\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_list_objects
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_list_objects"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_sistema = cJSON_GetObjectItemCaseSensitive(json_obj, "sistema");
				json_grupo = cJSON_GetObjectItemCaseSensitive(json_obj, "grupo");
				if(json_sistema && json_grupo)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT Id,Objeto,Estado,Icono0,Icono1 "
								   "FROM TB_DOMCLOUD_ASSIGN "
					               "WHERE System_Key = \'%s\' AND Grupo_Visual = %s;",
								   json_sistema->valuestring,
								   json_grupo->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					if(rc > 0)
					{
						cJSON_Delete(json_obj);
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_arr);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
					}
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_cloud_touch_object
			**************************************************************** */
			else if( !strcmp(fn, "dompi_cloud_touch_object"))
			{
				json_obj = cJSON_Parse(message);
				message[0] = 0;
				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "sistema");
				json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "objeto");
				if(json_System_Key && json_Objeto)
				{
					t = time(&t);
					m_pServer->m_pLog->Add(50, "Cambiar estado de Objeto: %s de %s",
						json_Objeto->valuestring,
						json_System_Key->valuestring);
					sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
										"VALUES (\'%s\', %lu, \'%s\', \'switch\') ",
										json_System_Key->valuestring,
										t,
										json_Objeto->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto\"}}");
				}
				cJSON_Delete(json_obj);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
		}
	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_cloud_check_user", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_web_notif", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_status", GM_MSG_TYPE_CR);

	m_pServer->UnSuscribe("dompi_cloud_user_list", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_user_list_all", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_user_get", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_user_add", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_user_delete", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_user_update", GM_MSG_TYPE_CR);

	m_pServer->UnSuscribe("dompi_cloud_list_objects", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("dompi_cloud_touch_object", GM_MSG_TYPE_CR);

	delete pConfig;
	delete m_pServer;
	exit(0);
}
