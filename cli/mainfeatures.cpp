#include "./mainfeatures.h"

#include "../network/download.h"
#include "../network/groovesharkdownload.h"
#include "../network/httpdownload.h"
#include "../network/youtubedownload.h"

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/conversion/stringconversion.h>

#include <QCoreApplication>

#include <iostream>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;
using namespace Network;

namespace Cli {

void download(int argc, char *argv[], const ArgumentOccurrence &, const Argument &urlsArg, const Argument &noConfirmArg)
{
    CMD_UTILS_START_CONSOLE;
    // init Qt
    QCoreApplication app(argc, argv);
    QObject rootObj;
    // instantiate downloads
    QList<Download *> downloads;
    downloads.reserve(urlsArg.values().size());
    QVariant currentTargetDirectory;
    QVariant currentTargetName;
    enum { Auto, HttpUrl, YoutubeUrl, YoutubeId, GroovesharkId } currentDownloadType = Auto;
    size_t specifiedDownloads = 0;
    for (const auto &val : urlsArg.values()) {
        // check whether value denotes target directory or download type
        auto parts = splitString<vector<string> >(val, "=", EmptyPartsTreat::Keep, 2);
        if (parts.size() >= 2) {
            if (parts.front() == "type") {
                // value denotes download type
                if (parts.back() == "http") {
                    currentDownloadType = HttpUrl;
                } else if (parts.back() == "youtubeurl") {
                    currentDownloadType = YoutubeUrl;
                } else if (parts.back() == "youtubeid") {
                    currentDownloadType = YoutubeId;
                } else if (parts.back() == "groovesharkid") {
                    currentDownloadType = GroovesharkId;
                } else {
                    currentDownloadType = Auto;
                    if (parts.back() != "auto") {
                        cout << "Specified type \"" << parts.back() << "\" is invalid; using auto-detection." << endl;
                    }
                }
                continue;
            } else if (parts.front() == "targetdir") {
                currentTargetDirectory = QString::fromStdString(parts.back());
                continue;
            } else if (parts.front() == "targetname") {
                currentTargetName = QString::fromStdString(parts.back());
                continue;
            }
        }
        // do actual instantiation of new download object
        Download *download;
        QString qstrVal = QString::fromStdString(val);
        switch (currentDownloadType) {
        case HttpUrl:
            download = new HttpDownload(QUrl(qstrVal));
            break;
        case YoutubeUrl:
            download = new YoutubeDownload(QUrl(qstrVal));
            break;
        case YoutubeId:
            download = new YoutubeDownload(qstrVal);
            break;
        case GroovesharkId:
            download = new GroovesharkDownload(qstrVal);
            break;
        default:
            download = Download::fromUrl(qstrVal);
            break;
        }
        // set properties of download accordingly current configuration
        if (download) {
            download->setParent(&rootObj);
            if (!currentTargetDirectory.isNull()) {
                download->setProperty("targetdir", currentTargetDirectory);
            }
            if (!currentTargetName.isNull()) {
                download->setProperty("targetname", currentTargetName);
                currentTargetName.clear();
            }
            downloads << download;
            cout << "Specified URL \"" << val << "\" has been added as " << download->typeName().toStdString() << " download." << endl;
        } else {
            cout << "Specified URL \"" << val << "\" can not be added." << endl;
        }
        ++specifiedDownloads;
    }
    // check whether downloads could be instantiated, print appropiate error messages if not
    if (!specifiedDownloads) {
        cout << "No downloads have been specified." << endl;
    } else if (downloads.isEmpty()) {
        cout << "None of the specified downloads could be added." << endl;
    } else {
        // ask the user to continue
        if (!noConfirmArg.isPresent() && !confirmPrompt("Do you want to start these downloads?", Response::Yes)) {
            return;
        }
        cout << "Starting downloads ..." << endl;
        //for(Download *download : downloads) {
        //    download->init();
        //}
        cout << "TODO" << endl;
    }
}
}
