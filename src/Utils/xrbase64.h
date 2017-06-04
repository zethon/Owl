/*

qutexrlib QT-based XML-RPC library
Copyright (C) 2003  P. Oscar Boykin <boykin@pobox.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   
*/
   
#ifndef xrbase64
#define xrbase64
#include <qstring.h>

/**
 * for some god forsaken reason, QT does not include Base64 conversion
 * code.  Here is something simple.
 */

class XRBase64 {

    public:
	static QByteArray decode(QString ascii);
	static QString encode(const QByteArray& bin);

    protected:
	static int convertToNumber(char a_byte);
	static const int MAX_LINE_LENGTH;
	static const char vec[];
	static const char padding;
};
#endif
