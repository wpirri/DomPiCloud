/***************************************************************************
 * Copyright (C) 2021 Walter Pirri
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <cjson/cJSON.h>

#include <gmonitor/gmc.h>

#include "config.h"
#include "strfunc.h"

#define MAX_POST_LEN 32767
#define MAX_GET_LEN 255

int trace;
int timeout;

int main(int /*argc*/, char** /*argv*/, char** env)
{
  CGMInitData gminit;
  CGMClient *pClient;
  CGMError gmerror;
  DPConfig *pConfig;
  STRFunc Str;
  int i;
  CGMClient::GMIOS call_resp;

  char server_address[16];
  char s[16];
  
  char remote_addr[16];
  char request_uri[MAX_GET_LEN+1];
  char request_method[8];
  int content_length;
  char s_content_length[8];
  char post_data[MAX_POST_LEN+1];
  int rc;

  signal(SIGALRM, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  remote_addr[0] = 0;
  request_uri[0] = 0;
  request_method[0] = 0;
  post_data[0] = 0;
  content_length = 0;
  s_content_length[0] = 0;
  trace = 0;

  pConfig = new DPConfig("/etc/dompicloud.config");

  if( !pConfig->GetParam("DOMPICLOUD_SERVER", server_address))
  {
    return 0;
  }

  if( pConfig->GetParam("TRACE-NOTIF.CGI", s))
  {
    trace = atoi(s);
  }

  timeout = 1000;
  if( pConfig->GetParam("CGI-TIMEOUT", s))
  {
    timeout = atoi(s) * 1000;
  }

  for(i = 0; env[i]; i++)
  {
    if( !memcmp(env[i], "REMOTE_ADDR=", 12))
    {
      strncpy(remote_addr, env[i]+12, 15);
    }
    else if( !memcmp(env[i], "REQUEST_URI=", 12))
    {
      strncpy(request_uri, env[i]+12, MAX_GET_LEN);
    }
    else if( !memcmp(env[i], "REQUEST_METHOD=", 15))
    {
      strncpy(request_method, env[i]+15, 7);
    }
    else if( !memcmp(env[i], "CONTENT_LENGTH=", 15))
    {
      strncpy(s_content_length, env[i]+15, 7);
      content_length = atoi(s_content_length);
    }
  }

  if(content_length)
  {
    fgets(post_data, ((content_length+1)<MAX_POST_LEN)?(content_length+1):MAX_POST_LEN, stdin);
  }

  fputs("Connection: close\r\n", stdout);
  fputs("Content-Type: text/html\r\n", stdout);
  fputs("Cache-Control: no-cache\r\n\r\n", stdout);


  Str.EscapeHttp(request_uri, request_uri);
  Str.EscapeHttp(post_data, post_data);

  if(trace)
  {
    openlog("dompi_cloud_notif.cgi", 0, LOG_USER);
    syslog(LOG_DEBUG, "REMOTE_ADDR: %s",remote_addr);
    syslog(LOG_DEBUG, "REQUEST_URI: [%s]",request_uri);
    syslog(LOG_DEBUG, "REQUEST_METHOD: %s",request_method);
    syslog(LOG_DEBUG, "CONTENT_LENGTH: %s",s_content_length);
    syslog(LOG_DEBUG, "CONFIG_FILE: /etc/dompiio.config");
    syslog(LOG_DEBUG, "DOMPIWEB_SERVER: [%s]",server_address);
  }

  gminit.m_host = server_address;
  gminit.m_port = 5533;

  pClient = new CGMClient(&gminit);

  if(trace)
  {
    syslog(LOG_DEBUG, "Call Q: dompi_web_notif [%s]", post_data); 
  }

  rc = pClient->Call("dompi_web_notif", post_data, strlen(post_data), &call_resp, timeout);
  if(rc == 0)
  {
    fprintf(stdout, "%s\r\n", (const char*)call_resp.data);
    if(trace)
    {
      syslog(LOG_DEBUG, "Call R: [%s]", (const char*)call_resp.data);
    }
  }
  else
  {
    fprintf(stdout, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
    if(trace)
    {
      syslog(LOG_DEBUG, "{ \"rc\":\"%02i\", \"msg\":\"%s\" }\r\n", rc, gmerror.Message(rc).c_str());
    }
  }
  delete pClient;
  return 0;
}
