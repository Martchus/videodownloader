#ifndef BITSHAREDOWNLOAD_H
#define BITSHAREDOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

namespace Network {

class BitshareDownload : public HttpDownloadWithInfoRequst
{
    Q_OBJECT

public:
    explicit BitshareDownload(const QUrl &url, QObject *parent = nullptr);

    virtual Download *infoRequestDownload(bool &success, QString &reasonForFail);
    virtual QString typeName() const;

protected:
    virtual void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    QByteArray m_postData;
};

}

#endif // BITSHAREDOWNLOAD_H
