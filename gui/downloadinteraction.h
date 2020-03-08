#ifndef DOWNLOADINTERACTION_H
#define DOWNLOADINTERACTION_H

#include <QObject>
#include <QSslError>

QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QWidget)

namespace Network {
class Download;
}

namespace QtGui {

class DownloadInteraction : public QObject {
    Q_OBJECT
public:
    explicit DownloadInteraction(QObject *parent = nullptr);
    explicit DownloadInteraction(QWidget *parent = nullptr);

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

private:
    QWidget *m_parentWidget;
};
} // namespace QtGui

#endif // DOWNLOADINTERACTION_H
