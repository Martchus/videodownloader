#include "./bitsharedownload.h"

#include "../application/utils.h"

#include <QUrlQuery>

using namespace Application;

namespace Network {

/*!
 * \class BitshareDownload
 * \brief Download implementation for specific hoster.
 */

/*!
 * \brief Constructs a new BitshareDownload for the specified \a url.
 */
BitshareDownload::BitshareDownload(const QUrl &url, QObject *parent)
    : HttpDownloadWithInfoRequst(url, parent)
{
}

Download *BitshareDownload::infoRequestDownload(bool &success, QString &)
{
    success = true;
    HttpDownload *download = new HttpDownload(initialUrl());
    download->setCookieJar(usedCookieJar());
    return download;
}

QString BitshareDownload::typeName() const
{
    return tr("Bitshare");
}

void BitshareDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    QString videoInfo(videoInfoBuffer->readAll());
    QString title;
    if (substring(videoInfo, title, 0, QStringLiteral("<title>Streaming "), QStringLiteral(" ")) > 0 && !title.isEmpty()) {
        setTitle(title);
    }
    QString url;
    if (substring(videoInfo, url, videoInfo.indexOf(QLatin1String("clip:"), Qt::CaseSensitive), QStringLiteral("url: '"), QStringLiteral("'")) > 0
        && !url.isEmpty()) {
        addDownloadUrl(tr("H.264/AAC/FLV"), url);
        reportInitiated(true);
    } else {
        reportInitiated(false, tr("The stream url couldn't be found."));
    }
}
}
