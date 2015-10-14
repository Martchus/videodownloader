#ifndef NETWORK_VIMEODOWNLOAD_H
#define NETWORK_VIMEODOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

namespace Network {

class VimeoDownload : public HttpDownloadWithInfoRequst
{
    Q_OBJECT

public:
    explicit VimeoDownload(const QUrl &url, QObject *parent = nullptr);
    explicit VimeoDownload(const QString &id, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &success, QString &reasonForFail);
    QString suitableFilename() const;
    QString typeName() const;

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

};

} // namespace Network

#endif // NETWORK_VIMEODOWNLOAD_H
