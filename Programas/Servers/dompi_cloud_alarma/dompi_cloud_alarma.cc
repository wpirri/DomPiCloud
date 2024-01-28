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
	char str_tmp[1024];

	time_t t;

	STRFunc Strf;

    cJSON *json_Message;
    cJSON *json_Query_Result;
    cJSON *json_System_Key;
    cJSON *json_EstadoPart;
    cJSON *json_EstadoZona;
    cJSON *json_EstadoSalida;
    cJSON *json_obj;
    cJSON *json_Part;
    cJSON *json_Part_Id;
    cJSON *json_Zona;
    cJSON *json_Salida;
	cJSON *json_Estado;
    cJSON *json_un_obj;

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
	m_pServer->Init("dompi_cloud_alarma");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor de administracion de Alarma...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompicloud.config");
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBNAME", db_name);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

    m_pServer->Suscribe("dompi_alarm_part_list", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_part_switch", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_zona_switch", GM_MSG_TYPE_CR);
    m_pServer->Suscribe("dompi_alarm_salida_pulse", GM_MSG_TYPE_CR);

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

	m_pServer->m_pLog->Add(1, "Servidor de administracion de Alarma inicializado.");

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 6000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(100, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_alarm_part_list
			**************************************************************** */
			if( !strcmp(fn, "dompi_alarm_part_list"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "sistema");
				if(json_System_Key)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT Id, Particion AS Nombre, Estado_Activacion AS Activada, Estado_Memoria AS Memoria, Estado_Alarma "
									"FROM TB_DOMCLOUD_ALARM "
									"WHERE System_Key = \'%s\' "
									"ORDER BY Nombre ASC;", json_System_Key->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					cJSON_Delete(json_Message);
					if(rc >= 0)
					{
						json_obj = cJSON_CreateObject();
						cJSON_AddItemToObject(json_obj, "response", json_Query_Result);
						cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
						cJSON_Delete(json_obj);
					}
					else
					{
						cJSON_Delete(json_Query_Result);
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_status
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_status"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				json_EstadoPart = nullptr;
				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "sistema");
				json_Part = cJSON_GetObjectItemCaseSensitive(json_Message, "part");

				if(json_System_Key && json_Part)
				{
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT Id, Particion AS Nombre, Estado_Activacion, Estado_Memoria AS Memoria, Estado_Alarma "
									"FROM TB_DOMCLOUD_ALARM "
									"WHERE Particion = \'%s\' AND System_Key = \'%s\';", json_Part->valuestring, json_System_Key->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						/* Paso el primero y único */
						cJSON_ArrayForEach(json_EstadoPart, json_Query_Result) { break; }
					}

					if(rc > 0 && json_EstadoPart)
					{
						json_Part_Id = cJSON_GetObjectItemCaseSensitive(json_EstadoPart, "Id");
						if(json_Part_Id)
						{
							/* Información de Zonas */
							json_EstadoZona = cJSON_CreateArray();
							sprintf(query, "SELECT Id, Entrada AS Objeto, Tipo_Entrada AS Tipo_Zona, Grupo, Activa, Estado_Entrada AS Estado "
											"FROM TB_DOMCLOUD_ALARM_ENTRADA "
											"WHERE Particion = \'%s\' AND System_Key = \'%s\';", json_Part_Id->valuestring, json_System_Key->valuestring);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(json_EstadoZona, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							if(rc >= 0)
							{
								cJSON_AddItemToObject(json_EstadoPart, "Zonas", json_EstadoZona);
							}

							json_EstadoSalida = cJSON_CreateArray();
							sprintf(query, "SELECT Id, Salida AS Objeto, Tipo_Salida, Estado_Salida AS Estado "
											"FROM TB_DOMCLOUD_ALARM_SALIDA "
											"WHERE Particion = \'%s\' AND System_Key = \'%s\';", json_Part_Id->valuestring, json_System_Key->valuestring);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(json_EstadoSalida, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							if(rc >= 0)
							{
								cJSON_AddItemToObject(json_EstadoPart, "Salidas", json_EstadoSalida);
							}
						}
					}

					if(json_EstadoPart)
					{
						if(json_EstadoPart)
						{
							json_obj = cJSON_CreateObject();
							cJSON_AddItemToObject(json_obj, "response", json_EstadoPart);
							cJSON_PrintPreallocated(json_obj, message, MAX_BUFFER_LEN, 0);
							cJSON_Delete(json_obj);
						}
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Faltan Datos\"}}");
				}

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_part_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_part_switch"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				str_tmp[0] = 0;

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "sistema");
				json_Part = cJSON_GetObjectItemCaseSensitive(json_Message, "part");
				if(json_System_Key && json_Part)
				{
					m_pServer->m_pLog->Add(50, "Activar / Desactivar Alarma: %s de %s",
						json_Part->valuestring,
						json_System_Key->valuestring);
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT Estado_Activacion FROM TB_DOMCLOUD_ALARM "
									"WHERE System_Key = \'%s\' AND Particion = \'%s\';",
									json_System_Key->valuestring,
									json_Part->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						cJSON_ArrayForEach(json_un_obj, json_Query_Result) { break; }
						json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado_Activacion");
						if(json_Estado)
						{
							if(atoi(json_Estado->valuestring) > 0) /* Alarma activada */
							{
								strcpy(str_tmp, "desactivar");
							}
							else
							{
								strcpy(str_tmp, "total");
							}
							t = time(&t);
							sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
												"VALUES (\'%s\', %lu, \'%s\', \'%s\');",
												json_System_Key->valuestring,
												t,
												json_Part->valuestring,
												str_tmp);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Estado no encontrado\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Particion no encontrada\"}}");
					}
					cJSON_Delete(json_Query_Result);
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Objeto no definido\"}}");
				}
				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_zona_switch
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_zona_switch"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				str_tmp[0] = 0;

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "sistema");
				json_Zona = cJSON_GetObjectItemCaseSensitive(json_Message, "zona");
				json_Part = cJSON_GetObjectItemCaseSensitive(json_Message, "part");
				if(json_System_Key && json_Part && json_Zona)
				{
					m_pServer->m_pLog->Add(50, "Activar / Desactivar Zona: %s de %s de %s",
						json_Zona->valuestring,
						json_Part->valuestring,
						json_System_Key->valuestring);
					json_Query_Result = cJSON_CreateArray();
					sprintf(query, "SELECT E.Activa "
										"FROM TB_DOMCLOUD_ALARM AS A, TB_DOMCLOUD_ALARM_ENTRADA AS E "
										"WHERE A.Id = E.Particion AND A.System_Key = E.System_Key AND "
										"A.System_Key = \'%s\' AND A.Particion = \'%s\' AND UPPER(E.Entrada) = UPPER(\'%s\');",
										json_System_Key->valuestring,
										json_Part->valuestring,
										json_Zona->valuestring);
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(json_Query_Result, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					if(rc > 0)
					{
						cJSON_ArrayForEach(json_un_obj, json_Query_Result) { break; }
						json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Activa");
						if(json_Estado)
						{
							if(atoi(json_Estado->valuestring) > 0) /* Entrada habilitada */
							{
								strcpy(str_tmp, "inhabilitar");
							}
							else
							{
								strcpy(str_tmp, "habilitar");
							}
							t = time(&t);
							sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
												"VALUES (\'%s\', %lu, \'%s:%s\', \'%s\');",
												json_System_Key->valuestring,
												t,
												json_Part->valuestring, json_Zona->valuestring,
												str_tmp);
							m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
							rc = pDB->Query(NULL, query);
							m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
							if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
							strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Estado no encontrado\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"2\", \"resp_msg\":\"Entrada no encontrada\"}}");
					}
					cJSON_Delete(json_Query_Result);
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Objeto no definido\"}}");
				}
				cJSON_Delete(json_Message);

				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
			}
			/* ****************************************************************
			*		dompi_alarm_salida_pulse
			**************************************************************** */
			else if( !strcmp(fn, "dompi_alarm_salida_pulse"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				str_tmp[0] = 0;

				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_Message, "sistema");
				json_Salida = cJSON_GetObjectItemCaseSensitive(json_Message, "salida");
				if(json_System_Key && json_Salida)
				{
					m_pServer->m_pLog->Add(50, "Activar / Pulso a: %s de %s",
						json_Salida->valuestring,
						json_System_Key->valuestring);
					t = time(&t);
					sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
										"VALUES (\'%s\', %lu, \'%s\', \'%s\');",
										json_System_Key->valuestring,
										t,
										json_Salida->valuestring,
										"pulse");
					m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
					rc = pDB->Query(NULL, query);
					m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
					if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
					strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Objeto no definido\"}}");
				}
				cJSON_Delete(json_Message);

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
    m_pServer->UnSuscribe("dompi_alarm_part_list", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_status", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_part_switch", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_zona_switch", GM_MSG_TYPE_CR);
    m_pServer->UnSuscribe("dompi_alarm_salida_pulse", GM_MSG_TYPE_CR);

	delete m_pServer;
	delete pConfig;
	delete pDB;
	exit(0);
}
