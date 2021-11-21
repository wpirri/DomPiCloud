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
#include "cpgdb.h"

CGMServerWait *m_pServer;
DPConfig *pConfig;

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

int main(/*int argc, char** argv, char** env*/void)
{
	int rc;
	char fn[33];
	char typ[1];
	char message[4096];
	unsigned long message_len;

	char db_host[32];
	char db_user[32];
	char db_password[32];

	time_t t;
	struct tm *p_tm;

	STRFunc Strf;
	CPgDB *pDB;

    cJSON *json_obj;
    cJSON *json_un_obj;
    cJSON *json_System_Key;
    cJSON *json_Objetos;
    cJSON *json_Id;
	cJSON *json_Objeto;
	cJSON *json_Tipo;
	cJSON *json_Estado;
	cJSON *json_Icono0;
	cJSON *json_Icono1;
	cJSON *json_Grupo_Visual;
	cJSON *json_Planta;
	cJSON *json_Cord_x;
	cJSON *json_Cord_y;
	cJSON *json_Coeficiente;
	cJSON *json_Analog_Mult_Div;
	cJSON *json_Analog_Mult_Div_Valor;
	cJSON *json_Flags;

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

	m_pServer->m_pLog->Add(1, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompicloud.config");
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

	m_pServer->Suscribe("dompi_web_notif", GM_MSG_TYPE_CR);

	m_pServer->m_pLog->Add(1, "Conectado a la base de datos...");
	pDB = new CPgDB(db_host, "DB_DOMPICLOUD", db_user, db_password);
	if(pDB == NULL)
	{
		m_pServer->m_pLog->Add(1, "ERROR: Al conectarse a la base de datos.");
	}
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR: En Open a la base de datos.");
	}

	m_pServer->m_pLog->Add(1, "Servicios de Domotica inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, 4096, &message_len, 1000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(50, "%s:(Q)[%s]", fn, message);
			/* ****************************************************************
			*		dompi_web_notif - 
			**************************************************************** */
			if( !strcmp(fn, "dompi_web_notif"))
			{
				json_obj = cJSON_Parse(message);
				json_System_Key = cJSON_GetObjectItemCaseSensitive(json_obj, "System_Key");
				json_Id = cJSON_GetObjectItemCaseSensitive(json_obj, "Id");
				if(json_Id)
				{
					//json_Objeto = cJSON_GetObjectItemCaseSensitive(json_obj, "Objeto");
					//json_Tipo = cJSON_GetObjectItemCaseSensitive(json_obj, "Tipo");
					json_Estado = cJSON_GetObjectItemCaseSensitive(json_obj, "Estado");
					json_Objetos = NULL;
				}
				else
				{
					json_Objetos = cJSON_GetObjectItemCaseSensitive(json_obj, "Objetos");
				}

				if(json_System_Key)
				{
					if(json_Id && json_Estado)
					{
						m_pServer->m_pLog->Add(10, "Status Update: [%s]", json_System_Key->valuestring);
						t = time(&t);
						p_tm = localtime(&t);
						m_pServer->m_pLog->Add(10, "Keep Alive: [%s]", json_System_Key->valuestring);
						rc = pDB->Query(NULL, "UPDATE TB_DOMCLOUD_ASSIGN "
													"SET Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\', "
													"Estado = %s "
													"WHERE System_Key = \'%s\' AND Id = %s;",
												p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
												p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
												json_Estado->valuestring,
												json_System_Key->valuestring,
												json_Id->valuestring);
						if(rc == 0)
						{
							rc = pDB->Query(NULL, "INSERT INTO TB_DOMCLOUD_ASSIGN (System_Key, Id, Ultimo_Update, Estado) "
														"VALUES (\'%s\', %s, \'%04i-%02i-%02i %02i:%02i:%02i\', %s);",
													json_System_Key->valuestring,
													json_Id->valuestring,
													p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
													p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
													json_Estado->valuestring);
							if(rc < 0)
							{
								m_pServer->m_pLog->Add(10, "ERROR: Al agregar [%s]", json_System_Key->valuestring);
							}
						}
						else if(rc < 0)
						{
							m_pServer->m_pLog->Add(10, "ERROR: Al actualizar [%s]", json_System_Key->valuestring);
						}
					}
					else if(json_Objetos)
					{
						m_pServer->m_pLog->Add(10, "Objects Update: [%s]", json_System_Key->valuestring);
						t = time(&t);
						p_tm = localtime(&t);

						cJSON_ArrayForEach(json_un_obj, json_Objetos)
						{
							json_Id = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Id");
							json_Objeto = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Objeto");
							json_Tipo = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Tipo");
							json_Estado = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Estado");
							json_Icono0 = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Icono0");
							json_Icono1 = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Icono1");
							json_Grupo_Visual = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Grupo_Visual");
							json_Planta = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Planta");
							json_Cord_x = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Cord_x");
							json_Cord_y = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Cord_y");
							json_Coeficiente = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Coeficiente");
							json_Analog_Mult_Div = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Analog_Mult_Div");
							json_Analog_Mult_Div_Valor = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Analog_Mult_Div_Valor");
							json_Flags = cJSON_GetObjectItemCaseSensitive(json_un_obj, "Flags");
							if(json_Id && json_Objeto && json_Tipo && json_Estado && json_Icono0 && json_Icono1 && 
								json_Grupo_Visual && json_Planta && json_Cord_x && json_Cord_y && json_Coeficiente && 
								json_Analog_Mult_Div && json_Analog_Mult_Div_Valor && json_Flags && json_System_Key && json_Id)
							{
								/* A pegarle a la base */
								rc = pDB->Query(NULL, "UPDATE TB_DOMCLOUD_ASSIGN "
															"SET Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\', "
															"Objeto = \'%s\', "
															"Tipo = %s, "
															"Estado = %s, "
															"Icono0 = \'%s\', "
															"Icono1 = \'%s\', "
															"Grupo_Visual = %s, "
															"Planta = %s, "
															"Cord_x = %s, "
															"Cord_y = %s, "
															"Coeficiente = %s, "
															"Analog_Mult_Div = %s, "
															"Analog_Mult_Div_Valor = %s, "
															"Flags = %s "
															"WHERE System_Key = \'%s\' AND Id = %s;",
														p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
														p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
														json_Objeto->valuestring,
														json_Tipo->valuestring,
														json_Estado->valuestring,
														json_Icono0->valuestring,
														json_Icono1->valuestring,
														json_Grupo_Visual->valuestring,
														json_Planta->valuestring,
														json_Cord_x->valuestring,
														json_Cord_y->valuestring,
														json_Coeficiente->valuestring,
														json_Analog_Mult_Div->valuestring,
														json_Analog_Mult_Div_Valor->valuestring,
														json_Flags->valuestring,
														json_System_Key->valuestring,
														json_Id->valuestring);
								if(rc == 0)
								{
									rc = pDB->Query(NULL, "INSERT INTO TB_DOMCLOUD_ASSIGN (System_Key, Id, Ultimo_Update, Objeto, Tipo, Estado, Icono0, Icono1, Grupo_Visual, Planta, Cord_x, Cord_y, Coeficiente, Analog_Mult_Div, Analog_Mult_Div_Valor, Flags) "
																"VALUES (\'%s\', %s, \'%04i-%02i-%02i %02i:%02i:%02i\', \'%s\', %s, %s, \'%s\', \'%s\', %s, %s, %s, %s, %s, %s, %s, %s );",
															json_System_Key->valuestring,
															json_Id->valuestring,
															p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
															p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
															json_Objeto->valuestring,
															json_Tipo->valuestring,
															json_Estado->valuestring,
															json_Icono0->valuestring,
															json_Icono1->valuestring,
															json_Grupo_Visual->valuestring,
															json_Planta->valuestring,
															json_Cord_x->valuestring,
															json_Cord_y->valuestring,
															json_Coeficiente->valuestring,
															json_Analog_Mult_Div->valuestring,
															json_Analog_Mult_Div_Valor->valuestring,
															json_Flags->valuestring);
									if(rc < 0)
									{
										m_pServer->m_pLog->Add(10, "ERROR: Al agregar [%s de %s]", json_Objeto->valuestring, json_System_Key->valuestring);
									}
								}
								else if(rc < 0)
								{
									m_pServer->m_pLog->Add(10, "ERROR: Al actualizar [%s de %s]", json_Objeto->valuestring, json_System_Key->valuestring);
								}
							}
							else
							{
								m_pServer->m_pLog->Add(10, "ERROR: Array de datos imcompleto de [%s]", json_System_Key->valuestring);
							}
						} /* cJSON_ArrayForEach(...) */
					}
					else /* Solo vino la Key */
					{
						t = time(&t);
						p_tm = localtime(&t);
						m_pServer->m_pLog->Add(10, "Keep Alive: [%s]", json_System_Key->valuestring);
						rc = pDB->Query(NULL, "UPDATE TB_DOMCLOUD_ASSIGN "
													"SET Ultimo_Update = \'%04i-%02i-%02i %02i:%02i:%02i\' "
													"WHERE System_Key = \'%s\' AND Id = 0;",
												p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
												p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec,
												json_System_Key->valuestring);
						if(rc == 0)
						{
							rc = pDB->Query(NULL, "INSERT INTO TB_DOMCLOUD_ASSIGN (System_Key, Id, Ultimo_Update) "
														"VALUES (\'%s\', 0, \'%04i-%02i-%02i %02i:%02i:%02i\');",
													json_System_Key->valuestring,
													p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
													p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
							if(rc < 0)
							{
								m_pServer->m_pLog->Add(10, "ERROR: Al agregar [%s]", json_System_Key->valuestring);
							}
						}
						else if(rc < 0)
						{
							m_pServer->m_pLog->Add(10, "ERROR: Al actualizar [%s]", json_System_Key->valuestring);
						}
					}
				}

				/* Siempre responder OK para evitar skimming */
				strcpy(message, "{\"response\":{\"resp_code\":\"0\", \"resp_msg\":\"Ok\"}}");

				m_pServer->m_pLog->Add(50, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(50, "ERROR al responder mensaje [dompi_infoio]");
				}
				cJSON_Delete(json_obj);
			}
		}
	}
	m_pServer->m_pLog->Add(50, "ERROR en la espera de mensajes");
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
