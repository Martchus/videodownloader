#ifndef YOUTUBEDOWNLOAD_H
#define YOUTUBEDOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

#include <QHash>
#include <QJsonObject>
#include <QStringList>

namespace Network {

class YoutubeDownload : public HttpDownloadWithInfoRequst {
    Q_OBJECT

public:
    explicit YoutubeDownload(const QUrl &url, QObject *parent = nullptr);
    explicit YoutubeDownload(const QString &id, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &success, QString &reasonForFail);
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
} // namespace Network

#endif // YOUTUBEDOWNLOAD_H
