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


void OnClose(int sig);
int ExcluirDeABM(const char* label);
int ExisteColumna(const char* columna, const char** lista);

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[MAX_BUFFER_LEN+1];
	unsigned long message_len;

	char db_host[32];
	char db_name[32];
	char db_user[32];
	char db_password[32];
	char query[4096];
	char query_into[2048];
	char query_values[2048];
	char query_where[512];
	char str_tmp[1024];

	time_t t;

	STRFunc Strf;
	//CPgDB *pDB;
	CMyDB *pDB;

    cJSON *json_obj;
    cJSON *json_arr;
    cJSON *json_row;
    cJSON *json_un_obj;
    cJSON *json_System_Key;
	cJSON *json_Objeto;
	cJSON *json_Estado;
	cJSON *json_User;
	cJSON *json_Password;
	cJSON *json_Time;
	cJSON *json_Id_Sistema;
	cJSON *json_Sistema;
	cJSON *json_Grupo;
	cJSON *json_Tipo;

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
	m_pServer->Init("dompi_cloud_abm");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de ABM Nube de Domotica...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompicloud.config");
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBNAME", db_name);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

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
	pDB = new CMyDB(db_host, db_name, db_user, db_password);
	if(pDB == NULL)
	{
		m_pServer->m_pLog->Add(1, "ERROR: Al conectarse a la base de datos.");
	}
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR: En Open a la base de datos.");
	}

	m_pServer->m_pLog->Add(1, "Servicios de Domotica en la nube inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 6000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(100, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_cloud_status - 
			**************************************************************** */
			if( !strcmp(fn, "dompi_cloud_status"))
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
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
								"FROM TB_DOMCLOUD_USER "
								"ORDER BY Usuario ASC;");
				m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
				rc = pDB->Query(json_arr, query);
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						/* reemplazo las claves por 16* */
						cJSON_ArrayForEach(json_row, json_arr)
						{
							if(cJSON_GetObjectItemCaseSensitive(json_row, "Clave"))
							{
								cJSON_DeleteItemFromObject(json_row, "Clave");
								cJSON_AddStringToObject(json_row, "Clave", "****************");
							}
						}
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
									if( !ExcluirDeABM(json_un_obj->string))
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
				m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
				if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
				if(rc < 0)
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc < 0)
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
									if( !ExcluirDeABM(json_un_obj->string))
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
											if(	strcmp(json_un_obj->string, "Clave") || 
												strcmp(json_un_obj->valuestring, "****************") )
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
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc < 0)
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
				json_Sistema = cJSON_GetObjectItemCaseSensitive(json_obj, "sistema");
				json_Grupo = cJSON_GetObjectItemCaseSensitive(json_obj, "grupo");
				if(json_Sistema && json_Grupo)
				{
					json_arr = cJSON_CreateArray();
					sprintf(query, "SELECT Id,Objeto,Estado,Icono_Apagado,Icono_Encendido,Icono_Auto "
								   "FROM TB_DOMCLOUD_ASSIGN "
					               "WHERE System_Key = \'%s\' AND Grupo_Visual = %s "
								   "ORDER BY Objeto ASC;",
								   json_Sistema->valuestring,
								   json_Grupo->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
				str_tmp[0] = 0;

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "sistema");
				json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "objeto");
				if(json_System_Key && json_Objeto)
				{
					json_arr = cJSON_CreateArray();
					m_pServer->m_pLog->Add(50, "Touch Objeto: %s de %s",
						json_Objeto->valuestring,
						json_System_Key->valuestring);
					sprintf(query, "SELECT * FROM TB_DOMCLOUD_ASSIGN "
									"WHERE System_Key = \'%s\' AND Objeto = \'%s\';",
									json_System_Key->valuestring,
									json_Objeto->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_arr, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						cJSON_ArrayForEach(json_un_obj, json_arr) { break; }
						json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado");
						json_Tipo = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo");
						if(json_Estado && json_Tipo)
						{
							if(atoi(json_Tipo->valuestring) == 5)	/* Salida de pulso */
							{
								strcpy(str_tmp, "pulse");
							}
							else if(atoi(json_Tipo->valuestring) >= 10) /* Automatismo */
							{
								strcpy(str_tmp, "switch");
							}
							else if(atoi(json_Estado->valuestring)) /* el resto */
							{
								strcpy(str_tmp, "off");
							}
							else
							{
								strcpy(str_tmp, "on");
							}
						}
						else
						{
							strcpy(str_tmp, "switch");
						}
						t = time(&t);
						sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
											"VALUES (\'%s\', %lu, \'%s\', \'%s\');",
											json_System_Key->valuestring,
											t,
											json_Objeto->valuestring,
											str_tmp);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Objeto no encontrado\"}}");
					}
					cJSON_Delete(json_arr);
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Objeto no definido\"}}");
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

		/* Borro registros viejos no reclamados */
		t = time(&t);
		sprintf(query, "DELETE FROM TB_DOMCLOUD_NOTIF WHERE Time_Stamp < %lu;", t-60);
		m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
		rc = pDB->Query(NULL, query);
		m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
		if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);
	m_pServer->UnSuscribe("dompi_cloud_check_user", GM_MSG_TYPE_CR);
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

void DeleteNotify(CMyDB *db, const char *client_key, time_t t)
{
	char query[4096];
	m_pServer->m_pLog->Add(100, "[DeleteNotify] System_Key = [%s].", client_key);
	sprintf(query, "DELETE FROM TB_DOMCLOUD_NOTIF "
					"WHERE System_Key = \'%s\' AND Time_Stamp <= %lu;", client_key, t);
	m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
	db->Query(NULL, query);
}

int ExcluirDeABM(const char* label)
{
	if( !strcmp(label, "REMOTE_ADDR")) return 1;
	if( !strcmp(label, "REQUEST_URI")) return 1;
	if( !strcmp(label, "REQUEST_METHOD")) return 1;
	if( !strcmp(label, "CONTENT_LENGTH")) return 1;
	if( !strcmp(label, "GET")) return 1;
	if( !strcmp(label, "POST")) return 1;
	return 0;
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
