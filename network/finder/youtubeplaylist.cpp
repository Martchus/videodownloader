#include "./youtubeplaylist.h"

#include "../httpdownload.h"
#include "../youtubedownload.h"

#include "../../application/utils.h"

#include <QUrlQuery>

using namespace ChronoUtilities;
using namespace Application;

namespace Network {

/*!
 * \class YoutubePlaylist
 * \brief The YoutubePlaylist class retrieves videos from a YouTube playlist.
 */

/*!
 * \brief Constructs a new YouTube playlist with the specified \a url.
 */
YoutubePlaylist::YoutubePlaylist(const QUrl &url, QObject *parent) :
    DownloadFinder(parent)
{
    if(url.hasQuery()) {
        QUrlQuery query(url.query());
        m_playlistId = query.queryItemValue(QStringLiteral("list"));
    }
}

/*!
 * \brief Constructs a new YouTube playlist with the specified \a id.
 */
YoutubePlaylist::YoutubePlaylist(const QString &id, QObject *parent) :
    DownloadFinder(parent),
    m_playlistId(id)
{}

Download *YoutubePlaylist::createRequest(QString &reasonForFail)
{
    if(!m_playlistId.isEmpty()) {
        return new HttpDownload(QUrl(QStringLiteral("https://www.youtube.com/playlist?list=%1").arg(m_playlistId)), this);
    }
    reasonForFail = tr("The playlist ID couldn't be found.");
    return nullptr;
}

YoutubePlaylist::ParsingResult YoutubePlaylist::parseResults(const QByteArray &data, QString &reasonForFail)
{
    QString playlistInfo(data);
    QString collectionTitle;
    if(substring(playlistInfo, collectionTitle, 0, QStringLiteral("\"title\" content=\""), QStringLiteral("\"")) > 0) {
        replaceHtmlEntities(collectionTitle);
    }
    reportCollectionTitle(collectionTitle);
    YoutubeDownload *res = nullptr;
    int indexIndex = 0;
    int hrefIndex = 0;
    int nextAmpIndex = 0;
    int videoOwnerIndex = 0;
    QString id, title, uploader, durationStr;
    for(int index = 1; ; ++index) {
        id.clear();
        title.clear();
        uploader.clear();
        durationStr.clear();
        indexIndex = playlistInfo.indexOf(QLatin1String("<a class=\"pl-video-title-link"), indexIndex, Qt::CaseInsensitive);
        if(indexIndex > -1) {
            indexIndex = playlistInfo.indexOf(QLatin1String("&amp;index="), indexIndex, Qt::CaseInsensitive);
            if(indexIndex > -1) {
                hrefIndex = playlistInfo.lastIndexOf(QLatin1String("href=\"/watch?v="), indexIndex, Qt::CaseInsensitive);
                if(hrefIndex > -1) {
                    hrefIndex += 15;
                    // get video id
                    nextAmpIndex = playlistInfo.indexOf(QLatin1String("&amp;list="), hrefIndex, Qt::CaseInsensitive);
                    if(nextAmpIndex > hrefIndex) {
                        id = playlistInfo.mid(hrefIndex, nextAmpIndex - hrefIndex);
                    } else {
                        continue;
                    }
                    // get title
                    if(substring(playlistInfo, title, indexIndex, QStringLiteral("title=\""), QStringLiteral("\""))  > indexIndex) {
                        replaceHtmlEntities(title);
                    } else {
                        continue;
                    }
                    // get uploader
                    videoOwnerIndex = playlistInfo.indexOf(QLatin1String("<span class=\"video-owner\">"), indexIndex);
                    substring(playlistInfo, uploader, videoOwnerIndex, QStringLiteral(" dir=\"ltr\">"), QStringLiteral("</a>"));
                    // get duration
                    substring(playlistInfo, durationStr, videoOwnerIndex, QStringLiteral("<span class=\"video-time\">"), QStringLiteral("</span>"));
                    // construct download, provide obtained meta data, and report it as result
                    res = new YoutubeDownload(id);
                    res->provideMetaData(title, uploader, durationStr.isEmpty() ? TimeSpan() : TimeSpan::fromString(durationStr.toStdString()), collectionTitle, index);
                    reportResult(res);
                } else {
                    continue;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
    if(res) {
        return ParsingResult::Success;
    } else {
        reasonForFail = tr("No videos could be found (maybe the playlist ID is invalid).");
        return ParsingResult::Error;
    }
}

}
