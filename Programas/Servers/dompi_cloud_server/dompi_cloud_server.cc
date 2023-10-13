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
	"Icono_Apagado",
	"Icono_Encendido",
	"Icono_Auto",
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
int QueryForNews(CMyDB* db, cJSON *response_array, const char *client_key);
void DeleteNotify(CMyDB *db, const char *client_key, time_t t);
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
	char query_set[2048];
	char query_where[512];
	char str_tmp[1024];

	time_t t;
	struct tm *p_tm;
	char save_client_key[256];

	STRFunc Strf;
	//CPgDB *pDB;
	CMyDB *pDB;

    cJSON *json_obj;
    cJSON *json_arr;
    cJSON *json_row;
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
	cJSON *json_Sistema;
	cJSON *json_Grupo;
	cJSON *json_Tipo;
	cJSON *json_Amazon_Key;
	cJSON *json_Google_Key;
	cJSON *json_Apple_Key;
	cJSON *json_Other_Key;

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
	pConfig->GetParam("DBNAME", db_name);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

	m_pServer->Suscribe("dompi_web_notif", GM_MSG_TYPE_CR);

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

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, (-1) )) >= 0)
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

				json_User = cJSON_GetObjectItemCaseSensitive(json_obj, "Usuario_Cloud");
				json_Password = cJSON_GetObjectItemCaseSensitive(json_obj, "Clave_Cloud");

				if(json_System_Key)
				{
					if( !strcmp(json_System_Key->valuestring, "CALLEYNUM00000-CP0000"))
					{
						/* No proceso mensajes de sistemas no configurados */
						strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
						if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
						{
							/* error al responder */
							m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
						}
						cJSON_Delete(json_obj);

						continue;
					}

					if(json_Id)
					{
						query[0] = 0;
						query_into[0] = 0;
						query_values[0] = 0;
						query_set[0] = 0;
						query_where[0] = 0;

						m_pServer->m_pLog->Add(100, "[*** Update Objeto ***] System_Key: %s", json_System_Key->valuestring);

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
											if( !ExcluirDeABM(json_un_obj->string) && ExisteColumna(json_un_obj->string, assign_columns))
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
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								}
							}
						}
					}
					else if(json_User && json_Password) /* Actualizacion de usuario del sistema */
					{
						json_Amazon_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "Amazon_Key");
						json_Google_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "Google_Key");
						json_Apple_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "Apple_Key");
						json_Other_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "Other_Key");
						json_Estado = cJSON_GetObjectItemCaseSensitive(json_obj, "Estado");
						m_pServer->m_pLog->Add(100, "[*** Update User ***] %s de System_Key: %s", json_User->valuestring, json_System_Key->valuestring);
						sprintf(query, "UPDATE TB_DOMCLOUD_USER "
													"SET Clave = \'%s\', Id_Sistema = \'%s\', Estado = \'%s\',  "
													"Amazon_Key = \'%s\', Google_Key = \'%s\', Apple_Key = \'%s\', Other_Key = \'%s\' "
													"WHERE Usuario = \'%s\';",
												json_Password->valuestring,
												json_System_Key->valuestring,
												(json_Estado)?json_Estado->valuestring:"0",
												(json_Amazon_Key)?json_Amazon_Key->valuestring:"",
												(json_Google_Key)?json_Google_Key->valuestring:"",
												(json_Apple_Key)?json_Apple_Key->valuestring:"",
												(json_Other_Key)?json_Other_Key->valuestring:"",
												json_User->valuestring);
						m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
						rc = pDB->Query(NULL, query);
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc == 0)
						{
							sprintf(query, "INSERT INTO TB_DOMCLOUD_USER (Usuario, Clave, Id_Sistema, Estado, Amazon_Key, Google_Key, Apple_Key, Other_Key, Errores, Ultima_Conexion) "
														"VALUES (\'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', 0, 0);",
													json_User->valuestring,
													json_Password->valuestring,
													json_System_Key->valuestring,
													(json_Estado)?json_Estado->valuestring:"0",
													(json_Amazon_Key)?json_Amazon_Key->valuestring:"",
													(json_Google_Key)?json_Google_Key->valuestring:"",
													(json_Apple_Key)?json_Apple_Key->valuestring:"",
													(json_Other_Key)?json_Other_Key->valuestring:"");
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							if(rc < 0)
							{
								m_pServer->m_pLog->Add(1, "ERROR: Al agregar [%s a %s]", json_User->valuestring, json_System_Key->valuestring);
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
						m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
						if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
						if(rc == 0)
						{
							sprintf(query, "INSERT INTO TB_DOMCLOUD_ASSIGN (System_Key, Id, Ultimo_Update) "
														"VALUES (\'%s\', 0, \'%04i-%02i-%02i %02i:%02i:%02i\');",
													json_System_Key->valuestring,
													p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
													p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
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
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
				cJSON_Delete(json_obj);
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
	m_pServer->UnSuscribe("dompi_web_notif", GM_MSG_TYPE_CR);

	delete pConfig;
	delete m_pServer;
	exit(0);
}

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
