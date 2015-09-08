#ifndef YOUTUBEDOWNLOAD_H
#define YOUTUBEDOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

#include <QStringList>
#include <QHash>
#include <QJsonObject>

namespace Network {

class YoutubeDownload : public HttpDownloadWithInfoRequst
{
    Q_OBJECT

public:
    explicit YoutubeDownload(const QUrl &url, QObject *parent = nullptr);
    explicit YoutubeDownload(const QString &id, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &sucess, QString &reasonForFail);
    QString videoInfo(QString field, const QString &defaultValue);
    QString suitableFilename() const;
    QString typeName() const;

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    QHash<QString, QString> m_fields;
    QStringList m_itags;
    static QJsonObject m_itagInfo;
};

}

#endif // YOUTUBEDOWNLOAD_H
