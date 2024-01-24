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
//CPgDB *pDB;
CMyDB *pDB;

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

	time_t t;
	struct tm *p_tm;
	char save_client_key[256];

	STRFunc Strf;

    cJSON *json_Message;
    cJSON *json_arr;
    cJSON *json_un_obj;
    cJSON *json_System_Key;
    cJSON *json_AssId;
    cJSON *json_Array_Alarma;
    cJSON *json_Alarma;
	cJSON *json_Estado;
	cJSON *json_Time_Stamp;
	cJSON *json_User;
	cJSON *json_Password;
	cJSON *json_Amazon_Key;
	cJSON *json_Google_Key;
	cJSON *json_Apple_Key;
	cJSON *json_Other_Key;
	cJSON *json_Id_Alarma;
	cJSON *json_Nombre;
	cJSON *json_Estado_Activacion;
	cJSON *json_Estado_Memoria;
	cJSON *json_Estado_Alarma;
	cJSON *json_Entradas;
	cJSON *json_Entrada;
	cJSON *json_Salidas;
	cJSON *json_Salida;
	cJSON *json_Id_Zona;
	cJSON *json_Nombre_Zona;
	cJSON *json_Tipo_Zona;
	cJSON *json_Grupo_Zona;
	cJSON *json_Zona_Activa;
	cJSON *json_Estado_Zona;
	cJSON *json_Id_Salida;
	cJSON *json_Nombre_Salida;
	cJSON *json_Tipo_Salida;
	cJSON *json_Estado_Salida;

	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, OnClose);
	signal(SIGTERM, OnClose);
	/* Dejo de capturar interrupciones para permitir Core Dumps */
	//signal(SIGSTOP, OnClose);
	//signal(SIGABRT, OnClose);
	//signal(SIGQUIT, OnClose);
	//signal(SIGINT,  OnClose);
	//signal(SIGILL,  OnClose);
	//signal(SIGFPE,  OnClose);
	//signal(SIGSEGV, OnClose);
	//signal(SIGBUS,  OnClose);

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

				json_Message = cJSON_Parse(message);

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "System_Key");
				json_AssId = cJSON_GetObjectItemCaseSensitive(json_Message, "Id");
				json_Array_Alarma = cJSON_GetObjectItemCaseSensitive(json_Message, "Alarma");
				if(!json_AssId)
				{
					json_AssId = cJSON_GetObjectItemCaseSensitive(json_Message, "ASS_Id");
				}

				json_User = cJSON_GetObjectItemCaseSensitive(json_Message, "Usuario_Cloud");
				json_Password = cJSON_GetObjectItemCaseSensitive(json_Message, "Clave_Cloud");

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
						cJSON_Delete(json_Message);

						continue;
					}

					if(json_User && json_Password) /* Actualizacion de usuario del sistema */
					{
						json_Amazon_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "Amazon_Key");
						json_Google_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "Google_Key");
						json_Apple_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "Apple_Key");
						json_Other_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "Other_Key");
						json_Estado = cJSON_GetObjectItemCaseSensitive(json_Message, "Estado");
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
					else if(json_Array_Alarma)
					{
						/* Actualizacion de estado de alarma */
						m_pServer->m_pLog->Add(100, "[*** Update Alarma ***] System_Key: %s", json_System_Key->valuestring);
						/*
						{"Alarma":[
							{"Id":"1","Nombre":"Casa","Estado_Activacion":"0","Estado_Memoria":"0","Estado_Alarma":"0",
								"Zonas":[
									{"Id":"1","Objeto":"Zona 1","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
									{"Id":"2","Objeto":"Zona 2","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
									{"Id":"3","Objeto":"Zona 3","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
									{"Id":"4","Objeto":"Zona 4","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
									{"Id":"5","Objeto":"Zona 5","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
									{"Id":"6","Objeto":"Zona 6","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"}],
								"Salidas":[
									{"Id":"1","Objeto":"Sirena Exterior","Tipo_Salida":"0","Estado":"0"}]},
							{"Id":"2","Nombre":"Taller","Estado_Activacion":"0","Estado_Memoria":"0","Estado_Alarma":"0"}],
						"System_Key":"D3S4RR0LL0-0001"}
						*/
						/* Viene informado un array de particiones de alarma */
						cJSON_ArrayForEach(json_Alarma, json_Array_Alarma)
						{
							json_Id_Alarma = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Id");
							json_Nombre = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Nombre");
							json_Estado_Activacion = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Estado_Activacion");
							json_Estado_Memoria = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Estado_Memoria");
							json_Estado_Alarma = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Estado_Alarma");

							if(json_Id_Alarma && json_Nombre && json_Estado_Activacion && json_Estado_Memoria && json_Estado_Alarma)
							{
								sprintf(query, "UPDATE TB_DOMCLOUD_ALARM "
												"SET Particion = \'%s\', "
													"Estado_Activacion = %s, "
													"Estado_Memoria = %s, "
													"Estado_Alarma = %s, "
													"Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\' "
												"WHERE System_Key = \'%s\' AND Id = %s;", 
												json_Nombre->valuestring,
												json_Estado_Activacion->valuestring,
												json_Estado_Memoria->valuestring,
												json_Estado_Alarma->valuestring,
												p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
												p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
												json_System_Key->valuestring,
												json_Id_Alarma->valuestring);
								m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
								rc = pDB->Query(NULL, query);
								m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
								if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								if(rc == 0)
								{
									/* Si no se actualiza nada va un insert */
									sprintf(query, "INSERT INTO TB_DOMCLOUD_ALARM (System_Key,Id,Particion,Estado_Activacion,Estado_Memoria,Estado_Alarma,Ultimo_Update) "
														"VALUES (\'%s\',%s,\'%s\',%s,%s,%s,\'%04i-%02i-%02i %02i:%02i:%02i\');",
													json_System_Key->valuestring,
													json_Id_Alarma->valuestring,
													json_Nombre->valuestring,
													json_Estado_Activacion->valuestring,
													json_Estado_Memoria->valuestring,
													json_Estado_Alarma->valuestring,
													p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
													p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(NULL, query);
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
								}
							}
							/* Si actualizó o insertó sigo con las entradas y salidas */
							if(rc == 1)
							{
								/* Entradas / Zonas */
								/*
								"Zonas":[
									{"Id":"1","Objeto":"Zona 1","Tipo_Zona":"0","Grupo":"0","Activa":"1","Estado":"0"},
								*/
								json_Entradas = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Zonas");
								if(json_Entradas)
								{
									cJSON_ArrayForEach(json_Entrada, json_Entradas)
									{
										json_Id_Zona = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Id");
										json_Nombre_Zona = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Objeto");
										json_Tipo_Zona = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Tipo_Zona");
										json_Grupo_Zona = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Grupo");
										json_Zona_Activa = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Activa");
										json_Estado_Zona = cJSON_GetObjectItemCaseSensitive(json_Entrada, "Estado");

										if(	json_Id_Zona && json_Nombre_Zona && json_Tipo_Zona && json_Grupo_Zona && json_Zona_Activa && json_Estado_Zona )
										{
											sprintf(query, "UPDATE TB_DOMCLOUD_ALARM_ENTRADA "
															"SET Entrada = \'%s\', "
																"Tipo_Entrada = %s, "
																"Grupo = %s, "
																"Activa = %s, "
																"Estado_Entrada = %s, "
																"Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\' "
															"WHERE System_Key = \'%s\' AND Id = %s AND Particion = %s;", 
															json_Nombre_Zona->valuestring,
															json_Tipo_Zona->valuestring,
															json_Grupo_Zona->valuestring,
															json_Zona_Activa->valuestring,
															json_Estado_Zona->valuestring,
															p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
															p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
															json_System_Key->valuestring,
															json_Id_Zona->valuestring,
															json_Id_Alarma->valuestring);
											m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
											rc = pDB->Query(NULL, query);
											m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
											if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
											if(rc == 0)
											{
												/* Si no se actualiza nada va un insert */
												sprintf(query, "INSERT INTO TB_DOMCLOUD_ALARM_ENTRADA (System_Key,Id,Particion,Entrada,Tipo_Entrada,Grupo,Activa,Estado_Entrada,Ultimo_Update) "
																"VALUES ( \'%s\',%s,%s,\'%s\',%s,%s,%s,%s,\'%04i-%02i-%02i %02i:%02i:%02i\');",
																json_System_Key->valuestring,
																json_Id_Zona->valuestring,
																json_Id_Alarma->valuestring,
																json_Nombre_Zona->valuestring,
																json_Tipo_Zona->valuestring,
																json_Grupo_Zona->valuestring,
																json_Zona_Activa->valuestring,
																json_Estado_Zona->valuestring,
																p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
																p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
												m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
												rc = pDB->Query(NULL, query);
												m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
												if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
											}
										}
									}
								}
								/*
								"Salidas":[
									{"Id":"1","Objeto":"Sirena Exterior","Tipo_Salida":"0","Estado":"0"}]},
								*/
								/* Salidas */
								json_Salidas = cJSON_GetObjectItemCaseSensitive(json_Alarma, "Salidas");
								if(json_Salidas)
								{
									cJSON_ArrayForEach(json_Salida, json_Salidas)
									{
										json_Id_Salida = cJSON_GetObjectItemCaseSensitive(json_Salida, "Id");
										json_Nombre_Salida = cJSON_GetObjectItemCaseSensitive(json_Salida, "Objeto");
										json_Tipo_Salida = cJSON_GetObjectItemCaseSensitive(json_Salida, "Tipo_Salida");
										json_Estado_Salida = cJSON_GetObjectItemCaseSensitive(json_Salida, "Estado");

										if( json_Id_Salida && json_Nombre_Salida && json_Tipo_Salida && json_Estado_Salida )
										{
											sprintf(query, "UPDATE TB_DOMCLOUD_ALARM_SALIDA "
															"SET Salida = \'%s\', "
																"Tipo_Salida = %s, "
																"Estado_Salida = %s, "
																"Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\' "
															"WHERE System_Key = \'%s\' AND Id = %s AND Particion = %s;", 
															json_Nombre_Salida->valuestring,
															json_Tipo_Salida->valuestring,
															json_Estado_Salida->valuestring,
															p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
															p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
															json_System_Key->valuestring,
															json_Id_Salida->valuestring,
															json_Id_Alarma->valuestring);
											m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
											rc = pDB->Query(NULL, query);
											m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
											if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
											if(rc == 0)
											{
												/* Si no se actualiza nada va un insert */
												sprintf(query, "INSERT INTO TB_DOMCLOUD_ALARM_SALIDA (System_Key,Id,Particion,Salida,Tipo_Salida,Estado_Salida,Ultimo_Update) "
																	"VALUES (\'%s\',%s,%s,\'%s\',%s,%s,\'%04i-%02i-%02i %02i:%02i:%02i\');",
															json_System_Key->valuestring,
															json_Id_Salida->valuestring,
															json_Id_Alarma->valuestring,
															json_Nombre_Salida->valuestring,
															json_Tipo_Salida->valuestring,
															json_Estado_Salida->valuestring,
															p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
															p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
												m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
												rc = pDB->Query(NULL, query);
												m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
												if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
											}
										}
									}
								}
							}
						}
					}
					else if(json_AssId)
					{
						query[0] = 0;
						query_into[0] = 0;
						query_values[0] = 0;
						query_set[0] = 0;
						query_where[0] = 0;

						m_pServer->m_pLog->Add(100, "[*** Update Objeto ***] System_Key: %s", json_System_Key->valuestring);

						json_un_obj = json_Message;
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
						cJSON_Delete(json_Message);
						json_Message = cJSON_CreateObject();
						cJSON_AddStringToObject(json_Message, "resp_code", "0");
						cJSON_AddStringToObject(json_Message, "resp_msg", "Ok");
						cJSON_AddItemToObject(json_Message, "response", json_arr);
						cJSON_PrintPreallocated(json_Message, message, MAX_BUFFER_LEN, 0);

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
				cJSON_Delete(json_Message);
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

	delete m_pServer;
	delete pConfig;
	delete pDB;
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
