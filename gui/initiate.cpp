#include "./initiate.h"
#include "./settings.h"
#include "./mainwindow.h"

#include "resources/config.h"

#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>

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
    // load resources needed by classes of qtutilities
    QtUtilitiesResources::init();
    // apply settings specified via command line args
    qtConfigArgs.applySettings();
    QtGui::restoreSettings();
    QtGui::MainWindow w;
    w.show();
    int r = a.exec();
    QtGui::saveSettings();
    return r;
#else
    CMD_UTILS_START_CONSOLE;
    cout << "Application has not been build with Qt widgets GUI support." << endl;
    return 0;
#endif
}

}
