#ifndef QTGUI_INIT_H
#define QTGUI_INIT_H

namespace CppUtilities {
class QtConfigArguments;
}

namespace QtGui {

int runWidgetsGui(int argc, char *argv[], const CppUtilities::QtConfigArguments &qtConfigArgs);
}

#endif // QTGUI_INIT_H
