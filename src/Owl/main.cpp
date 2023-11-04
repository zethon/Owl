#ifdef _WINDOWS
#include <WinSock2.h>
#endif

#include <QTranslator>

#include "ZFontIcon/ZFontIcon.h"
#include "ZFontIcon/ZFont_fa4.h"
#include "ZFontIcon/ZFont_fa5.h"

#include "Core.h"
#include "MainWindow.h"
#include "OwlApplication.h"
#include "ErrorReportDlg.h"

#include <Utils/OwlLogger.h>

using namespace owl;

#define OWLEXCEPTION        1
#define STDEXCEPTION        2
#define UNKNOWNEXCEPTION    3

void register_font_awesome5()
{
    if (!ZFontIcon::addFont(":/fonts/fa4/" + Fa4::FA4_TTF_FILE_REGULAR))
    {
        OWL_THROW_EXCEPTION(owl::Exception(fmt::format("Failed to load {}", Fa4::FA4_TTF_FILE_REGULAR.toStdString())));
    }

    if (!ZFontIcon::addFont(":/fonts/fa5/" + Fa5::FA5_OTF_FILE_FREE_SOLID))
    {
        OWL_THROW_EXCEPTION(owl::Exception(fmt::format("Failed to load {}", Fa5::FA5_OTF_FILE_FREE_SOLID.toStdString())));
    }

    if (!ZFontIcon::addFont(":/fonts/fa5/" + Fa5brands::FA5_WOFF_FILE_BRANDS))
    {
        OWL_THROW_EXCEPTION(owl::Exception(fmt::format("Failed to load {}", Fa5brands::FA5_WOFF_FILE_BRANDS.toStdString())));
    }
}

int main(int argc, char *argv[])
{
    owl::OwlApplication app(argc, &argv);

    // main() return value, assume error
    int retval = 1;

    try
    {
        QLoggingCategory::setFilterRules(QStringLiteral("qt.network.ssl=false"));
        register_font_awesome5();

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
    catch (const Exception& ex)
    {
        owl::rootLogger()->critical("There was an unrecoverable application error: {}", ex.message().toStdString());

        owl::ErrorReportDlg dlg{ ex, QObject::tr("Application Error") };
        dlg.exec();

        retval = OWLEXCEPTION;
    }
    catch (const std::exception& ex)
    {
        owl::rootLogger()->critical("There was an unrecoverable system error: {}", ex.what());

        QMessageBox::warning(nullptr, APP_TITLE,
            QString(QObject::tr("There was an unrecoverable system error: %1").arg(ex.what())));

        retval = STDEXCEPTION;
    }
    catch(...)
    {
        owl::rootLogger()->critical("There was an unknown unrecoverable error");

        QMessageBox::warning(nullptr, APP_TITLE, 
            QString(QObject::tr("There was an unknown error")));

        retval = UNKNOWNEXCEPTION;
    }
    
    return retval;
}
