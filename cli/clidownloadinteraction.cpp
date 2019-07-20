#include "./clidownloadinteraction.h"

#include "../network/download.h"
#include "../network/permissionstatus.h"

#include <c++utilities/application/commandlineutils.h>

#include <iostream>

using namespace std;
using namespace Network;
using namespace CppUtilities;

namespace Cli {

CliDownloadInteraction::CliDownloadInteraction(QObject *parent)
    : QObject(parent)
{
}

void CliDownloadInteraction::connectDownload(Download *download)
{
    connect(download, &Download::outputDeviceRequired, this,
        static_cast<void (CliDownloadInteraction::*)(Download *, size_t)>(&CliDownloadInteraction::downloadRequiresOutputDevice),
        Qt::QueuedConnection);
    connect(download, &Download::overwriteingPermissionRequired, this, &CliDownloadInteraction::downloadRequriesOverwritePermission,
        Qt::QueuedConnection);
    connect(
        download, &Download::appendingPermissionRequired, this, &CliDownloadInteraction::downloadRequriesAppendingPermission, Qt::QueuedConnection);
    connect(download, &Download::redirectionPermissonRequired, this, &CliDownloadInteraction::downloadRequiresRedirectionPermission,
        Qt::QueuedConnection);
    connect(download, &Download::authenticationRequired, this, &CliDownloadInteraction::downloadRequiresAuthentication, Qt::QueuedConnection);
    connect(download, &Download::sslErrors, this, &CliDownloadInteraction::downloadHasSslErrors, Qt::QueuedConnection);
}

void CliDownloadInteraction::disconnectDownload(Download *download)
{
    download->disconnect(this);
}

void CliDownloadInteraction::downloadRequiresOutputDevice(Download *download, size_t optionIndex)
{
    downloadRequiresOutputDevice(download, optionIndex, false);
}

void CliDownloadInteraction::downloadRequiresOutputDevice(Download *download, size_t optionIndex, bool forceFileDialog)
{
    // TODO
}

void CliDownloadInteraction::downloadRequriesOverwritePermission(Download *download, size_t optionIndex, const QString &file)
{
    // TODO
}

void CliDownloadInteraction::downloadRequriesAppendingPermission(
    Download *download, size_t optionIndex, const QString &file, quint64 offset, quint64 fileSize)
{
    // TODO
}

void CliDownloadInteraction::downloadRequiresRedirectionPermission(Download *download, size_t optionIndex)
{
    // TODO
}

void CliDownloadInteraction::downloadRequiresAuthentication(Download *download, size_t optionIndex, const QString &realm)
{
    // TODO
}

void CliDownloadInteraction::downloadHasSslErrors(Download *download, size_t optionIndex, const QList<QSslError> &sslErrors)
{
    // TODO
    const string downloadName = (download->downloadUrl().isEmpty() ? download->id() : download->downloadUrl().toString()).toStdString();
    cout << "The download \"" << downloadName << "\" has SSL errors:" << endl;
    foreach (const QSslError &error, sslErrors) {
        cout << "- " << error.errorString().toStdString() << ":" << endl;
        cout << "  " << error.certificate().toText().toStdString() << endl;
    }
    if (confirmPrompt("Do you want to ignore the SSL errors for this download?")) {
        // TODO
    }
}
} // namespace Cli
