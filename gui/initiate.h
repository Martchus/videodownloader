#ifndef QTGUI_INIT_H
#define QTGUI_INIT_H

namespace ApplicationUtilities {
class QtConfigArguments;
}

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const ApplicationUtilities::QtConfigArguments &qtConfigArgs);

}

#endif // QTGUI_INIT_H
