#ifndef SOCKSHAREDOWNLOAD_H
#define SOCKSHAREDOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

namespace Network {

class SockshareDownload : public HttpDownloadWithInfoRequst {
    Q_OBJECT

public:
    explicit SockshareDownload(const QUrl &url, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &success, QString &reasonForFail);
    QString typeName() const;

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    int m_currentStep;
    QByteArray m_postData;
    QUrl m_playlistUrl;
};
} // namespace Network

#endif // SOCKSHAREDOWNLOAD_H
