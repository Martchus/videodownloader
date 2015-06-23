#include "cli/mainfeatures.h"
#include "gui/initiate.h"

#if defined(GUI_QTWIDGETS) || defined(GUI_QTQUICK)
# include <qtutilities/resources/qtconfigarguments.h>
#else
# include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <c++utilities/application/failure.h>

#include <QCoreApplication>

#include <iostream>

#include "main.h"

using namespace std;
using namespace std::placeholders;
using namespace ApplicationUtilities;

int main(int argc, char *argv[])
{
    // setup argument parser
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
        cout << "Unable to parse arguments. " << ex.what() << "\nSee --help for available commands." << endl;
    }
    // set meta info for application
    QCoreApplication::setOrganizationName(QStringLiteral("Martchus"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("http://martchus.netai.net/"));
    QCoreApplication::setApplicationName(QStringLiteral("Video Downloader"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.7"));
    if(qtConfigArgs.areQtGuiArgsPresent()) {
        return QtGui::runWidgetsGui(argc, argv);
    }
    return 0;
}
