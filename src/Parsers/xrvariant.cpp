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

#include "xrvariant.h"

XRVariant::XRVariant(const QVariant& aqv)
{
  this->QVariant::operator=(aqv);
}

XRVariant::XRVariant(QDomElement& xrv)
{
  fromDomElement(xrv);
}

XRVariant::XRVariant( const QString& xml )
{
	QDomDocument doc;
	if (doc.setContent(xml))
	{
		QDomNodeList nodeList = doc.elementsByTagName("value");

		if (nodeList.count() > 0)
		{
            QDomElement tempElement = nodeList.at(0).toElement();
			fromDomElement(tempElement);
		}
	}
	else
	{
		throw std::logic_error("Could not parse XML from XML-RPC call");
	}
}


XRVariant XRVariant::arrayFromDomElement(QDomElement& qde)
{
    QDomNode a_n = qde.firstChild();
    QDomElement tmp_a = a_n.toElement();
    
    if( tmp_a.tagName() == "data" ) {
        //All arrays must have data
	a_n = tmp_a.firstChild();

	QList<QVariant> tmp_list;
        bool failed = false;
	while((!a_n.isNull()) && (failed == false)) {
	    QDomElement a_e = a_n.toElement();
            XRVariant tmp_xrv(a_e);
	    if(tmp_xrv.typeName() == 0) {
	        //This means it is an invalid variant.
                failed = true;
	    }
	    else {
                tmp_list.push_back(tmp_xrv);
	    }
	    a_n = a_n.nextSibling();
        }
	if( failed == false ) {
            return QVariant(tmp_list);
	}
    }
    return QVariant();
}

void XRVariant::fromDomElement(QDomElement& xrv)
{
    if( !xrv.isNull() ) {
    //xrv should be a dom node which
    //holds a value.  Check to see:
    if( xrv.tagName() != "value" ) {
       
    }
    else {
        //This looks good so far.
	if( xrv.hasChildNodes() ) {
	  //According to the xml-rpc spec a value should have only one child:
	  QDomNode xrv_node = xrv.firstChild();
	  QDomElement xrv_type = xrv_node.toElement();
	  if( xrv_type.isNull() ) {
	    this->QVariant::operator=( QVariant(xrv.text()) );
	  }
          if( (xrv_type.tagName() == "i4") ||
	    (xrv_type.tagName() == "int") ) {
	    this->QVariant::operator=( QVariant( xrv_type.text().toInt() ) );
   	  }
	  else if( xrv_type.tagName() == "boolean") {
            
            if( xrv_type.text() == "0" ) {
		    //The second arg is a dummy that Qt needs for compatibility
	      this->QVariant::operator=( QVariant(false) );
	    }
	    else if(xrv_type.text() == "1") 
		{
		    //The second arg is a dummy that Qt needs for compatibility
	      this->QVariant::operator=( QVariant(true) );
	    }
	  }
	  else if( xrv_type.tagName() == "string") {
	    this->QVariant::operator=( QVariant( xrv_type.text()) );
	  }
	  else if( xrv_type.tagName() == "double") {
	    this->QVariant::operator=( QVariant( xrv_type.text().toDouble()) );
	  }
	  else if( xrv_type.tagName() == "dateTime.iso8601") {
            //We must insert the "-" for Qt to recogize them:
	    QString tmps_date(xrv_type.text());
	    tmps_date.insert(4,'-');
	    tmps_date.insert(7,'-');
	    QVariant tmp_date( QDateTime::fromString(tmps_date,Qt::ISODate) );
	    if( tmp_date.isValid() ) {
	      this->QVariant::operator=( tmp_date );
	    }
	  }
	  else if( xrv_type.tagName() == "base64") {
              this->QVariant::operator=(
			      QVariant( XRBase64::decode( xrv_type.text() ) )
			                );
	  }
	  else if( xrv_type.tagName() == "array") {
              this->operator=( arrayFromDomElement(xrv_type) );
	  }
	  else if( xrv_type.tagName() == "struct") {
              this->operator=( structFromDomElement(xrv_type) );
	  }
	}
      }
    }
}

bool XRVariant::isXmlRpcType(const QVariant &qv)
{
    if ((qv.canConvert<QString>()) ||
        (qv.canConvert<int>()) ||
        (qv.canConvert<bool>()) ||
        (qv.canConvert<double>()) ||
        (qv.canConvert<QDateTime>()) ||
        (qv.canConvert<QByteArray>()))
    {
        return true;
    }
    else if (qv.canConvert<QVariantList>())
    {
        bool all_good = true;
        for (const QVariant& v : qv.toList())
        {
            all_good = all_good && isXmlRpcType(v);
        }

        return all_good;
    }
    else if (qv.canConvert<QVariantMap>())
    {
        bool all_good = true;
        auto list = qv.toMap();
        QMapIterator<QString, QVariant> mi(list);
        while (mi.hasNext())
        {
            mi.next();
            all_good = all_good && isXmlRpcType(mi.value());
        }

        return all_good;
    }

    return false;
}

XRVariant XRVariant::structFromDomElement(QDomElement& qde)
{
 
    QMap<QString, QVariant> tmp_map;
	      
    QDomNode a_n = qde.firstChild();
    bool failed = false;
    while( (!a_n.isNull()) && (failed == false)) {
        QDomElement a_e = a_n.toElement();
	if( a_e.tagName() == "member" ) {
            bool have_name=false, have_value=false;
	    
            QDomNode one_node = a_e.firstChild();
		
            QString member_name;
	    QVariant member_value;
		
            while( !one_node.isNull() ) {
                QDomElement member_e = one_node.toElement();
		if( member_e.tagName() == "name") {
		    have_name=true;
                    member_name = member_e.text();
		}
		else if( member_e.tagName() == "value") {
		    have_value=true;
                    member_value = XRVariant(member_e);     
		}
		if( have_name && have_value ) {
                  tmp_map.insert(member_name, member_value);
		  have_name=false;
		  have_value=false;
		}
		one_node = one_node.nextSibling();
            }
	}
	a_n = a_n.nextSibling();
    }
    return QVariant(tmp_map);
}

QDomElement XRVariant::toDomElement(QDomDocument& doc) const
{
	QDomElement ret_el = doc.createElement("value");
	QDomElement type_el;
	QDomText t;

	if (const auto this_type = metaType().id(); this_type == QMetaType::QString) 
	{
		type_el = doc.createElement("string");
		t = doc.createTextNode(toString());
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::Int || this_type == QMetaType::UInt) 
    {
		type_el = doc.createElement("int");
		t = doc.createTextNode(toString());
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::Bool) 
    {
		type_el = doc.createElement("boolean");
		if(toBool()) 
        {
			t = doc.createTextNode("1");
		}
		else 
        {
			t = doc.createTextNode("0");
		}
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::Double) 
    {
		type_el = doc.createElement("double");
		t = doc.createTextNode(toString());
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::QDateTime) 
    {
		type_el = doc.createElement("dateTime.iso8601");

		/* QT has ISODate type, but since XML-RPC requires
		* it to be exact, just tell it the format
		*/
		QString tmps_date(toDateTime().toString("yyyyMMddThh:mm:ss"));

		t = doc.createTextNode( tmps_date );
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::QDate)
    {
		type_el = doc.createElement("dateTime.iso8601");

		/*
		* We will store the date with Time 00:00:00
		*/
		QDateTime tmp_dt(toDate(), QTime(0,0));
		QString tmps_date(tmp_dt.toString("yyyyMMddThh:mm:ss"));

		t = doc.createTextNode( tmps_date );
		type_el.appendChild(t);
	}
	else if (this_type == QMetaType::QByteArray) 
    {
		type_el = doc.createElement("base64");
		t = doc.createTextNode( XRBase64::encode( toByteArray() ));
		type_el.appendChild(t);
	}
	// else if (this_type == QMetaType::QList || this_type == QMetaType::QStringList) 
    else if (this_type == QMetaType::QStringList) 
	{
		//type_el = doc.createElement("array");
		//QDomElement data_el = doc.createElement("data");
		//type_el.appendChild(data_el);

		//QValueListConstIterator<QVariant> it;
		//for(it = listBegin(); it != listEnd(); it++) 
		//{
		//	data_el.appendChild( XRVariant(*it).toDomElement(doc) );
		//}
	}
	else if (this_type == QMetaType::QVariantMap) 
	{
		//type_el = doc.createElement("struct");
		//QMapConstIterator<QString,QVariant> it;
		//for(it = mapBegin(); it != mapEnd(); it++) 
		//{
		//	QDomElement mem_el = doc.createElement("member");
		//	//Add the name:
		//	QDomElement name_el = doc.createElement("name");
		//	t = doc.createTextNode(it.key());
		//	name_el.appendChild(t);
		//	mem_el.appendChild(name_el);
		//	//Add the value:
		//	mem_el.appendChild( XRVariant(it.data()).toDomElement(doc) );
		//	//Put the member into the struct:
		//	type_el.appendChild(mem_el);
		//}
	}
	//else if( canCast( QVariant::String ) ) 
	//{
	//	/* Here is a type we don't know what else to do with,
	//	* but we can cast it into a string
	//	*/
	//	type_el = doc.createElement("string");
	//	t = doc.createTextNode(toString());
	//	type_el.appendChild(t);
	//}
	else 
	{
		/* this is the catch all, empty string */
		type_el = doc.createElement("string");
		t = doc.createTextNode("");
		type_el.appendChild(t);
	}

	ret_el.appendChild(type_el);
	return ret_el;
}
