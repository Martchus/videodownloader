#include "../cli/mainfeatures.h"
#include "../gui/initiate.h"

#include "resources/config.h"

#if defined(VIDEODOWNLOADER_GUI_QTWIDGETS) || defined(VIDEODOWNLOADER_GUI_QTQUICK)
#include <qtutilities/resources/qtconfigarguments.h>
#else
#include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/application/failure.h>
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
    Argument noConfirmArg("no-confirm", 'n', "start downloading without confirmation");
    noConfirmArg.setCombinable(true);
    Argument urlsArg("urls", 'u', "specifies the URLs to download");
    urlsArg.setRequiredValueCount(-1);
    urlsArg.setValueNames({ "URL1", "URL2", "URL3" });
    urlsArg.setImplicit(true);
    Argument downloadArg("download", 'd', "downloads the specified data");
    downloadArg.setDenotesOperation(true);
    downloadArg.setSubArguments({ &urlsArg, &noConfirmArg });
    downloadArg.setCallback(bind(Cli::download, argc, argv, _1, cref(urlsArg), cref(noConfirmArg)));
    parser.setMainArguments({ &qtConfigArgs.qtWidgetsGuiArg(), &downloadArg, &helpArg });
    // parse arguments
    parser.parseArgsOrExit(argc, argv);
    // set meta info for application
    if (qtConfigArgs.areQtGuiArgsPresent()) {
        return QtGui::runWidgetsGui(argc, argv, qtConfigArgs);
    }
    return 0;
}
