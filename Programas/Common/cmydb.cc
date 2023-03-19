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

#define MYDB_QUERY_RETRY 2

CMyDB::CMyDB(void)
{
    m_pMYConn = NULL;
    m_host[0] = 0;
    m_dbname[0] = 0;
    m_username[0] = 0;
    m_userpass[0] = 0;
    m_connect_error_count = 0;
    m_last_query_time = 0;
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
    m_last_query_time = 0;
}

CMyDB::~CMyDB()
{
    Close();
}

int CMyDB::Open(void)
{
    if(IsOpen()) Close();

    m_last_error_text[0] = 0;

    m_last_query_time = 0;
    m_pMYConn = mysql_init(NULL);
    if(m_pMYConn == NULL)
    {
        strncpy(m_last_error_text, mysql_error(m_pMYConn), CMYDB_MAX_ERROR_TEXT);
        syslog(LOG_INFO, "CMyDB::Open(%s,%s,%s,xxxx) ERROR: [%s]", m_host, m_dbname, m_username, m_last_error_text);
        return (-1);
    }

    if( mysql_real_connect(m_pMYConn, m_host, m_username, m_userpass, m_dbname, 0, NULL, 0) == NULL )
    {
        strncpy(m_last_error_text, mysql_error(m_pMYConn), CMYDB_MAX_ERROR_TEXT);
        syslog(LOG_INFO, "CMyDB::Open(%s,%s,%s,xxxx) ERROR: [%s]", m_host, m_dbname, m_username, m_last_error_text);
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




int CMyDB::Begin(void)
{
    int rc;

    rc = Query(nullptr, "SET autocommit = OFF;");
    if(rc == 0)
    {
      rc = Query(nullptr, "START TRANSACTION;");
    }
    return rc;
}

int CMyDB::Query(cJSON *json_array, const char* query_fmt, ...)
{
    va_list arg;
    int i;
    int rc;
    char query[32768];
    int num_fields;
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL_FIELD *field;
    char row_names[256][256];
    cJSON *json_obj;
    time_t t1, t2;
    int retry;

    va_start(arg, query_fmt);
    vsprintf(query, query_fmt, arg);
    va_end(arg);

    m_last_error_text[0] = 0;

    if(!IsOpen()) Open();

    t1 = time(&t1);
    m_last_query_time = 0;
    retry = 0;
    rc = 0;

    while(retry < MYDB_QUERY_RETRY)
    {
        retry++;

        if(mysql_real_query(m_pMYConn, query, strlen(query)) != 0)
        {
            strncpy(m_last_error_text, mysql_error(m_pMYConn), CMYDB_MAX_ERROR_TEXT);
            syslog(LOG_INFO, "CMyDB::Query([%s]) ERROR: [%s] %s", query, m_last_error_text, (retry < MYDB_QUERY_RETRY)?"Reintentando...":"Error Fatal");
            Open();
            rc = (-1);
            continue;
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
                    cJSON_AddStringToObject(json_obj,row_names[i], (row[i])?row[i]:"NULL");
                }
                rc++;
                cJSON_AddItemToArray(json_array, json_obj);
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
                strncpy(m_last_error_text, mysql_error(m_pMYConn), CMYDB_MAX_ERROR_TEXT);
                syslog(LOG_INFO, "CMyDB::Query([%s]) ERROR: [%s] %s", query, m_last_error_text, (retry < MYDB_QUERY_RETRY)?"Reintentando...":"Error Fatal");
                Open();
                rc = (-1);
                continue;
            }
        }
        break;
    }

    t2 = time(&t2);

    m_last_query_time = t2 - t1;

    return rc;
}

int CMyDB::Commit(void)
{
    int rc;

    rc = Query(nullptr, "COMMIT;");
    Query(nullptr, "SET autocommit = ON;");

    return rc;
}

int CMyDB::Rollback(void)
{
    int rc;

    rc = Query(nullptr, "COMMIT;");
    Query(nullptr, "SET autocommit = ON;");

    return rc;
}

long CMyDB::NextId(const char* table_name, const char* row_name)
{
    long rc = 0;
    char query[1024];
    MYSQL_RES *result;
    MYSQL_ROW row;
    int retry = 0;

    while(retry < MYDB_QUERY_RETRY)
    {
        retry++;

        m_last_error_text[0] = 0;

        sprintf(query, "SELECT MAX(%s) FROM %s;", row_name, table_name);

        if(!IsOpen()) Open();

        if(mysql_real_query(m_pMYConn, query, strlen(query)) != 0)
        {
            strncpy(m_last_error_text, mysql_error(m_pMYConn), CMYDB_MAX_ERROR_TEXT);
            syslog(LOG_INFO, "CMyDB::Query([%s]) ERROR: [%s] %s", query, m_last_error_text, (retry < MYDB_QUERY_RETRY)?"Reintentando...":"Error Fatal");
            Open();
            rc = (-1);
            continue;
        }
        result = mysql_store_result(m_pMYConn);
        if (result)  // there are rows
        {
            if(mysql_num_fields(result) == 1)
            {
                row = mysql_fetch_row(result);
                rc = atol(row[0])+1;
            }
            mysql_free_result(result);
        }
        break;
    }

    return rc;
}



char* CMyDB::LastErrorMsg(char* msg)
{
    if(msg) strcpy(msg, mysql_error(m_pMYConn));
    return msg;    
}

long CMyDB::LastQueryTime()
{
    return m_last_query_time;
}

void CMyDB::Manten(void)
{

}
