#ifdef _WINDOWS
#include <WinSock2.h>
#endif

#include <QTranslator>

#include <boost/program_options.hpp>

#include <Utils/OwlLogger.h>

#include "Core.h"
#include "MainWindow.h"
#include "OwlApplication.h"
#include "ErrorReportDlg.h"

namespace po = boost::program_options;

namespace
{
    const size_t    SUCCESS = 0;
    const size_t    OWL_EXCEPTION = 1;
    const size_t    EXCEPTION = 2;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Owl);

    po::options_description desc("Program options");
    desc.add_options()
        ("help,?", "print help message")
        ("version,v", "print version string")
        ("reset", "reset all program data and settings")

        ("db,d", "use the specified database")
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch(const po::error& er)
    {
        std::cout << "command line argument error: " << er.what() << '\n';
        return EXCEPTION;
    }

    if (vm.count("help") > 0)
    {
        std::cout << desc << '\n';
        return SUCCESS;
    }


    owl::OwlApplication app(argc, &argv);
    QLoggingCategory::setFilterRules(QStringLiteral("qt.network.ssl=false"));

    // main() return value, assume error
    int retval = SUCCESS;

    try
    {
        app.init();

        owl::SplashScreen splash(QPixmap(":/images/splash-full.png"));

        // show the splash screen
        splash.show();

        // make the splash appear while we load
        app.processEvents();

        // initialize the main window
        owl::MainWindow window(&splash, nullptr);

        // finish the splash screen
        splash.finish(&window);

        // show the main window
        window.show();
        retval = app.exec();
    }
    catch (const owl::Exception& ex)
    {
        owl::rootLogger()->critical("There was an unrecoverable application error: {}", ex.message().toStdString());

        owl::ErrorReportDlg dlg{ ex, QObject::tr( "Application Error") };
        dlg.exec();

        retval = OWL_EXCEPTION;
    }
    catch (const std::exception& ex)
    {
        owl::rootLogger()->critical("There was an unrecoverable system error: {}", ex.what());

        QMessageBox::warning(nullptr, APP_TITLE,
            QString(QObject::tr("There was an unrecoverable system error: %1").arg(ex.what())));

        retval = EXCEPTION;
    }
    catch(...)
    {
        owl::rootLogger()->critical("There was an unknown unrecoverable error");

        QMessageBox::warning(nullptr, APP_TITLE, 
            QString(QObject::tr("There was an unknown error")));

        retval = EXCEPTION;
    }
    
    return retval;
}
