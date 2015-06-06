#include "gui/mainwindow.h"
#include "gui/settings.h"

#if defined(GUI_QTWIDGETS) || defined(GUI_QTQUICK)
# include <qtutilities/resources/qtconfigarguments.h>
#else
# include <c++utilities/application/fakeqtconfigarguments.h>
#endif
#include <qtutilities/resources/resources.h>

#if defined(GUI_QTWIDGETS)
# include <QApplication>
#elif defined(GUI_QTQUICK)
# include <QGuiApplication>
#endif

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;

int main(int argc, char *argv[])
{
    ArgumentParser parser;
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    HelpArgument helpArg(parser);
    parser.setMainArguments({&qtConfigArgs.qtWidgetsGuiArg(), &helpArg});
    parser.parseArgs(argc, argv);
    if(qtConfigArgs.areQtGuiArgsPresent()) {
#ifdef GUI_QTWIDGETS
        QGuiApplication::setOrganizationName(QStringLiteral("Martchus"));
        QGuiApplication::setOrganizationDomain(QStringLiteral("http://martchus.netai.net/"));
        QGuiApplication::setApplicationName(QStringLiteral("Video Downloader"));
        QGuiApplication::setApplicationVersion(QStringLiteral("1.0.7"));
        QApplication a(argc, argv);
        QtUtilitiesResources::init();
        Theme::setup();
        QtGui::restoreSettings();
        QtGui::MainWindow w;
        w.show();
        int r = a.exec();
        QtGui::saveSettings();
        return r;
#else
        cout << "Application has not been build with Qt widgets GUI support." << endl;
#endif
    }
    return 0;
}
