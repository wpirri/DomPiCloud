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
#ifndef _STRFUNC_H_
#define _STRFUNC_H_

class STRFunc
{
public:
	STRFunc();
	virtual ~STRFunc();

  /*  Separa en campos segun el separador y devuelve el número de campo solicitado
    en count. El primer campo es el 1.
    Si count e 0 devuelve lo que hay antes del separador
    El dato devuelto se termina al encntrar un separador o cualquier caracter que no se número o letra
*/
  int Section(const char *in, char sep, unsigned int count, char *out);
/*
  En una cadena label=val&label=val devuelvel el val  segun el label
*/
  int ParseData(const char *buffer, const char *label, char *value);

/*
  En una cadena label=val&label=val devuelvel el par label, value del orden indicado por el indice donde 0 es el primero
*/
  int ParseDataIdx(const char *buffer, char *label, char *value, int idx);

/*
  Elimina las secuencias de escape HTTP %xx
*/
  int EscapeHttp(const char* in, char* out);

  int StrHex2Int(const char *str_hex);

  int ParseCommand(const char *buffer, char *comando, char *objeto, char *parametro);


private:

};
#endif /* _STRFUNC_H_ */

