#include "./socksharedownload.h"

#include "../application/utils.h"

#include <QUrlQuery>

using namespace Application;
using namespace CppUtilities;

namespace Network {

/*!
 * \class SockshareDownload
 * \brief Download implementation for specific hoster.
 * \remarks Not maintained anymore.
 */

/*!
 * \brief Constructs a new SockshareDownload for the specified \a url.
 */
SockshareDownload::SockshareDownload(const QUrl &url, QObject *parent)
    : HttpDownloadWithInfoRequst(url, parent)
    , m_currentStep(0)
{
}

Download *SockshareDownload::infoRequestDownload(bool &success, QString &reasonForFail)
{
    HttpDownload *download;
    switch (m_currentStep) {
    case 0:
        download = new HttpDownload(initialUrl());
        success = true;
        return download;
    case 1:
        download = new HttpDownload(initialUrl());
        download->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        download->setMethod(HttpDownloadMethod::Post);
        download->setPostData(m_postData);
        success = true;
        return download;
    case 2:
        download = new HttpDownload(m_playlistUrl);
        success = true;
        return download;
    default:
        reasonForFail = tr("Internal error.");
        success = false;
        return nullptr;
    }
}

QString SockshareDownload::typeName() const
{
    return tr("Sockshare/Putlocker");
}

void SockshareDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    QString videoInfo(videoInfoBuffer->readAll());
    int pos;
    QString str;
    switch (m_currentStep) {
    case 0:
        if (substring(videoInfo, str, 0, QStringLiteral("<h1>"), QStringLiteral("<")) > 0 && !str.isEmpty()) {
            setTitle(str);
        }
        if (substring(videoInfo, str, 0, QStringLiteral("<input type=\"hidden\" value=\""), QStringLiteral("\"")) > 0) {
            if (str.isEmpty()) {
                reportInitiated(false, tr("Couldn't find the hash (empty \"value\"-attribute)."));
            } else {
                QUrlQuery query;
                query.addQueryItem(QStringLiteral("hash"), str);
                query.addQueryItem(QStringLiteral("confirm"), QStringLiteral("Continue as Free User"));
                m_postData.append(query.toString(QUrl::FullyEncoded));
                m_currentStep++;
                doInit();
            }
        } else
            reportInitiated(false, QStringLiteral("Couldn't find the hash."));
        break;
    case 1:
        if (substring(videoInfo, str, 0, QStringLiteral("get_file.php?id="), QStringLiteral("\"")))
            addDownloadUrl(tr("H.263/MP3/AVI"), QUrl(QStringLiteral("http://%1/get_file.php?id=%2").arg(initialUrl().host(), str)));
        if (substring(videoInfo, str, 0, QStringLiteral("get_file.php?"), QStringLiteral("'")) != -1) {
            m_playlistUrl = QUrl(QStringLiteral("http://%1/get_file.php?%2").arg(initialUrl().host(), str));
            ++m_currentStep;
            doInit();
        } else {
            if (availableOptionCount() > 0) {
                reportInitiated(true);
            } else {
                reportInitiated(false, tr("Couldn't find the \"get_file.php\"-url."));
            }
        }
        break;
    case 2:
        pos = videoInfo.indexOf(QLatin1String("<media:content"));
        if (pos > 0) {
            if (substring(videoInfo, str, pos, QStringLiteral("url=\""), QStringLiteral("\"")) > 0) {
                if (str.isEmpty()) {
                    if (availableOptionCount() < 1) {
                        reportInitiated(false, tr("The \"url\"-attribute in the \"media\"-tag is empty."));
                    } else {
                        reportInitiated(true);
                    }
                } else {
                    str = str.replace(QLatin1String("&amp;"), QLatin1String("&"));
                    addDownloadUrl(tr("H.264/AAC/FLV"), QUrl(str));
                    setChosenOption(availableOptionCount() - 1);
                    if (substring(videoInfo, str, pos, QStringLiteral("duration=\""), QStringLiteral("\""))) {
                        bool ok;
                        int duration = str.toInt(&ok);
                        if (ok)
                            setDuration(TimeSpan(duration));
                    }
                    reportInitiated(true);
                }
            } else {
                if (availableOptionCount() < 1) {
                    reportInitiated(false, tr("Couldn't find the \"url\"-attribute in the \"media\"-tag."));
                } else {
                    reportInitiated(true);
                }
            }
        } else {
            if (availableOptionCount() < 1) {
                reportInitiated(false, tr("Couldn't find the \"media\"-tag."));
            } else {
                reportInitiated(true);
            }
        }
        break;
    default:
        reportInitiated(false, tr("Internal error."));
    }
}
} // namespace Network
