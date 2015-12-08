#include "./youtubedownload.h"

#include "../application/utils.h"

// include configuration from separate header file when building with CMake
#ifndef APP_METADATA_AVAIL
#include "resources/config.h"
#endif

#include <qtutilities/resources/resources.h>

#include <QUrlQuery>
#include <QJsonDocument>

using namespace ChronoUtilities;
using namespace Application;

namespace Network {

QJsonObject YoutubeDownload::m_itagInfo = QJsonObject();

/*!
 * \class YoutubeDownload
 * \brief Download implementation for YouTube videos.
 */

/*!
 * \brief Constructs a new YoutubeDownload for the specified \a url.
 */
YoutubeDownload::YoutubeDownload(const QUrl &url, QObject *parent) :
    HttpDownloadWithInfoRequst(url, parent)
{}

/*!
 * \brief Constructs a new YoutubeDownload for the specified video \a id.
 */
YoutubeDownload::YoutubeDownload(const QString &id, QObject *parent) :
    HttpDownloadWithInfoRequst(QUrl(QStringLiteral("http://www.youtube.com/watch?v=%1").arg(id)), parent)
{}

Download *YoutubeDownload::infoRequestDownload(bool &success, QString &reasonForFail)
{
    const QUrl &url = initialUrl();
    QString videoId;
    if(url.hasQuery()) {
        videoId = QUrlQuery(url.query(QUrl::FullyDecoded)).queryItemValue("v", QUrl::FullyDecoded);
    } else if(url.host(QUrl::FullyDecoded).contains(QLatin1String("youtu.be"), Qt::CaseInsensitive)) {
        videoId = url.path(QUrl::FullyDecoded);
        videoId.remove(0, 1);
    }
    if(videoId.isEmpty()) {
        success = false;
        reasonForFail = tr("The video ID couldn't be identified.");
        return nullptr;
    } else {
        setId(videoId);
        success = true;
        return new HttpDownload(QUrl(QStringLiteral("http://www.youtube.com/get_video_info?video_id=%1&asv=3&el=detailpage&hl=en_US").arg(videoId)));
    }
}

void YoutubeDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    if(m_itagInfo.isEmpty()) {
        // allow an external config file to be used instead of built-in values
        QString path = ConfigFile::locateConfigFile(QStringLiteral(PROJECT_NAME), QStringLiteral("json/itaginfo.json"));
        if(path.isEmpty()) {
            path = QStringLiteral(":/jsonobjects/itaginfo");
        }
        m_itagInfo = loadJsonObjectFromResource(path);
    }
    QString videoInfo(videoInfoBuffer->readAll());
    QStringList completeFields = videoInfo.split(QChar('&'), QString::SkipEmptyParts, Qt::CaseSensitive);
    foreach(QString completeField, completeFields) {
        QStringList fieldParts = completeField.split(QChar('='), QString::SkipEmptyParts, Qt::CaseSensitive);
        if(fieldParts.count() < 2) {
            continue;
        }
        m_fields.insert(QUrl::fromPercentEncoding(fieldParts.at(0).toUtf8()), QUrl::fromPercentEncoding(fieldParts.at(1).toUtf8()));
    }
    QString status = m_fields.value(QStringLiteral("status"));
    if(status == QLatin1String("ok")) {
        QString title = m_fields.value(QStringLiteral("title"));
        if(!title.isEmpty()) {
            setTitle(title.replace(QChar('+'), QChar(' ')));
        }
        QString uploader = m_fields.value(QStringLiteral("author"));
        if(!uploader.isEmpty()) {
            setUploader(uploader.replace(QChar('+'), QChar(' ')));
        }
        bool ok;
        double duration = m_fields.value(QStringLiteral("length_seconds")).toDouble(&ok);
        if(ok) {
            setDuration(TimeSpan::fromSeconds(duration));
        }
        QString rating = m_fields.value(QStringLiteral("avg_rating"));
        if(!rating.isEmpty()) {
            setRating(rating);
        }
        QStringList fmtFieldIds = QStringList() << QStringLiteral("url_encoded_fmt_stream_map") << QStringLiteral("adaptive_fmts");
        foreach(const QString &fmtFieldId, fmtFieldIds) {
            QString fmtField = m_fields.value(fmtFieldId, QString());
            if(!fmtField.isEmpty()) {
                QStringList sections = fmtField.split(QChar(','), QString::SkipEmptyParts, Qt::CaseSensitive);
                foreach(QString section, sections) {
                    QStringList fmtParts = section.split(QChar('&'), QString::SkipEmptyParts, Qt::CaseSensitive);
                    QString itag, urlPart1, urlPart2, name;
                    QJsonObject itagObj;
                    foreach(QString fmtPart, fmtParts) {
                        QStringList fmtSubParts = fmtPart.split(QChar('='), QString::SkipEmptyParts, Qt::CaseSensitive);
                        if(fmtSubParts.count() >= 2) {
                            QString fieldIdentifier = fmtSubParts.at(0).toLower();
                            if(fieldIdentifier == QLatin1String("url")) {
                                urlPart1 = QUrl::fromPercentEncoding(fmtSubParts.at(1).toUtf8());
                            } else if(fieldIdentifier == QLatin1String("sig")) {
                                urlPart2 = QUrl::fromPercentEncoding(fmtSubParts.at(1).toUtf8());
                            } else if(fieldIdentifier == QLatin1String("itag")) {
                                itag = fmtSubParts.at(1);
                            }
                        }
                    }
                    if(!itag.isEmpty() && !urlPart1.isEmpty()) {
                        if(m_itagInfo.contains(itag)) {
                            itagObj = m_itagInfo.value(itag).toObject();
                            name.append(itagObj.value(QStringLiteral("container")).toString());
                            if(!itagObj.value(QStringLiteral("videoCodec")).isNull()) {
                                name.append(", ");
                                name.append(itagObj.value(QStringLiteral("videoResolution")).toString());
                            }
                            if(itagObj.value(QStringLiteral("videoCodec")).isNull()) {
                                name.append(tr(", no video"));
                            }
                            if(itagObj.value(QStringLiteral("audioCodec")).isNull()) {
                                name.append(tr(", no audio"));
                            }
                            name.append(QStringLiteral(" ("));
                            name.append(itag);
                            name.append(QStringLiteral(")"));
                        } else {
                            name = itag;
                        }
                        QByteArray url;
                        url.append(urlPart1);
                        if(!urlPart2.isEmpty()) {
                            url.append("&signature=");
                            url.append(urlPart2);
                        }
                        addDownloadUrl(name, QUrl::fromPercentEncoding(url));
                        m_itags.append(itag);
                    }
                }
            }
        }
        if(availableOptionCount()) {
            reportInitiated(true);
        } else {
            reportInitiated(false, tr("Couldn't pharse the video info. The status of the video info is ok, but it seems like YouTube changed something in their API."));
        }
    } else {
        QString reason = m_fields.value("reason");
        if(reason.isEmpty()) {
            reportInitiated(false, tr("Failed to retieve the video info. The reason couldn't be identified. It seems like YouTube changed something in their API."));
        } else {
            reportInitiated(false, tr("Failed to retieve the video info. The reason returned by Youtube is: \"%1\".").arg(reason.replace(QChar('+'), QChar(' '))));
        }
    }
}


QString YoutubeDownload::videoInfo(QString field, const QString &defaultValue)
{
    return m_fields.value(field, defaultValue);
}

QString YoutubeDownload::suitableFilename() const
{
    auto filename = Download::suitableFilename();
    // get chosen option, the original option (not the redirection!) is required
    auto originalOption = chosenOption();
    while(originalOption != options().at(originalOption).redirectionOf()) {
        originalOption = options().at(originalOption).redirectionOf();
    }
    QString extension;
    if(originalOption < static_cast<size_t>(m_itags.size())) {
        const auto itag = m_itags.at(originalOption);
        if(m_itagInfo.contains(itag)) {
            const auto itagObj = m_itagInfo.value(itag).toObject();
            extension = itagObj.value(QStringLiteral("ext")).toString();
            if(extension.isEmpty()) {
                extension = itagObj.value(QStringLiteral("container")).toString().toLower();
            }
        }
    }
    if(extension.isEmpty()) {
        extension = QStringLiteral("flv"); // assume flv
    }
    extension.insert(0, QStringLiteral("."));
    if(!filename.endsWith(extension)) {
        filename.append(extension);
    }
    return filename;
}

QString YoutubeDownload::typeName() const
{
    return tr("YouTube");
}

}
