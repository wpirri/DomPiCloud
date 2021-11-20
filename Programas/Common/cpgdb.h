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
#ifndef _CPGDB_H_
#define _CPGDB_H_

/*
instalar libcjson-dev (https://github.com/DaveGamble/cJSON)
#include <cjson/cJSON.h>
-lcjson
*/
#include <cjson/cJSON.h>

/*
apt install libpq-dev
#include <postgresql/libpq-fe.h>
-lpq
*/
#include <postgresql/libpq-fe.h>

class CPgDB
{
public:
    CPgDB(void);
    CPgDB(const char* host, const char* dbname, const char* username, const char* userpass);
    virtual ~CPgDB();

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
    PGconn *m_pPGConn;
    char m_host[64];
    char m_dbname[32];
    char m_username[32];
    char m_userpass[32];
    int m_connect_error_count;

};

#endif /* _CPGDB_H_ */