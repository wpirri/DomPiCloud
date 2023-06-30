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
	char str_tmp[256];
	char *p;

	time_t t;
	struct tm *p_tm;

	STRFunc Strf;
	//CPgDB *pDB;
	CMyDB *pDB;

    cJSON *json_Message;

	cJSON *json_Request;
	cJSON *json_Data;
	cJSON *json_User;
	cJSON *json_User_Id;
    cJSON *json_Response;
	cJSON *json_Directive;
	cJSON *json_Header;
	cJSON *json_EndPoint;
	cJSON *json_EndPoint_Id;
	cJSON *json_Query_Row;
	cJSON *json_Id_Sistema;

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
	m_pServer->Suscribe("amazon_TurnOn", GM_MSG_TYPE_CR);
	m_pServer->Suscribe("amazon_TurnOff", GM_MSG_TYPE_CR);

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
			* Discover
			**************************************************************** */
			if( !strcmp(fn, "amazon_Discover"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Data = cJSON_GetObjectItemCaseSensitive(json_Request, "data")) != nullptr )
					{
						if((json_User = cJSON_GetObjectItemCaseSensitive(json_Request, "user")) != nullptr )
						{
							if((json_User_Id = cJSON_GetObjectItemCaseSensitive(json_User, "user_id")) != nullptr )
							{
								if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Data, "directive")) != nullptr )
								{
									if((json_EndPoint = cJSON_GetObjectItemCaseSensitive(json_Directive, "endpoint")) != nullptr )
									{
										/* Busco el sistema por el cliente  */
										sprintf(query, "SELECT Id_Sistema "
											"FROM TB_DOMCLOUD_USER "
											"WHERE Amazon_Key = \'%s\';", json_User_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										json_Query_Result = cJSON_CreateArray();
										rc = pDB->Query(json_Query_Result, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
										if(rc > 0)
										{
											cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
											if(json_Query_Row)
											{
												if((json_Id_Sistema = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id_Sistema")) != nullptr )
												{
													if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Data, "directive")) != nullptr )
													{
														if((json_Header = cJSON_GetObjectItemCaseSensitive(json_Directive, "header")) != nullptr )
														{
															sprintf(query, "SELECT Id, Objeto, Tipo, Grupo_Visual "
																					"FROM TB_DOMCLOUD_ASSIGN "
																					"WHERE System_Key = \'%s\' AND Id > 0;", 
																					json_Id_Sistema->valuestring);
															m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
															cJSON_Delete(json_Query_Result);
															json_Query_Result = cJSON_CreateArray();
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
													strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno Id_Sistema\"}}");
												}
											}
											else
											{
												strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Usuario no relacionado con sistema\"}}");
											}
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno USUARIO\"}}");
										}
										cJSON_Delete(json_Query_Result);
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
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user_id\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto data\"}}");
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
			/* ****************************************************************
			* ReportState
			**************************************************************** */
			else if( !strcmp(fn, "amazon_ReportState"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Data = cJSON_GetObjectItemCaseSensitive(json_Request, "data")) != nullptr )
					{
						if((json_User = cJSON_GetObjectItemCaseSensitive(json_Request, "user")) != nullptr )
						{
							if((json_User_Id = cJSON_GetObjectItemCaseSensitive(json_User, "user_id")) != nullptr )
							{
								if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Data, "directive")) != nullptr )
								{
									if((json_EndPoint = cJSON_GetObjectItemCaseSensitive(json_Directive, "endpoint")) != nullptr )
									{
										/* Busco el sistema por el cliente  */
										sprintf(query, "SELECT Id_Sistema "
											"FROM TB_DOMCLOUD_USER "
											"WHERE Amazon_Key = \'%s\';", json_User_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										json_Query_Result = cJSON_CreateArray();
										rc = pDB->Query(json_Query_Result, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
										if(rc > 0)
										{
											cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
											if(json_Query_Row)
											{
												if((json_Id_Sistema = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id_Sistema")) != nullptr )
												{
													if((json_EndPoint_Id = cJSON_GetObjectItemCaseSensitive(json_EndPoint, "endpointId")) != nullptr )
													{
														/* Reemplazo '-' por ' ' en el Id */
														strcpy(str_tmp, json_EndPoint_Id->valuestring);
														p = &str_tmp[0];
														while(*p)
														{
															if(*p == '-') *p = ' ';
															p++;
														}
														sprintf(query, "SELECT Id, Objeto, Estado, Ultimo_Update "
																				"FROM TB_DOMCLOUD_ASSIGN "
																				"WHERE System_Key = \'%s\' AND UPPER(Objeto) = UPPER(\'%s\') AND Id > 0;", 
																				json_Id_Sistema->valuestring, str_tmp);
														m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
														cJSON_Delete(json_Query_Result);
														json_Query_Result = cJSON_CreateArray();
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
															strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno ASSIGN\"}}");
														}
													}
													else
													{
														strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta objeto endpointId\"}}");
													}
												}
												else
												{
													strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno Id_Sistema\"}}");
												}
											}
											else
											{
												strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Usuario no relacionado con sistema\"}}");
											}
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno USUARIO\"}}");
										}
										cJSON_Delete(json_Query_Result);
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
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user_id\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto data\"}}");
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
			/* ****************************************************************
			* TurnOn
			**************************************************************** */
			else if( !strcmp(fn, "amazon_TurnOn"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Data = cJSON_GetObjectItemCaseSensitive(json_Request, "data")) != nullptr )
					{
						if((json_User = cJSON_GetObjectItemCaseSensitive(json_Request, "user")) != nullptr )
						{
							if((json_User_Id = cJSON_GetObjectItemCaseSensitive(json_User, "user_id")) != nullptr )
							{
								if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Data, "directive")) != nullptr )
								{
									if((json_EndPoint = cJSON_GetObjectItemCaseSensitive(json_Directive, "endpoint")) != nullptr )
									{
										/* Busco el sistema por el cliente  */
										sprintf(query, "SELECT Id_Sistema "
											"FROM TB_DOMCLOUD_USER "
											"WHERE Amazon_Key = \'%s\';", json_User_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										json_Query_Result = cJSON_CreateArray();
										rc = pDB->Query(json_Query_Result, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
										if(rc > 0)
										{
											cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
											if(json_Query_Row)
											{
												if((json_Id_Sistema = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id_Sistema")) != nullptr )
												{
													if((json_EndPoint_Id = cJSON_GetObjectItemCaseSensitive(json_EndPoint, "endpointId")) != nullptr )
													{
														/* Reemplazo '-' por ' ' en el Id */
														strcpy(str_tmp, json_EndPoint_Id->valuestring);
														p = &str_tmp[0];
														while(*p)
														{
															if(*p == '-') *p = ' ';
															p++;
														}
														sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
																			"VALUES (\'%s\', %lu, \'%s\', \'on\');",
																			json_Id_Sistema->valuestring,
																			t,
																			str_tmp);
														m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
														rc = pDB->Query(NULL, query);
														m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
														if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

														sprintf(message, "{ \"response\": {\"resp_code\": \"0\", \"resp_msg\": \"Ok\", \"Objeto\": \"%s\", \"Estado\": \"1\", \"Ultimo_Update\": \"%04i-%02i-%02i %02i:%02i:%02i\" } }",
																str_tmp,
																p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
																p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
													}
													else
													{
														strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta objeto endpointId\"}}");
													}
												}
												else
												{
													strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno Id_Sistema\"}}");
												}
											}
											else
											{
												strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Usuario no relacionado con sistema\"}}");
											}
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno USUARIO\"}}");
										}
										cJSON_Delete(json_Query_Result);
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
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user_id\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto data\"}}");
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
			/* ****************************************************************
			* TurnOff
			**************************************************************** */
			else if( !strcmp(fn, "amazon_TurnOff"))
			{
				json_Message = cJSON_Parse(message);
				message[0] = 0;

				if((json_Request = cJSON_GetObjectItemCaseSensitive(json_Message, "request")) != nullptr )
				{
					if((json_Data = cJSON_GetObjectItemCaseSensitive(json_Request, "data")) != nullptr )
					{
						if((json_User = cJSON_GetObjectItemCaseSensitive(json_Request, "user")) != nullptr )
						{
							if((json_User_Id = cJSON_GetObjectItemCaseSensitive(json_User, "user_id")) != nullptr )
							{
								if((json_Directive = cJSON_GetObjectItemCaseSensitive(json_Data, "directive")) != nullptr )
								{
									if((json_EndPoint = cJSON_GetObjectItemCaseSensitive(json_Directive, "endpoint")) != nullptr )
									{
										/* Busco el sistema por el cliente  */
										sprintf(query, "SELECT Id_Sistema "
											"FROM TB_DOMCLOUD_USER "
											"WHERE Amazon_Key = \'%s\';", json_User_Id->valuestring);
										m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
										json_Query_Result = cJSON_CreateArray();
										rc = pDB->Query(json_Query_Result, query);
										m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
										if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);
										if(rc > 0)
										{
											cJSON_ArrayForEach(json_Query_Row, json_Query_Result) { break; }
											if(json_Query_Row)
											{
												if((json_Id_Sistema = cJSON_GetObjectItemCaseSensitive(json_Query_Row, "Id_Sistema")) != nullptr )
												{
													if((json_EndPoint_Id = cJSON_GetObjectItemCaseSensitive(json_EndPoint, "endpointId")) != nullptr )
													{
														/* Reemplazo '-' por ' ' en el Id */
														strcpy(str_tmp, json_EndPoint_Id->valuestring);
														p = &str_tmp[0];
														while(*p)
														{
															if(*p == '-') *p = ' ';
															p++;
														}
														sprintf(query, "INSERT INTO TB_DOMCLOUD_NOTIF (System_Key, Time_Stamp, Objeto, Accion) "
																			"VALUES (\'%s\', %lu, \'%s\', \'off\');",
																			json_Id_Sistema->valuestring,
																			t,
																			str_tmp);
														m_pServer->m_pLog->Add(100, "[QUERY][%s]", query);
														rc = pDB->Query(NULL, query);
														m_pServer->m_pLog->Add((pDB->LastQueryTime()>1)?1:100, "[QUERY] rc= %i, time= %li [%s]", rc, pDB->LastQueryTime(), query);
														if(rc < 0) m_pServer->m_pLog->Add(1, "[QUERY] ERROR [%s] en [%s]", pDB->m_last_error_text, query);

														sprintf(message, "{ \"response\": {\"resp_code\": \"0\", \"resp_msg\": \"Ok\", \"Objeto\": \"%s\", \"Estado\": \"0\", \"Ultimo_Update\": \"%04i-%02i-%02i %02i:%02i:%02i\" } }",
																str_tmp,
																p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday,
																p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
													}
													else
													{
														strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta objeto endpointId\"}}");
													}
												}
												else
												{
													strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno Id_Sistema\"}}");
												}
											}
											else
											{
												strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Usuario no relacionado con sistema\"}}");
											}
										}
										else
										{
											strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Error interno USUARIO\"}}");
										}
										cJSON_Delete(json_Query_Result);
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
								strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user_id\"}}");
							}
						}
						else
						{
							strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto user\"}}");
						}
					}
					else
					{
						strcpy(message, "{\"response\":{\"resp_code\":\"10\", \"resp_msg\":\"Falta Objeto data\"}}");
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
	m_pServer->UnSuscribe("amazon_TurnOn", GM_MSG_TYPE_CR);
	m_pServer->UnSuscribe("amazon_TurnOff", GM_MSG_TYPE_CR);

	delete pConfig;
	delete m_pServer;
	exit(0);
}
