/***************************************************************************
  Copyright (C) 2020   Walter Pirri
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
#include "cmydb.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>
#include <syslog.h>

#define __TRACE_ON

CMyDB::CMyDB(void)
{
  m_pMYConn = NULL;
  m_host[0] = 0;
  m_dbname[0] = 0;
  m_username[0] = 0;
  m_userpass[0] = 0;
  m_connect_error_count = 0;
}

CMyDB::CMyDB(const char* host, const char* dbname, const char* username, const char* userpass)
{
  m_pMYConn = NULL;
  m_host[0] = 0;
  m_dbname[0] = 0;
  m_username[0] = 0;
  m_userpass[0] = 0;
  if(host) strcpy(m_host, host);
  if(dbname) strcpy(m_dbname, dbname);
  if(username) strcpy(m_username, username);
  if(userpass) strcpy(m_userpass, userpass);
  m_connect_error_count = 0;
}

CMyDB::~CMyDB()
{
  Close();
}

int CMyDB::Open(void)
{
    if(IsOpen()) Close();

    m_pMYConn = mysql_init(NULL);
    if(m_pMYConn == NULL)
    {
        syslog(LOG_INFO, "CMyDB::Open(%s,%s,%s,xxxx) ERROR: [%s]", m_host, m_dbname, m_username, mysql_error(m_pMYConn));
        return (-1);
    }

    if( mysql_real_connect(m_pMYConn, m_host, m_username, m_userpass, m_dbname, 0, NULL, 0) == NULL )
    {
        syslog(LOG_INFO, "CMyDB::Open(%s,%s,%s,xxxx) ERROR: [%s]", m_host, m_dbname, m_username, mysql_error(m_pMYConn));
        return (-1);
    }

    return 0;
}

int CMyDB::Open(const char* host, const char* dbname, const char* username, const char* userpass)
{
  if(host) strcpy(m_host, host);
  if(dbname) strcpy(m_dbname, dbname);
  if(username) strcpy(m_username, username);
  if(userpass) strcpy(m_userpass, userpass);
  return this->Open();    
}

int CMyDB::Begin(void)
{
    return (-1);
}

int CMyDB::Query(cJSON *json_array, const char* query_fmt, ...)
{
    va_list arg;
    int i;
    int rc;
    char query[30000];
    int num_fields;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    char row_names[256][256];
    cJSON *json_obj;

    if(!IsOpen())
    {
        return(-1);
    }
    va_start(arg, query_fmt);
    vsprintf(query, query_fmt, arg);
    va_end(arg);

    if(!IsOpen()) Open();

#ifdef TRACE_ON
    syslog(LOG_INFO, "CMyDB::Query([%s])", query);
#endif

    if(mysql_real_query(m_pMYConn, query, strlen(query)) != 0)
    {
        syslog(LOG_INFO, "CMyDB::Query([%s]) ERROR: [%s]", query, mysql_error(m_pMYConn));
        return (-1);        
    }
    result = mysql_store_result(m_pMYConn);
    if (result)  // there are rows
    {
        num_fields = mysql_num_fields(result);
        for(i = 0; i < num_fields; i++)
        {
          field = mysql_fetch_field(result);
          if(field)
          {
#ifdef TRACE_ON
            syslog(LOG_INFO, "CMyDB::Query Columna: %i Nombre: %s", i+1, field->name);
#endif
            strcpy(row_names[i], field->name);
          }
        }
        rc = 0;
        /* TODO: Falta trerse los datos y generar el JSON */
        while((row = mysql_fetch_row(result)) != NULL)
        {
          json_obj = cJSON_CreateObject();
          for(i = 0; i < num_fields; i++)
          {
#ifdef TRACE_ON
            syslog(LOG_INFO, "CMyDB::Query Fila: %i Columna: %i Valor: %s", rc+1, i+1, row[i]);
#endif
						cJSON_AddStringToObject(json_obj,row_names[i], row[i]);
          }
          rc++;
          cJSON_AddItemToArray(json_array, json_obj);
          //cJSON_Delete(json_obj);
        }
        mysql_free_result(result);
    }
    else  // mysql_store_result() returned nothing; should it have?
    {
        if(mysql_field_count(m_pMYConn) == 0)
        {
            // query does not return data
            // (it was not a SELECT)
            rc = mysql_affected_rows(m_pMYConn);
        }
        else // mysql_store_result() should have returned data
        {
          syslog(LOG_INFO, "CMyDB::Query([%s]) ERROR: [%s]", query, mysql_error(m_pMYConn));
          return (-1);        
        }
    }
    return rc;
}

int CMyDB::Commit(void)
{
    return (-1);
}

int CMyDB::Rollback(void)
{
    return (-1);
}

char* CMyDB::LastErrorMsg(char* msg)
{
    if(msg) strcpy(msg, mysql_error(m_pMYConn));
    return msg;    
}

int CMyDB::IsOpen(void)
{
  if(m_pMYConn == NULL) return 0;
  return 1;
}

void CMyDB::Close(void)
{
  if(m_pMYConn)
  {
    mysql_close(m_pMYConn);
    m_pMYConn = NULL;
  }
}
