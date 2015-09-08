#include "../cli/mainfeatures.h"
#include "../gui/initiate.h"

#if defined(GUI_QTWIDGETS) || defined(GUI_QTQUICK)
# include <qtutilities/resources/qtconfigarguments.h>
#else
# include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <c++utilities/application/failure.h>
#include <c++utilities/application/commandlineutils.h>
#include <qtutilities/resources/resources.h>

#include <QCoreApplication>

#include <iostream>

#include "main.h"

using namespace std;
using namespace std::placeholders;
using namespace ApplicationUtilities;

int main(int argc, char *argv[])
{
    // setup argument parser
    SET_APPLICATION_INFO;
    ArgumentParser parser;
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    HelpArgument helpArg(parser);
    Argument noConfirmArg("no-confirm", "n", "start downloading without confirmation");
    noConfirmArg.setCombinable(true);
    Argument downloadArg("download", "d", "downloads the specified data");
    downloadArg.setValueNames({"URL1", "URL2", "URL3"});
    downloadArg.setRequiredValueCount(-1);
    downloadArg.setDenotesOperation(true);
    downloadArg.setSecondaryArguments({&noConfirmArg});
    downloadArg.setCallback(bind(Cli::download, argc, argv, _1, cref(noConfirmArg)));
    parser.setMainArguments({&qtConfigArgs.qtWidgetsGuiArg(), &downloadArg, &helpArg});
    // parse arguments
    try {
        parser.parseArgs(argc, argv);
    } catch (Failure &ex) {
        CMD_UTILS_START_CONSOLE;
        cout << "Unable to parse arguments. " << ex.what() << "\nSee --help for available commands." << endl;
    }
    // set meta info for application
    if(qtConfigArgs.areQtGuiArgsPresent()) {
        return QtGui::runWidgetsGui(argc, argv, qtConfigArgs);
    }
    return 0;
}
