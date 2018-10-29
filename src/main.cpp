#ifdef _WINDOWS
#include <WinSock2.h>
#endif

#include "Core.h"
#include "MainWindow.h"
#include "OwlApplication.h"

using namespace owl;

#define OWLEXCEPTION        1
#define STDEXCEPTION        2
#define UNKNOWNEXCEPTION    3

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Owl);
    owl::OwlApplication app(argc, &argv);
    QLoggingCategory::setFilterRules(QStringLiteral("qt.network.ssl=false"));

    // main() return value, assume error
    int retval = 1;

    try
    {
        app.init();

        owl::SplashScreen splash(QPixmap(":/images/splash-full.png"));

        // show the splash screen
        splash.show();

        // make the splash appear while we load
        app.processEvents();

        // initialize the main window
        MainWindow window(&splash, nullptr);

        // finish the splash screen
        splash.finish(&window);

        // show the main window
        window.show();
        retval = app.exec();
    }
    catch (const OwlException& ex)
    {
        app.logger()->fatal("There was an unrecoverable error: %1", ex);
        retval = OWLEXCEPTION;
    }
    catch (const std::exception& ex)
    {
        app.logger()->fatal("There was an unrecoverable error: %1", ex.what());
        retval = STDEXCEPTION;
    }
    catch(...)
    {
        app.logger()->fatal("There was an unknown unrecoverable error");

        QMessageBox::warning(nullptr, APP_TITLE, 
            QString("There was an unknown error"));

        retval = UNKNOWNEXCEPTION;
    }
    
    return retval;
}
