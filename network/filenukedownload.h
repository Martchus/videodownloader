#ifndef FILENUKE_DOWNLOAD_H
#define FILENUKE_DOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

namespace Network {

class FileNukeDownload : public HttpDownloadWithInfoRequst {
    Q_OBJECT

public:
    explicit FileNukeDownload(const QUrl &url, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &success, QString &reasonForFail);
    QString typeName() const;

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    int m_currentStep;
    QByteArray m_postData;
    QUrl m_playlistUrl;
};
}

#endif // FILENUKE_H
