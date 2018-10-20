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

#ifndef xrvariant
#define xrvariant

#include <qvariant.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qstring.h>
#include "xrbase64.h"
/**
 * This subclass of QT's QVariant class represents the data types that are
 * defined in XML-RPC.  It handles serialization to and from XML using QDOM.
 * Users of the library should probably NEVER use this class.
 * It is used exclusively by XRMethodCall and
 * XRMethodResponse.
 *
 * @see http://www.xmlrpc.com/spec
 * 
 * The clean mapping between QT types is the following.
 * A key feature is that is REVERSIBLE, i.e. it is one-to-one.
 * All deserialization is done according to the clean mapping.
 * <ul>
 *   <li>C++ int : XML-RPC int</li>
 *   <li>C++ bool : XML-RPC boolean</li>
 *   <li>C++ double : XML-RPC double</li>
 *   <li>QT QString : XML-RPC string</li>
 *   <li>QT QDateTime : XML-RPC dateTime.iso8601</li>
 *   <li>QT QByteArray : XML-RPC base64</li>
 *   <li>QT QValueList<QVariant> : XML-RPC array</li>
 *   <li>QT QMap<QString,QVariant> : XML-RPC struct</li>
 * </ul>
 *
 * The lossy mapping is the above plus the following:
 * <ul>
 *   <li>C++ uint : XML-RPC int</li>
 *   <li>QT QDate : XML-RPC dateTime.iso8601 with time 00:00:00</li> 
 *   <li>QT QStringList : XML-RPC array</li>
 *   <li>All others, if they can be cast to QString : XML-RPC string</li>
 *   <li>All others, to empty strings.</li>
 * </ul>
 *  Note that the lossy mapping is NOT reversible.  You will have to
 *  use QVariant::cast() to convert it back to it's original type,
 *  and you will not know what the original type was without extra
 *  information.
 *
 * Use the static method XRVariant::isXmlRpcType() to check to see
 * if you can cleanly map.  If you do not check before hand, a lossy
 * mapping may occur.  The responsibility to make sure that data can
 * be cleanly mapped is on users of the library.  The library will always
 * make a best-effort, which may be lossy.
 *   
 */

class XRVariant : public QVariant
{
    public:
	/**
	 * @param aqv QVariant from which to create a XRVariant
	 *
	 * This is how we create XRVariants to do serialization
	 */
	XRVariant(const QVariant& aqv);
	/**
	 * read an XML-RPC value from this Dom node.
	 * Note, this is not const for the QDomNode
	 */
        XRVariant(QDomElement& a_xml_rpc_value);

		XRVariant(const QString& xml);

	/**
	 * Reinitialize the XRVariant from this dom element
	 */
	void fromDomElement(QDomElement& qde);
	/**
	 * If this is a map or list type, it recurses through
	 * all the elements and checks all of them.
	 * @return true if qv is not one of the lossy types
	 */
	static bool isXmlRpcType(const QVariant& qv);
	/**
	 * output the XRVariant into a dom element
	 */
	QDomElement toDomElement(QDomDocument& doc) const;

    private:
	/*
	 * static private methods:
	 */
	static XRVariant arrayFromDomElement(QDomElement& qde);
	static XRVariant structFromDomElement(QDomElement& qde);
};

#endif
