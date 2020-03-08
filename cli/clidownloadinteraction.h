#ifndef CLI_DOWNLOADINTERACTION_H
#define CLI_DOWNLOADINTERACTION_H

#include <QObject>
#include <QSslError>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

namespace Network {
class Download;
}

namespace Cli {

class CliDownloadInteraction : public QObject {
    Q_OBJECT
public:
    explicit CliDownloadInteraction(QObject *parent = nullptr);

public Q_SLOTS:
    void connectDownload(Network::Download *download);
    void disconnectDownload(Network::Download *download);

private Q_SLOTS:
    void downloadRequiresOutputDevice(Network::Download *download, size_t optionIndex);
    void downloadRequiresOutputDevice(Network::Download *download, size_t optionIndex, bool forceFileDialog);
    void downloadRequriesOverwritePermission(Network::Download *download, size_t optionIndex, const QString &file);
    void downloadRequriesAppendingPermission(Network::Download *download, size_t optionIndex, const QString &file, quint64 offset, quint64 fileSize);
    void downloadRequiresRedirectionPermission(Network::Download *download, size_t optionIndex);
    void downloadRequiresAuthentication(Network::Download *download, size_t optionIndex, const QString &realm);
    void downloadHasSslErrors(Network::Download *download, size_t optionIndex, const QList<QSslError> &sslErrors);
};
} // namespace Cli

#endif // CLI_DOWNLOADINTERACTION_H
