#ifndef CLI_MAINFEATURES_H
#define CLI_MAINFEATURES_H

#include <vector>

namespace CppUtilities {
class Argument;
class ArgumentOccurrence;
} // namespace CppUtilities

namespace Cli {

void download(int argc, char *argv[], const CppUtilities::ArgumentOccurrence &parameterValues, const CppUtilities::Argument &urlsArg,
    const CppUtilities::Argument &noConfirmArg);
}

#endif // CLI_MAINFEATURES_H
