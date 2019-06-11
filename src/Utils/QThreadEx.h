// Owl - www.owlclient.com
// Copyright (c) 2012-2019, Adalid Claure <aclaure@gmail.com>

#pragma once
#include <QtCore>

namespace owl
{

// automatically starts thread objects
// See: http://labs.qt.nokia.com/2006/12/04/threading-without-the-headache/
class QThreadEx : public QThread
{

protected:
    void run() 
	{ 
		exec(); 
	}
};

} // namespace
