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

#include <xrbase64.h>

/**
 * this code is adapted from Wei Dai's public domain base64.cpp
 */

const int XRBase64::MAX_LINE_LENGTH = 76;
const char XRBase64::vec[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
;
const char XRBase64::padding = '=';

int XRBase64::convertToNumber(char inByte) {

    if (inByte >= 'A' && inByte <= 'Z')
        return (inByte - 'A');

    if (inByte >= 'a' && inByte <= 'z')
        return (inByte - 'a' + 26);

    if (inByte >= '0' && inByte <= '9')
        return (inByte - '0' + 52);

    if (inByte == '+')
        return (62);

    if (inByte == '/')
        return (63);

    return (-1);
}

QByteArray XRBase64::decode(QString ascii)
{
	return QByteArray::fromBase64(ascii.toLatin1()).trimmed();
}

QString XRBase64::encode(const QByteArray& bin)
{
	QByteArray array(bin);
	return array.toBase64();
}
