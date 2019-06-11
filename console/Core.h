#pragma once

#ifdef _WINDOWS
#pragma warning(disable:4530)
#pragma warning(disable:4068)
#endif

#define __STRINGIFY__(x)            #x
#define __EXPAND__(x)               __STRINGIFY__(x)

#define APP_NAME                    "OwlConsole"
#define APP_TITLE					APP_NAME " " OWL_VERSION

#define ORGANZATION_DOMAIN          "owlclient.com"

#ifdef Q_OS_MACX
#define ORGANIZATION_NAME			"Adalid Claure"
#else
#define ORGANIZATION_NAME			"lulzApps"
#endif

#define OWLCONSOLE_MAJOR			0
#define	OWLCONSOLE_MINOR			1
#define OWLCONSOLE_BUILD			0
#define OWLCONSOLE_VERSION			__EXPAND__(OWLCONSOLE_MAJOR) "." __EXPAND__(OWLCONSOLE_MINOR) "." __EXPAND__(OWLCONSOLE_BUILD)
#define OWLCONSOLE_BUILDTIMESTAMP	__DATE__ " " __TIME__

#define COPYRIGHT                   "Copyright (c) 2012-2019, " ORGANIZATION_NAME ". All Rights Reserved."
#define DEFAULT_USER_AGENT			"Mozilla/5.0 Firefox/3.5.6 " APP_NAME "/" OWL_VERSION
