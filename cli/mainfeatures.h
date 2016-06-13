#ifndef CLI_MAINFEATURES_H
#define CLI_MAINFEATURES_H

#include <vector>

namespace ApplicationUtilities {
class Argument;
}

namespace Cli {

void download(int argc, char *argv[], const std::vector<const char *> &parameterValues, const ApplicationUtilities::Argument &urlsArg, const ApplicationUtilities::Argument &noConfirmArg);

}

#endif // CLI_MAINFEATURES_H
