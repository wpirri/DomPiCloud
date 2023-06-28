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

void OnClose(int sig);

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
	char sistema[256];
	char str_tmp[256];
	char *p;

	time_t t;
	struct tm *p_tm;

	STRFunc Strf;
	//CPgDB *pDB;
	CMyDB *pDB;

    cJSON *json_Message;

	cJSON *json_Request;
    cJSON *json_Response;
	cJSON *json_Directive;
	cJSON *json_Header;
	cJSON *json_Payload;
	cJSON *json_Scope;
	cJSON *json_NameSpace;
	cJSON *json_Name;
	cJSON *json_Scope_Type;
	cJSON *json_Scope_Token;
	cJSON *json_EndPoint;
	cJSON *json_EndPoint_Id;
	cJSON *json_Query_Row;

	cJSON *json_Query_Result;

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
	m_pServer->Init("dompi_amazon_server");
	m_pServer->m_pLog->Add(1, "Iniciando Servidor interface con Amazon...");

	m_pServer->m_pLog->Add(10, "Leyendo configuración...");
	pConfig = new DPConfig("/etc/dompicloud.config");
	pConfig->GetParam("DBHOST", db_host);
	pConfig->GetParam("DBNAME", db_name);
	pConfig->GetParam("DBUSER", db_user);
	pConfig->GetParam("DBPASSWORD", db_password);

	m_pServer->m_pLog->Add(10, "Conectando a la base de datos...");
	pDB = new CMyDB(db_host, db_name, db_user, db_password);
	if(pDB == NULL)
	{
		m_pServer->m_pLog->Add(1, "ERROR: Al conectarse a la base de datos.");
	}
	if(pDB->Open() != 0)
	{
		m_pServer->m_pLog->Add(1, "ERROR: En Open a la base de datos.");
	}
	else
	{
		//m_pServer->m_pLog->Add(10, "Conectado a la base de datos %s", db_filename);
		m_pServer->m_pLog->Add(10, "Conectado a la base de datos %s en %s.", db_name, db_host);
	}

	m_pServer->Suscribe("amazon_Discover", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("amazon_ReportState", GM_MSG_TYPE_CR);
	

	m_pServer->m_pLog->Add(1, "Servicios de interface con Amazon inicializados.");

	while((rc = m_pServer->Wait(fn, typ, message, MAX_BUFFER_LEN, &message_len, 3000 )) >= 0)
	{
		if(rc > 0)
		{
			message[message_len] = 0;
			m_pServer->m_pLog->Add(90, "%s:(Q)[%s]", fn, message);
			t = time(&t);
			p_tm = localtime(&t);

			/* ****************************************************************
			*		dompi_amazon_Discover - 
			**************************************************************** */
			if( !strcmp(fn, "amazon_Discover"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				/* Requerimiento
					query = 
						{
						"CONTENT_LENGTH":"632",
						"REMOTE_ADDR":"3.236.68.226",
						"REQUEST_METHOD":"POST",
						"REQUEST_URI":"/cgi-bin/dompi_cloud_amazon.cgi/?funcion=Discover",
						"request":
							{
							"directive":
								{
								"header":
									{
									"namespace":"Alexa.Discovery",
									"name":"Discover",
									"payloadVersion":"3",
									"messageId":"02fc7372-88f9-43a0-bcc0-5bcc73b09a1f"
									},
								"payload":
									{
									"scope":
										{
										"type":"BearerToken",
										"token":"Atza|........"
										}
									}
								}
							}
						}
				*/

				/* Respuesta
					response = 
						{ 
						"event": 
							{ 
							"header": request["directive"]["header"],
							"payload": {
				===========================================================================================
								"endpoints": 
									[
										{ 	
										"endpointId": "luz-cocina",
										"manufacturerName": "WGP",
										"friendlyName": "luz cocina",
										"description": "Virtual smart light bulb",
										"displayCategories": ["LIGHT"],
										"additionalAttributes":  {
										"manufacturer" : "WGP",
										"model" : "DomPiWeb",
										"serialNumber": "DPW1",
										"firmwareVersion" : "1.00",
										"softwareVersion": "1.00",
										"customIdentifier": "DPW-1.00"
										},
										"cookie": 
										{
										"key1": "-",
										"key2": "-",
										"key3": "-",
										"key4": "-"
										},
										"capabilities": 
											[
												{
												"interface": "Alexa.PowerController",
												"version": "3",
												"type": "AlexaInterface",
												"properties":
													{
													"supported": 
														[
															{
															"name": "powerState"
															}
														],
													"retrievable": "true"
													}
												},
												{
												"type": "AlexaInterface",
												"interface": "Alexa.EndpointHealth",
												"version": "3.2",
												"properties":
													{
													"supported": 
														[
															{
															"name": "connectivity"
															}
														],
													"retrievable": "true"
													}
												},
												{
												"type": "AlexaInterface",
												"interface": "Alexa",
												"version": "3"
												}
											]
										}
									]
				===========================================================================================
								}
							}
						}
				*/
				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Request, "directive")) != nullptr )
					{
						json_Header = cJSON_GetObjectItemCaseSensitive(json_Directive, "header");
						json_Payload = cJSON_GetObjectItemCaseSensitive(json_Directive, "payload");
						if(json_Header && json_Payload)
						{
							json_NameSpace = cJSON_GetObjectItemCaseSensitive(json_Header, "namespace");
							json_Name = cJSON_GetObjectItemCaseSensitive(json_Header, "name");
							if((json_Scope = cJSON_GetObjectItemCaseSensitive(json_Payload, "scope")) != nullptr)
							{
								json_Scope_Type = cJSON_GetObjectItemCaseSensitive(json_Scope, "type");
								json_Scope_Token = cJSON_GetObjectItemCaseSensitive(json_Scope, "token");
								if(json_Scope_Type && json_Scope_Token)
								{
									if( !strcmp(json_Name->valuestring, "Discover") && !strcmp(json_NameSpace->valuestring, "Alexa.Discovery") )
									{

										json_Query_Result = cJSON_CreateArray();
										strcpy(sistema, "D3S4RR0LL0-0001");
										sprintf(query, "SELECT Id, Objeto, Tipo, Grupo_Visual "
																"FROM TB_DOMCLOUD_ASSIGN "
																"WHERE System_Key = \'%s\' AND Id > 0;", sistema);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										rc = pDB->Query(json_Query_Result, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
										if(rc > 0)
										{
											if(cJSON_IsArray(json_Query_Result))
											{
												json_Response = cJSON_CreateObject();
												cJSON_AddItemToObject(json_Response, "response", json_Query_Result);
												cJSON_PrintPreallocated(json_Response, message, MAX_BUFFER_LEN, 0);
											}
											else
											{
												strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno\"}}");
											}
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno\"}}");
										}
										cJSON_Delete(json_Query_Result);
									}
									else
									{
										strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error de datos en requerimiento\"}}");
									}
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto type/token\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto scope\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto header/payload\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto directive\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto request\"}}");
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
				cJSON_Delete(json_Message);
			}
			else if( !strcmp(fn, "amazon_ReportState"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;
				/* Requerimiento
					query = 
						{
						"CONTENT_LENGTH":"632",
						"REMOTE_ADDR":"3.236.68.226",
						"REQUEST_METHOD":"POST",
						"REQUEST_URI":"/cgi-bin/dompi_cloud_amazon.cgi/?funcion=ReportState",
						"request":
							{
								"directive": {
									"header": {
										"namespace": "Alexa",
										"name": "ReportState",
										"payloadVersion": "3",
										"messageId": "6ce37ae0-4418-4ed4-bd83-075c6c0bc74c",
										"correlationToken": "AAAAAAAAAADEVbPXc+NW3PHt/5PhzDkeAAIAAAAAAAC5+aP9qYktPhDiowA07FQlUDwYQjIRGeUFY7vJU+ht0d1kS5TTouRvpyfXXtpYjfBr3HDl9tpE7DwVanbPAZfxJpGJOkoAXTif51/3ECvGIswc+2RUhXKeti3U3AH+s2W2T/xvIE/90l5V7C2NyFmBngxRthvT5/eb5ko3uYlOsXw3QqAJxaQBwjfU+FXwQiMna7eNKU6m0f14927ksjpwvRstC/qREW5AA3V42W8LloutsIAePTm7QPB4tDTfHgVvgeQaVnz+Jz5p4/yXhwffylRvfn5d9d3ZdgDVXed9FlhbPU5/OrDq0fvHyPgRUY9W28olCdbtMGeBbYnSMJT9rSnx8IgnaGUMg6NHrqot9cwyveUL4nFzGB1T52GDVu/eRGWcMpJaXGoXfcLsDKOEo4V1ZNaU00+N9JIZCehtpfKVuu+DxY/BEXzIKLSRnft9m33ktf/OKL+oOyYVIKr1LFxnFvo4+SDwHWsE28vUbtQilodsteyrxf6wjqQ5xsgZqgVU7CnEuPJbxffIz/VddRmUFy/mte2jfoAgm17/OmLjA3jsjO+Ga7zNr85JztWQR/f+ik9L6kh7KTOAW7i9gjfZwQacPZI/QX/CyQpWBs5d2xWbK1xAGnt9OxAVCluu42RQUnH5UKPHF787Xuyd0UEg4ERPG2x9I07unyM9SQ=="
									},
									"endpoint": {
										"scope": {
											"type": "BearerToken",
											"token": "Atza|IwEBIJ1WbPdybs6OeB8Y0zEFqMwtDQ-mOgvEgDGe4eir_McFEpmX3I3nHzXM2d_l59wMDCopGparf21FSLbiU-5BbEh29sEybOIxmBt51TEgoD88TGh_WS8xX4ftOdo-TYVK-vMqDRnqvei_tLc0z4uHJ9jgQj9f6qugHu0hI79jBGipNR-u_-vw3dNwWQJfkoCQ9I-w4OtkbS_85zUQ7mWt0ql49DJCfavbCVJ-YjMMj9Y7TkCyJmCK0MnryODq6n9SBb_xnKX-ooxCsWttNICkLZQLcP3CLtwq5a5onXDXbBClZ3P7EPhXEgBFSLgXvvVlQ4I_VxvO9EccAT9DqVBtozOQeY7eX93aa3DsuT6Vj5aE_JXahtcPoqLwU458HQryWfU"
										},
										"endpointId": "luz-cocina", <---
										"cookie": {
											"key1": "-",
											"key2": "-",
											"key3": "-",
											"key4": "-"
										}
									},
									"payload": {}
								}
							}
						}
				*/

				/* Respuesta
					{
					"event": {
						"header": {
						"namespace": "Alexa",
						"name": "StateReport",
						"messageId": "Unique identifier, preferably a version 4 UUID",
						"correlationToken": "Opaque correlation token that matches the request",
						"payloadVersion": "3"
						},
						"endpoint": {
						"scope": {
							"type": "BearerToken",
							"token": "OAuth2.0 bearer token"
						},
						"endpointId": "Endpoint ID"
						},
						"payload": {}
					},
					"context": {
				===========================================================================================
						"properties": [
						{
							"namespace": "Alexa.PowerController",
							"name": "powerState",
							"value": "OFF",
							"timeOfSample": "2017-02-03T16:20:50.52Z",
							"uncertaintyInMilliseconds": 0
						}
						]
				===========================================================================================
					}
					}
				*/
				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Request, "directive")) != nullptr )
					{
						json_EndPoint = cJSON_GetObjectItemCaseSensitive(json_Directive, "endpoint");
						if(json_EndPoint)
						{
							json_EndPoint_Id = cJSON_GetObjectItemCaseSensitive(json_EndPoint, "endpointId");
							if((json_Scope = cJSON_GetObjectItemCaseSensitive(json_EndPoint, "scope")) != nullptr)
							{
								json_Scope_Type = cJSON_GetObjectItemCaseSensitive(json_Scope, "type");
								json_Scope_Token = cJSON_GetObjectItemCaseSensitive(json_Scope, "token");
								if(json_EndPoint_Id && json_Scope_Type && json_Scope_Token)
								{
									json_Query_Result = cJSON_CreateArray();
									/* Reemplazo '-' por ' ' en el Id */
									strcpy(str_tmp, json_EndPoint_Id->valuestring);
									p = &str_tmp[0];
									while(*p)
									{
										if(*p == '-') *p = ' ';
										p++;
									}
									strcpy(sistema, "D3S4RR0LL0-0001");
									sprintf(query, "SELECT Id, Objeto, Estado, Ultimo_Update "
															"FROM TB_DOMCLOUD_ASSIGN "
															"WHERE System_Key = \'%s\' AND UPPER(Objeto) = UPPER(\'%s\') AND Id > 0;", sistema, str_tmp);
									m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
									rc = pDB->Query(json_Query_Result, query);
									m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
									if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
									if(rc > 0)
									{
										cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }

										if(json_Query_Row)
										{
											json_Response = cJSON_CreateObject();
											cJSON_AddItemToObject(json_Response, "response", json_Query_Row);
											cJSON_PrintPreallocated(json_Response, message, MAX_BUFFER_LEN, 0);
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno\"}}");
										}
									}
									else
									{
										strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno\"}}");
									}
									cJSON_Delete(json_Query_Result);
								}
								else
								{
									strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto type/token/endpointId\"}}");
								}
							}
							else
							{
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto scope\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto header/payload\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto directive\"}}");
					}
				}
				else
				{
					strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto request\"}}");
				}
				m_pServer->m_pLog->Add(90, "%s:(R)[%s]", fn, message);
				if(m_pServer->Resp(message, strlen(message), GME_OK) != GME_OK)
				{
					/* error al responder */
					m_pServer->m_pLog->Add(1, "ERROR al responder mensaje [%s]", fn);
				}
				cJSON_Delete(json_Message);
			}



			/* último else por Servicio no encontrado */
			else
			{
				m_pServer->m_pLog->Add(50, "GME_SVC_NOTFOUND");
				m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
			}
			/* Procesamiento luego de procesar un mensaje recibido */





		}
		else
		{
			/* Vencimiento del timer */




		}
		/* Se procesó un mensaje o time-out */






	}
	m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
	OnClose(0);
	return 0;
}

void OnClose(int sig)
{
	m_pServer->m_pLog->Add(1, "Exit on signal %i", sig);

	m_pServer->UnSuscribe("amazon_Discover", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("amazon_ReportState", GM_MSG_TYPE_CR);

	delete pConfig;
	delete m_pServer;
	exit(0);
}
