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
#include "cpgdb.h"

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

CPgDB::CPgDB(void)
{
  m_pPGConn = NULL;
  m_host[0] = 0;
  m_dbname[0] = 0;
  m_username[0] = 0;
  m_userpass[0] = 0;
  m_connect_error_count = 0;
}

CPgDB::CPgDB(const char* host, const char* dbname, const char* username, const char* userpass)
{
  m_pPGConn = NULL;
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

CPgDB::~CPgDB()
{
  Close();
}

int CPgDB::Open(void)
{
  char connStr[256];
  
  if(IsOpen()) Close();

  connStr[0] = 0;
  if(strlen(m_host))
  {
    strcat(connStr, "host=");
    strcat(connStr, m_host);
  }

  if(strlen(m_dbname))
  {
    if(strlen(connStr)) strcat(connStr, " ");
    strcat(connStr, "dbname=");
    strcat(connStr, m_dbname);
  }

  if(strlen(m_username))
  {
    if(strlen(connStr)) strcat(connStr, " ");
    strcat(connStr, "user=");
    strcat(connStr, m_username);
  }

  if(strlen(m_userpass))
  {
    if(strlen(connStr)) strcat(connStr, " ");
    strcat(connStr, "password=");
    strcat(connStr, m_userpass);
  }

  m_pPGConn = PQconnectdb(connStr);

  if(PQstatus(m_pPGConn) != CONNECTION_OK)
  {
    syslog(LOG_INFO, "CPgDB::Open([%s]) ERROR: [%s]", connStr, PQerrorMessage(m_pPGConn));
    return (-1);
  }

  return 0;
}

int CPgDB::Open(const char* host, const char* dbname, const char* username, const char* userpass)
{
  if(host) strcpy(m_host, host);
  if(dbname) strcpy(m_dbname, dbname);
  if(username) strcpy(m_username, username);
  if(userpass) strcpy(m_userpass, userpass);
  return this->Open();    
}

int CPgDB::Begin(void)
{
    return (-1);
}

int CPgDB::Query(cJSON *json_array, const char* query_fmt, ...)
{
  va_list arg;
  int rc;
  int line;
  char *rc_line;
  char query[4096];
  PGresult *pgrc;

  if(!IsOpen())
  {
    return(-1);
  }
  va_start(arg, query_fmt);
  vsprintf(query, query_fmt, arg);
  va_end(arg);

  if(!IsOpen()) Open();
  pgrc = PQexec(m_pPGConn, query);
  /*
  PQntuples() - number of rows in the query result
  PQcmdTuples() - number of rows affected by the sql-command 
  PQerrorMessage - get more information about such errors
  */
  if(PQresultStatus(pgrc) == PGRES_COMMAND_OK)
  {
    rc = atoi(PQcmdTuples(pgrc));
  }
  else if(PQresultStatus(pgrc) == PGRES_TUPLES_OK)
  {
    rc = PQntuples(pgrc);
    for(line = 0; line < rc; line++)
    {


      rc_line = PQgetvalue(pgrc, line, 0);


    }
  }
  else
  {
    rc = (-1);
    syslog(LOG_INFO, "CPgDB::Query([%s]) ERROR: [%s]", query, PQerrorMessage(m_pPGConn));
  }
  return rc;
}

int CPgDB::Commit(void)
{
    return (-1);
}

int CPgDB::Rollback(void)
{
    return (-1);
}

char* CPgDB::LastErrorMsg(char* msg)
{
  return msg;    
}

int CPgDB::IsOpen(void)
{
  if(m_pPGConn == NULL) return 0;
  if(PQstatus(m_pPGConn) == CONNECTION_OK) return 1;
  return 0;
}

void CPgDB::Close(void)
{
  if(m_pPGConn)
  {
    PQfinish(m_pPGConn);
    m_pPGConn = NULL;
  }
}
