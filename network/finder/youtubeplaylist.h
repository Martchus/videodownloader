#ifndef YOUTUBEPLAYLIST_H
#define YOUTUBEPLAYLIST_H

#include "downloadfinder.h"

namespace Network {

class YoutubePlaylist : public DownloadFinder
{
    Q_OBJECT
public:
    explicit YoutubePlaylist(const QUrl &url, QObject *parent = nullptr);
    explicit YoutubePlaylist(const QString &id, QObject *parent = nullptr);

protected:
    Download *createRequest(QString &reasonForFail);

protected slots:
    ParsingResult parseResults(const QByteArray &data, QString &reasonForFail);
    
private:
    QString m_playlistId;  
};

}

#endif // YOUTUBEPLAYLIST_H
