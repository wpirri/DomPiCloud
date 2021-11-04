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
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <cstdio>

typedef struct _dpconfig
{
  char *label;
  char *value;
  struct _dpconfig *next;
} dpconfig;

class DPConfig
{
public:
	DPConfig();
	DPConfig(const char* filename);
	virtual ~DPConfig();

  int Load(const char* filename);
  char* GetParam(const char* label, char* out);

protected:
  char m_config_file[FILENAME_MAX+1];
  dpconfig *m_data;

  int Load();
  void Clean(dpconfig* p);
  void AddData(const char* label, const char* value);
};




#endif /* _CONFIG_H_ */

