#ifdef _WINDOWS
#include <WinSock2.h>
#endif

#include "Core.h"
#include "MainWindow.h"
#include "OwlApplication.h"
#include <spdlog/spdlog.h>

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
        spdlog::get("Owl")->critical("There was an unrecoverable application error: %1", ex.message().toStdString());

        QMessageBox::warning(nullptr, APP_TITLE,
            QString("There was an unrecoverable application error: %1").arg(ex.message()));

        retval = OWLEXCEPTION;
    }
    catch (const std::exception& ex)
    {
        spdlog::get("Owl")->critical("There was an unrecoverable system error: %1", ex.what());

        QMessageBox::warning(nullptr, APP_TITLE,
            QString("There was an unrecoverable system error: %1").arg(ex.what()));

        retval = STDEXCEPTION;
    }
    catch(...)
    {
        spdlog::get("Owl")->critical("There was an unknown unrecoverable error");

        QMessageBox::warning(nullptr, APP_TITLE, 
            QString("There was an unknown error"));

        retval = UNKNOWNEXCEPTION;
    }
    
    return retval;
}
