#include "./filenukedownload.h"

#include "../application/utils.h"

#include <QUrlQuery>

using namespace Application;

namespace Network {

/*!
 * \class FileNukeDownload
 * \brief Download implementation for specific hoster.
 * \remarks Not maintained anymore.
 */

/*!
 * \brief Constructs a new FileNukeDownload for the specified \a url.
 */
FileNukeDownload::FileNukeDownload(const QUrl &url, QObject *parent)
    : HttpDownloadWithInfoRequst(url, parent)
    , m_currentStep(0)
{
}

void FileNukeDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    videoInfoBuffer->seek(0);
    QString videoInfo(videoInfoBuffer->readAll());
    int pos;
    QString str;
    QString id;
    QString fname;
    switch (m_currentStep) {
    case 0:
        if (substring(videoInfo, str, 0, QStringLiteral("<h1>"), QStringLiteral("<")) > 0 && !str.isEmpty()) {
            setTitle(str);
        }
        substring(videoInfo, id, 0, QStringLiteral("<input type=\"hidden\" name=\"id\" value=\""), QStringLiteral("\""));
        substring(videoInfo, fname, 0, QStringLiteral("<input type=\"hidden\" name=\"fname\" value=\""), QStringLiteral("\""));
        if (id.isEmpty()) {
            reportInitiated(false, tr("Couldn't find the id."));
        } else if (fname.isEmpty()) {
            reportInitiated(false, tr("Couldn't find the file name."));
        } else {
            QUrlQuery query;
            query.addQueryItem(QStringLiteral("op1"), QStringLiteral("download1"));
            query.addQueryItem(QStringLiteral("user_login"), QString());
            query.addQueryItem(QStringLiteral("id"), id);
            query.addQueryItem(QStringLiteral("fname"), fname);
            query.addQueryItem(QStringLiteral("referer"), QString());
            query.addQueryItem(QStringLiteral("method_free"), QStringLiteral("Free"));
            m_postData.append(query.toString(QUrl::FullyEncoded));
            ++m_currentStep;
            doInit();
        }
        break;
    case 1:
        pos = videoInfo.indexOf(QLatin1String(";return p}("));
        if (pos > 0) {
            pos = videoInfo.indexOf(QLatin1String(",'"), pos + 1);
            if (pos > 0) {
                substring(videoInfo, str, pos, QStringLiteral("'"), QStringLiteral("'"));
                QStringList parts = str.split(QChar('|'), QString::KeepEmptyParts);

                if (parts.count() >= 21) {
                    addDownloadUrl(tr("H.264/AAC/MP4"),
                        QUrl(QStringLiteral("http://%1.%2.%3.%4/d/%5/%6.%7")
                                 .arg(parts.at(8), parts.at(7), parts.at(6), parts.at(5), parts.at(20), parts.at(19), parts.at(18))));
                    reportInitiated(true);
                } else
                    reportInitiated(false, tr("Download link info is incomplete."));
            } else
                reportInitiated(false, tr("Download link info is missing (2)."));
        } else
            reportInitiated(false, tr("Download link info is missing (1)."));
        break;
    default:
        reportInitiated(false, tr("Internal error."));
    }
}

Download *FileNukeDownload::infoRequestDownload(bool &success, QString &reasonForFail)
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

QString FileNukeDownload::typeName() const
{
    return tr("FileNuke");
}
}
