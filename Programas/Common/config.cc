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
#include "config.h"

#include <string>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
using namespace std;

#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <string.h>

DPConfig::DPConfig()
{
  m_data = NULL;
  m_config_file[0] = 0;
}

DPConfig::DPConfig(const char* filename)
{
  m_data = NULL;
  strncpy(m_config_file, filename, FILENAME_MAX);
  Load();
}

DPConfig::~DPConfig()
{
  Clean(m_data);
}

int DPConfig::Load(const char* filename)
{
  Clean(m_data);
  strncpy(m_config_file, filename, FILENAME_MAX);
  return Load();
}

int DPConfig::Load()
{
  char line[256];
  char *p;
  size_t len;
  char *p_label;
  char *p_data;
  int count = 0;
  FILE *f;

  Clean(m_data);

  p = &line[0];
  len = sizeof(line);
  f = fopen(m_config_file, "r");
  if(f)
  {
    while(getline(&p, &len, f) > 0)
    {
      line[len] = 0;
      /* Salteo las lineas de comentarios */
      if(line[0] != '#' && line[0] != ';' && line[0] != 0x0d && line[0] != 0x0a && line[0] != 0x00)
      {
        p_data = strchr(line, '=');
        if(p_data)
        {
          /* Estoy en una linea de configuracion */
          *p_data = 0;
          p_data++;
          /* Salteo los espacios despues del = */
          while(*p_data == ' ') p_data++;
          p_label = &line[0];
          /* Elimino los espacios antes del = */
          if(strchr(p_label, ' ')) *(strchr(p_label, ' ')) = 0;
          /* Pongo el null al final si no esta ya */
          if(strchr(p_data, 0x0d)) *(strchr(p_data, 0x0d)) = 0;
          if(strchr(p_data, 0x0a)) *(strchr(p_data, 0x0a)) = 0;
          if(strchr(p_data, ' ')) *(strchr(p_data, 0x0d)) = 0;
          if(strchr(p_data, ';')) *(strchr(p_data, 0x0d)) = 0;
          if(strchr(p_data, '#')) *(strchr(p_data, 0x0d)) = 0;
          AddData(p_label, p_data);
          count++;
        }
      }
      len = sizeof(line);
    }
    fclose(f);
  }
  return count;
}

char* DPConfig::GetParam(const char* label, char* out)
{
  dpconfig *p = m_data;

  if(label && out && p)
  {
    do
    {
      if( !strcmp(p->label, label))
      {
        strcpy(out, p->value);
        return out;
      }
      p = p->next;
    } while (p);
  }
  return NULL;
}

void DPConfig::AddData(const char* label, const char* value)
{
  dpconfig *p;

  if(label && value)
  {
    /* Busco el siguiene slot */
    if(m_data)
    {
      /* Elementos 2 a n */
      p = m_data;
      while(p->next) p = p->next;
      p->next = (dpconfig*)calloc(1, sizeof(dpconfig));
      p = p->next;
    }
    else
    {
      /* Primer elemento */
      m_data = (dpconfig*)calloc(1, sizeof(dpconfig));
      p = m_data;
    }
    /* Si pudo alocar un nuevo slot continuo */
    if(p)
    {
      p->label = (char*)calloc(strlen(label)+1, sizeof(char));
      if(p->label)
      {
        strcpy(p->label, label);
      }
      p->value = (char*)calloc(strlen(value)+1, sizeof(char));
      if(p->label)
      {
        strcpy(p->value, value);
      }
      p->next = NULL;
    }
  }
}

void DPConfig::Clean(dpconfig* p)
{
  if(p)
  {
    if(p->next) Clean(p->next);
    free(p->label);
    free(p->value);
    free(p);
  }
}
