#ifndef CLI_MAINFEATURES_H
#define CLI_MAINFEATURES_H

#include <vector>

namespace ApplicationUtilities {
class Argument;
class ArgumentOccurance;
}

namespace Cli {

void download(int argc, char *argv[], const ApplicationUtilities::ArgumentOccurance &parameterValues, const ApplicationUtilities::Argument &urlsArg, const ApplicationUtilities::Argument &noConfirmArg);

}

#endif // CLI_MAINFEATURES_H
