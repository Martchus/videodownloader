#ifndef HTTPDOWNLOADWITHINFOREQUST_H
#define HTTPDOWNLOADWITHINFOREQUST_H

#include "./httpdownload.h"

#include <QBuffer>
#include <QList>
#include <QMap>

#include <memory>

namespace Network {

class HttpDownloadWithInfoRequst : public HttpDownload {
    Q_OBJECT

public:
    explicit HttpDownloadWithInfoRequst(const QUrl &url, QObject *parent = nullptr);
    ~HttpDownloadWithInfoRequst();

    virtual Download *infoRequestDownload(bool &success, QString &reasonForFail) = 0;
    void doInit();
    void abortDownload();

protected:
    virtual void evalVideoInformation(Download *, QBuffer *) = 0;

private slots:
    void infoRequestChangedStatus(Download *download);

private:
    std::unique_ptr<Download> m_infoDownload;
    std::unique_ptr<QBuffer> m_infoBuffer;
};
} // namespace Network

#endif // HTTPDOWNLOADWITHINFOREQUST_H
