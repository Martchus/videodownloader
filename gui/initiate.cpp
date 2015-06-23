#include "initiate.h"
#include "settings.h"
#include "mainwindow.h"

#include <qtutilities/resources/resources.h>

#if defined(GUI_QTWIDGETS)
# include <QApplication>
#elif defined(GUI_QTQUICK)
# include <QGuiApplication>
#endif

using namespace std;

namespace QtGui {

int runWidgetsGui(int argc, char *argv[])
{
#ifdef GUI_QTWIDGETS
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
        return 0;
#endif
}

}
