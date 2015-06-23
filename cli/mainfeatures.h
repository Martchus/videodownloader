#ifndef CLI_MAINFEATURES_H
#define CLI_MAINFEATURES_H

#include <vector>
#include <string>

namespace ApplicationUtilities {

typedef std::vector<std::string> StringVector;
class Argument;

}

namespace Cli {

void download(int argc, char *argv[], const ApplicationUtilities::StringVector &parameterValues, const ApplicationUtilities::Argument &noConfirmArg);

}

#endif // CLI_MAINFEATURES_H
