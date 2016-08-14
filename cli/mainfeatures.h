#ifndef CLI_MAINFEATURES_H
#define CLI_MAINFEATURES_H

#include <vector>

namespace ApplicationUtilities {
class Argument;
class ArgumentOccurrence;
}

namespace Cli {

void download(int argc, char *argv[], const ApplicationUtilities::ArgumentOccurrence &parameterValues, const ApplicationUtilities::Argument &urlsArg, const ApplicationUtilities::Argument &noConfirmArg);

}

#endif // CLI_MAINFEATURES_H
