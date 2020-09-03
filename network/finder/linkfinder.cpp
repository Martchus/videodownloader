#include "./linkfinder.h"

#include "../httpdownload.h"

#include "../../application/utils.h"

#include <QRegularExpression>

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
    static const QRegularExpression titlePattern(
        QStringLiteral("<title>(.+)</title>"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression linkPattern(
        QStringLiteral("<a([^>]+)>(.+)</a>"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression commentPattern(
        QStringLiteral("<!--(.+)-->"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression hrefPattern1(
        QStringLiteral("\\s*href\\s*=\\s*['](.+)['>]"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    static const QRegularExpression hrefPattern2(
        QStringLiteral("\\s*href\\s*=\\s*[\"](.+)[\">]"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::InvertedGreedinessOption);
    QString pageTitle;
    const auto titleMatch = titlePattern.match(html);
    if (titleMatch.hasMatch()) {
        pageTitle = titleMatch.captured(1);
        replaceHtmlEntities(pageTitle);
    }
    auto commentMatch = commentPattern.match(html, 0);
    decltype(commentMatch.capturedEnd()) overallIndex = 0;
    for (auto linkMatch = linkPattern.match(html, overallIndex); linkMatch.hasMatch(); linkMatch = linkPattern.match(html, overallIndex)) {
        if (commentMatch.capturedStart() >= 0 && commentMatch.capturedStart() < linkMatch.capturedStart()) {
            // skip comment
            overallIndex = commentMatch.capturedEnd();
            commentMatch = commentPattern.match(html, overallIndex);
            break;
        }
        // read actual link
        QString title = linkMatch.captured(2), href = linkMatch.captured(1), urlStr;
        const auto hrefMatch1 = hrefPattern1.match(href);
        if (hrefMatch1.hasMatch()) {
            urlStr = hrefMatch1.captured(1);
        } else {
            const auto hrefMatch2 = hrefPattern2.match(href);
            if (hrefMatch2.hasMatch()) {
                urlStr = hrefMatch2.captured(1);
            }
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
            if (Download *const duplicateDownload = downloadByInitialUrl(url)) {
                if (!title.isEmpty() && duplicateDownload->title().isEmpty()) {
                    duplicateDownload->provideMetaData(title);
                }
            } else if (Download *result = Download::fromUrl(url)) {
                result->provideMetaData(title, QString(), TimeSpan(), pageTitle, results().size());
                reportResult(result);
            }
        }
        overallIndex = linkMatch.capturedEnd();
    }
    return DownloadFinder::ParsingResult::Success;
}
} // namespace Network
