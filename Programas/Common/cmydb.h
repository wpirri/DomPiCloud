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
#ifndef _CMYDB_H_
#define _CMYDB_H_

/*
instalar libcjson-dev (https://github.com/DaveGamble/cJSON)
#include <cjson/cJSON.h>
-lcjson
*/
#include <cjson/cJSON.h>

/*
https://dev.mysql.com/doc/c-api/8.0/en/c-api-function-reference.html
apt install default-libmysqlclient-dev
#include <mysql.h>
Ejecutar 'mysql_config --cflags --libs' para obtener los parametros de compilacion
*/
#include <mysql.h>

class CMyDB
{
public:
    CMyDB(void);
    CMyDB(const char* host, const char* dbname, const char* username, const char* userpass);
    virtual ~CMyDB();

    int Open(void);
    int Open(const char* host, const char* dbname, const char* username, const char* userpass);

    int Begin(void);
    int Query(cJSON *json_array, const char* query_fmt, ...);
    int Commit(void);
    int Rollback(void);

    char* LastErrorMsg(char* msg);

    int IsOpen(void);

    void Close(void);
private:
    MYSQL *m_pMYConn;
    char m_host[64];
    char m_dbname[32];
    char m_username[32];
    char m_userpass[32];
    int m_connect_error_count;

};

#endif /* _CMYDB_H_ */