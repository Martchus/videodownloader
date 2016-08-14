#include "./initiate.h"
#include "./settings.h"
#include "./mainwindow.h"

#include "resources/config.h"

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#if defined(GUI_QTWIDGETS)
# include <QApplication>
#elif defined(GUI_QTQUICK)
# include <QGuiApplication>
#endif

using namespace std;
using namespace ApplicationUtilities;

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs)
{
#ifdef GUI_QTWIDGETS
    SET_QT_APPLICATION_INFO;
    QApplication a(argc, argv);
    restoreSettings();
    // apply settings specified via command line args after the settings chosen in the GUI to give the CLI options precedence
    qtSettings().apply();
    qtConfigArgs.applySettings(qtSettings().hasCustomFont());
    // load resources needed by classes of qtutilities
    QtUtilitiesResources::init();
    MainWindow w;
    w.show();
    int r = a.exec();
    saveSettings();
    return r;
#else
    CMD_UTILS_START_CONSOLE;
    cout << "Application has not been build with Qt widgets GUI support." << endl;
    return 0;
#endif
}

}
