#include "./vimeodownload.h"

#include <c++utilities/chrono/timespan.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

using namespace ChronoUtilities;

namespace Network {

/*!
 * \class VimeoDownload
 * \brief Download implementation for Vimeo videos.
 */

/*!
 * \brief Constructs a new VimeoDownload for the specified \a url.
 */
VimeoDownload::VimeoDownload(const QUrl &url, QObject *parent) :
    HttpDownloadWithInfoRequst(url, parent)
{}

/*!
 * \brief Constructs a new VimeoDownload for the specified video \a id.
 */
VimeoDownload::VimeoDownload(const QString &id, QObject *parent) :
    HttpDownloadWithInfoRequst(QUrl(QStringLiteral("https://vimeo.com/%1").arg(id)), parent)
{}

Download *VimeoDownload::infoRequestDownload(bool &success, QString &reasonForFail)
{
    const auto pathParts = initialUrl().path(QUrl::FullyDecoded).splitRef(QChar('/'), QString::SkipEmptyParts);
    if(pathParts.size() < 2) {
        const auto &id = pathParts.back();
        bool isInt;
        id.toULongLong(&isInt);
        if(isInt) {
            setId(id.toString());
            success = true;
            return new HttpDownload(QUrl(QStringLiteral("https://player.vimeo.com/video/%1/config").arg(id.toString())));
        }
    }
    success = false;
    reasonForFail = tr("The video ID couldn't be identified.");
    return nullptr;
}

QString VimeoDownload::suitableFilename() const
{
    auto filename = Download::suitableFilename();
    if(!filename.endsWith(QLatin1String(".mp4"))) {
        filename.append(QStringLiteral(".mp4"));
    }
    return filename;
}

QString VimeoDownload::typeName() const
{
    return tr("Vimeo");
}

void VimeoDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(videoInfoBuffer->readAll(), &error);
    if(error.error == QJsonParseError::NoError) {
        const auto h264Object = doc.object().value(QStringLiteral("request"))
                .toObject().value(QStringLiteral("files"))
                .toObject().value(QStringLiteral("h264")).toObject();
        const auto videoObject = doc.object().value(QStringLiteral("video")).toObject();
        const auto title = videoObject.value(QStringLiteral("title")).toString();
        if(!title.isEmpty()) {
            setTitle(title);
        }
        const auto uploader = videoObject.value(QStringLiteral("owner")).toObject().value(QStringLiteral("name")).toString();
        if(!uploader.isEmpty()) {
            setUploader(uploader);
        }
        if(const auto duration = videoObject.value(QStringLiteral("duration")).toInt()) {
            setDuration(TimeSpan::fromSeconds(duration));
        }
        for(const auto &value : h264Object) {
            const auto optionObject = value.toObject();
            const auto url = optionObject.value(QStringLiteral("url")).toString();
            if(!url.isEmpty()) {
                const auto width = optionObject.value(QStringLiteral("width")).toInt();
                const auto height = optionObject.value(QStringLiteral("height")).toInt();
                const auto bitrate = optionObject.value(QStringLiteral("bitrate")).toInt();
                addDownloadUrl(QStringLiteral("%1 x %2, %3 kbit/s").arg(width).arg(height).arg(bitrate), url);
            }
        }
        if(availableOptionCount() > 0) {
            reportInitiated(true);
        } else {
            reportInitiated(false, tr("No video URLs found. The video config could be parsed, but it seems like Vimeo changed something in their API."));
        }
    } else {
        reportInitiated(false, tr("Couldn't parse video configuration (invalid JSON)."));
    }
}

} // namespace Network

