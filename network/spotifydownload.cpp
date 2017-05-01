#include "./spotifydownload.h"

#include "../application/utils.h"

#include <QApplication>
#include <QClipboard>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUrlQuery>

using namespace Application;

namespace Network {

const QUrl SpotifyDownload::m_spotifyUrl = QUrl(QStringLiteral("https://play.spotify.com/"));
bool SpotifyDownload::m_authenticationCredentialsValidated = false;
QString SpotifyDownload::m_csrftoken = QString();
QString SpotifyDownload::m_trackingId = QString();
QString SpotifyDownload::m_referrer = QString();
QString SpotifyDownload::m_creationFlow = QString();
QStringList SpotifyDownload::m_supportedUseragents = QStringList()
    << QStringLiteral("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:23.0) Gecko/20100101 Firefox/23.0");

/*!
 * \class SpotifyDownload
 * \brief Download implementation for Spotify songs.
 * \remarks Under construction (not even rudimental finished).
 */

/*!
 * \brief Constructs a new SpotifyDownload for the specified \a songid.
 */
SpotifyDownload::SpotifyDownload(const QString &songid, QObject *parent)
    : HttpDownloadWithInfoRequst(QUrl(), parent)
    , m_currentRequest(SpotifyRequestType::Invalid)
    , m_triesToGetAuthenticationData(0)
{
    setId(songid);
}

QString SpotifyDownload::typeName() const
{
    return tr("Spotify");
}

void SpotifyDownload::resetSession()
{
    m_authenticationCredentialsValidated = false;
    m_csrftoken.clear();
    m_trackingId.clear();
    m_referrer.clear();
    m_creationFlow.clear();
}

Download *SpotifyDownload::infoRequestDownload(bool &success, QString &reasonForFail)
{
    HttpDownload *download = nullptr;
    if (m_csrftoken.isEmpty() || m_trackingId.isEmpty() /* || creationFlow.isEmpty()*/) {
        if (m_triesToGetAuthenticationData < 2) {
            download = new HttpDownload(m_spotifyUrl);
            download->setCustomUserAgent(m_supportedUseragents.at(0));
            success = true;
            ++m_triesToGetAuthenticationData;
            m_currentRequest = SpotifyRequestType::GetAuthenticationData;
        } else {
            reasonForFail = tr("Unable to find data required for autentication.");
            success = false;
        }
    } else if (!m_authenticationCredentialsValidated) {
        if (initialAuthenticationCredentials().isIncomplete()) {
            reportAuthenticationRequired(
                -1, tr("To download songs from Spotify authentication is required so you have to enter the credentials of your Spotify account."));
            reasonForFail = tr("Authentication credentials not given.");
            //networkError = QNetworkReply::AuthenticationRequiredError;
            success = false;
        } else {
            QUrl url(m_spotifyUrl);
            url.setPath(QStringLiteral("/xhr/json/auth.php"));
            download = new HttpDownload(url);
            QByteArray postData;
            QUrlQuery query;
            query.addQueryItem(QStringLiteral("type"), QStringLiteral("sp"));
            query.addQueryItem(QStringLiteral("username"), initialAuthenticationCredentials().userName());
            query.addQueryItem(QStringLiteral("password"), initialAuthenticationCredentials().password());
            query.addQueryItem(QStringLiteral("secret"), m_csrftoken);
            query.addQueryItem(QStringLiteral("trackingId"), m_trackingId);
            query.addQueryItem(QStringLiteral("landingURL"), m_spotifyUrl.toString());
            query.addQueryItem(QStringLiteral("cf"), m_creationFlow);
            postData.append(query.toString(QUrl::FullyEncoded));
            download->setMethod(HttpDownloadMethod::Post);
            download->setPostData(postData);
            download->setCustomUserAgent(m_supportedUseragents.at(0));

            success = true;
            m_currentRequest = SpotifyRequestType::Authenticate;
        }
    }
    return download;
}

void SpotifyDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    if (m_currentRequest == SpotifyRequestType::Invalid)
        reportInitiated(false, tr("Interal error (current request type not set)."));
    else {
        switch (m_currentRequest) {
        case SpotifyRequestType::GetAuthenticationData: {
            QString responseData(videoInfoBuffer->readAll());
            substring(responseData, m_csrftoken, 0, QStringLiteral("\"csrftoken\":\""), QStringLiteral("\""));
            substring(responseData, m_trackingId, 0, QStringLiteral("\"trackingId\":\""), QStringLiteral("\""));
            substring(responseData, m_creationFlow, 0, QStringLiteral("\"creationFlow\":\""), QStringLiteral("\""));
            substring(responseData, m_referrer, 0, QStringLiteral("\"referrer\":\""), QStringLiteral("\""));
            m_currentRequest = SpotifyRequestType::Invalid;
            doInit();
            break;
        }
        case SpotifyRequestType::Authenticate: {
            QByteArray responseData(videoInfoBuffer->readAll());
            QApplication::clipboard()->setText(QString(responseData));
            QJsonParseError error;
            QJsonDocument document = QJsonDocument::fromJson(responseData, &error);
            if (error.error == QJsonParseError::NoError) {
                QJsonObject obj = document.object();
                QJsonValue statusVal = obj.value(QStringLiteral("status"));
                if (statusVal.toString().compare(QLatin1String("ok"), Qt::CaseInsensitive) == 0) {
                    m_authenticationCredentialsValidated = true;
                    reportInitiated(true);
                } else {
                    QString error = obj.value(QStringLiteral("error")).toString();
                    QString message;
                    if (error.isEmpty()) {
                        message = tr("Authentication failed. Spotify returned no error message.");
                    } else if (error.compare(QLatin1String("invalid_credentials"), Qt::CaseInsensitive) == 0) {
                        message = tr("Authentication failed because the entered credentials are invalid.");
                    } else {
                        message = tr("Authentication failed. Error message returned by Spotify: %1").arg(error);
                    }
                    reportInitiated(false, message);
                }
            } else {
                reportInitiated(false,
                    tr("Authentication failed because the response by Spotify is no valid Json document (parse error: %1).")
                        .arg(error.errorString()));
            }
            break;
        }
        case SpotifyRequestType::Invalid:;
        }
    }
}
}
