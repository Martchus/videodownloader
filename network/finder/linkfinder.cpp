#include "./linkfinder.h"

#include "../httpdownload.h"

#include "../../application/utils.h"

#include <QRegExp>

using namespace CppUtilities;
using namespace Application;

namespace Network {

/*!
 * \class LinkFinder
 * \brief The LinkFinder class retrieves links from an ordinary webpage.
 */

/*!
 * \brief Constructs a new link finder with the specified \a url.
 */
LinkFinder::LinkFinder(const QUrl &url, QObject *parent)
    : DownloadFinder(parent)
    , m_url(url)
{
}

Download *LinkFinder::createRequest(QString &)
{
    return new HttpDownload(m_url, this);
}

DownloadFinder::ParsingResult LinkFinder::parseResults(const QByteArray &data, QString &)
{
    QString html(data);
    QRegExp titlePattern(QStringLiteral("<title>(.+)</title>"), Qt::CaseInsensitive);
    QRegExp linkPattern(QStringLiteral("<a([^>]+)>(.+)</a>"), Qt::CaseInsensitive);
    QRegExp commentPattern(QStringLiteral("<!--(.+)-->"), Qt::CaseInsensitive);
    QRegExp hrefPattern1(QStringLiteral("\\s*href\\s*=\\s*['](.+)['>]"), Qt::CaseInsensitive);
    QRegExp hrefPattern2(QStringLiteral("\\s*href\\s*=\\s*[\"](.+)[\">]"), Qt::CaseInsensitive);
    titlePattern.setMinimal(true);
    linkPattern.setMinimal(true);
    commentPattern.setMinimal(true);
    hrefPattern1.setMinimal(true);
    hrefPattern2.setMinimal(true);
    QString pageTitle;
    if (titlePattern.indexIn(html) >= 0 && titlePattern.captureCount() >= 1) {
        pageTitle = titlePattern.cap(1);
        replaceHtmlEntities(pageTitle);
    }
    int overallIndex = 0;
    int commentIndex = commentPattern.indexIn(html, overallIndex);
    int linkIndex = 0;
    while (((linkIndex = linkPattern.indexIn(html, overallIndex)) >= 0)) {
        if (commentIndex >= 0 && commentIndex < linkIndex) {
            // skip comment
            overallIndex = commentIndex + commentPattern.matchedLength();
            commentIndex = commentPattern.indexIn(html, overallIndex);
        } else if (linkIndex >= 0) {
            // read actual link
            if (linkPattern.captureCount() >= 2) {
                QString title(linkPattern.cap(2));
                QString href(linkPattern.cap(1));
                QString urlStr;
                if (hrefPattern1.indexIn(href) >= 0 && hrefPattern1.captureCount() >= 1) {
                    urlStr = hrefPattern1.cap(1);
                } else if (hrefPattern2.indexIn(href) >= 0 && hrefPattern2.captureCount() >= 1) {
                    urlStr = hrefPattern2.cap(1);
                }
                if (!urlStr.isEmpty()) {
                    replaceHtmlEntities(title);
                    replaceHtmlEntities(urlStr);
                    // resolve relative URLs
                    QUrl url(urlStr);
                    if (url.isRelative()) {
                        url = m_url.resolved(url);
                    }
                    // avoid duplicate results
                    if (Download *duplicateDownload = downloadByInitialUrl(url)) {
                        if (!title.isEmpty() && duplicateDownload->title().isEmpty()) {
                            duplicateDownload->provideMetaData(title);
                        }
                    } else if (Download *result = Download::fromUrl(url)) {
                        result->provideMetaData(title, QString(), TimeSpan(), pageTitle, results().size());
                        reportResult(result);
                    }
                }
            }
            overallIndex = linkIndex + linkPattern.matchedLength();
        } else {
            // no more links
            break;
        }
    }
    return DownloadFinder::ParsingResult::Success;
}
} // namespace Network
