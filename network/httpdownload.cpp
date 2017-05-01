#include "./httpdownload.h"

#include "./misc/contentdispositionparser.h"

#include <QFileInfo>

using namespace std;

namespace Network {

QNetworkAccessManager *HttpDownload::m_mgr = nullptr;

/*!
 * \class HttpDownloadInfo
 * \brief The HttpDownloadInfo class wraps a QNetworkReply, the corresponding option index and additional meta information.
 */

/*!
 * \class HttpDownload
 * \brief The HttpDownload class is an implementation of Download for HTTP and if OpenSSL is available
 *        HTTPS downloads. It is used as base class for more advanced HTTP downloads such as YouTube downloads.
 */

/*!
 * \brief Constructs a new HttpDownload with the specified \a url.
 */
HttpDownload::HttpDownload(const QUrl &url, QObject *parent)
    : Download(url, parent)
    ,
    //m_replies(nullptr),
    m_method(HttpDownloadMethod::Get)
    , m_redirectionIndex(-1)
{
    if (!m_mgr) {
        m_mgr = new QNetworkAccessManager();
    }
    // connect signals and slots
    connect(m_mgr, &QNetworkAccessManager::authenticationRequired, this, &HttpDownload::slotAuthenticationRequired);
#ifndef QT_NO_OPENSSL
    connect(m_mgr, &QNetworkAccessManager::sslErrors, this, &HttpDownload::slotSslErrors);
#endif
}

HttpDownload::~HttpDownload()
{
    qDeleteAll(m_replies);
}

void HttpDownload::doInit()
{
    setTitleFromFilename(readTitleFromUrl(initialUrl()));
    QString protocol = initialUrl().scheme();
    if (protocol.compare(QStringLiteral("http"), Qt::CaseInsensitive) || protocol.compare(QStringLiteral("https"), Qt::CaseInsensitive)
        || protocol.compare(QStringLiteral("ftp"), Qt::CaseInsensitive)) {
        addDownloadUrl(tr("Standard %1 download").arg(protocol), initialUrl());
        reportInitiated(true);
    } else {
        reportInitiated(false, tr("The protocol is not supported."));
    }
}

/*!
 * \brief Starts the request for the specified \a optionIndex.
 */
void HttpDownload::startRequest(size_t optionIndex)
{
    // apply current configuration
    m_mgr->setProxy(proxy());
    m_request.setUrl(downloadUrl(optionIndex));
    if (!userAgent().isEmpty()) {
        m_request.setHeader(QNetworkRequest::UserAgentHeader, userAgent().toLocal8Bit());
    }
    const DownloadRange &range = this->range(); // set range only if used
    if (range.isUsedForRequest()) {
        qint64 currentOffset = range.currentOffset();
        qint64 endOffset = range.endOffset();
        if (currentOffset > 0 || endOffset > 0) {
            QByteArray rangeVal;
            rangeVal.append("bytes=");
            if (currentOffset > 0) {
                rangeVal.append(QString::number(currentOffset));
            } else {
                rangeVal.append('0');
            }
            rangeVal.append('-');
            if (endOffset > 0) {
                rangeVal.append(QString::number(endOffset));
            }
            m_request.setRawHeader("Range", rangeVal);
        } else {
            if (m_request.hasRawHeader("Range")) {
                m_request.setRawHeader("Range", QByteArray());
            }
        }
    }
    // send request
    QNetworkReply *reply;
    switch (m_method) {
    case HttpDownloadMethod::Get:
        reply = m_mgr->get(m_request);
        break;
    case HttpDownloadMethod::Post:
        reply = m_mgr->post(m_request, m_postData);
        break;
    }
    m_replies << reply;
    reply->setProperty("optionindex", QVariant::fromValue(optionIndex));
    reply->setProperty("headerread", false);
    connect(reply, &QNetworkReply::downloadProgress, this, &HttpDownload::slotDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &HttpDownload::slotReadyRead);
    connect(reply, &QNetworkReply::finished, this, &HttpDownload::slotFinished);
}

/*!
 * \brief Returns a title derived from the specified \a url.
 */
QString HttpDownload::readTitleFromUrl(const QUrl &url)
{
    QString title = QFileInfo(url.path()).fileName();
    if (title.isEmpty()) {
        title = url.host();
    }
    return title;
}

void HttpDownload::checkStatusAndClear(size_t optionIndex)
{
    bool ok;
    for (QNetworkReply *reply : m_replies) {
        if (reply->property("optionindex").toUInt(&ok) == optionIndex && ok) {
            QString reasonForFail;
            QNetworkReply::NetworkError error = reply->error();
            if (error != QNetworkReply::NoError) {
                reasonForFail = reply->errorString();
                if ((error == QNetworkReply::OperationCanceledError) && (status() == DownloadStatus::Interrupting)) {
                    // download has been interrupted by the user
                    reportDownloadInterrupted(optionIndex);
                    reply->deleteLater();
                    m_replies.removeAll(reply);
                } else if (error == QNetworkReply::AuthenticationRequiredError) {
                    // authentication is required
                    reportAuthenticationRequired(optionIndex, m_realm);
                    reply->deleteLater();
                    m_replies.removeAll(reply);
                    // wrong to report a failed download here?
                    reportFinalDownloadStatus(optionIndex, false, reasonForFail, error);
                } else {
                    // some other error occured
                    reply->deleteLater();
                    m_replies.removeAll(reply);
                    reportFinalDownloadStatus(optionIndex, false, reasonForFail, error);
                }
            } else {
                // no error occured
                // check if there's a redirection
                QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
                reply->deleteLater();
                m_replies.removeAll(reply);
                if (!redirectionTarget.isNull()) {
                    // there's a redirection available
                    QUrl newUrl = downloadUrl().resolved(redirectionTarget.toUrl());
                    QString newOption;
                    int counter = 1;
                    size_t option = optionIndex;
                    while (option != options().at(option).redirectionOf()) {
                        option = options().at(option).redirectionOf();
                        ++counter;
                    }
                    if (counter > 1) {
                        newOption = tr("%1 - redirection (%2)").arg(optionName(option)).arg(counter);
                    } else {
                        newOption = tr("%1 - redirection").arg(optionName(option));
                    }
                    addDownloadUrl(newOption, newUrl, chosenOption());
                    reportRedirectionAvailable(optionIndex);
                } else {
                    // the download has been finished successfully
                    reportFinalDownloadStatus(optionIndex, true);
                }
            }
            return;
        }
    }
}

bool HttpDownload::followRedirection(size_t redirectionOptionIndex)
{
    setChosenOption(redirectionOptionIndex);
    if (isValidOptionChosen()) {
        setTitleFromFilename(readTitleFromUrl(downloadUrl()));
        size_t originalIndex = options().at(redirectionOptionIndex).redirectionOf();
        if (originalIndex < availableOptionCount()) {
            finalizeOutputDevice(originalIndex);
        }
        startRequest(redirectionOptionIndex);
        return true;
    }
    return false;
}

/*!
 * \brief Handles the authentication required signal emitted by the network reply.
 */
void HttpDownload::slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    if (m_replies.contains(reply)) {
        bool ok;
        auto optionIndex = reply->property("optionindex").toUInt(&ok);
        if (ok) {
            m_realm = authenticator->realm();
            AuthenticationCredentials &credentials = options().at(optionIndex).authenticationCredentials();
            if (!credentials.isIncomplete()) {
                authenticator->setUser(credentials.userName());
                authenticator->setPassword(credentials.password());
                // this slot will be called again if the credentials are wrong
                // so we need clear the supplied credentials to prevent in infinite loop
                credentials.clear();
            }
        }
    }
}

#ifndef QT_NO_OPENSSL
/*!
 * \brief Handles the SSL errors signal emitted by the network reply.
 */
void HttpDownload::slotSslErrors(QNetworkReply *reply, const QList<QSslError> &sslErrors)
{
    if (m_replies.contains(reply)) {
        bool ok;
        auto optionIndex = reply->property("optionindex").toUInt(&ok);
        if (ok) {
            reportSslErrors(optionIndex, reply, sslErrors);
        }
    }
}
#endif

/*!
 * \brief Handles the finished signal emitted by the network reply.
 */
void HttpDownload::slotFinished()
{
    bool ok;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    auto optionIndex = reply->property("optionindex").toUInt(&ok);
    if (ok) {
        if (reply->bytesAvailable()) {
            reportNewDataToBeWritten(optionIndex, reply);
        }
        reportDownloadComplete(optionIndex);
    }
}

/*!
 * \brief Handles the ready read signal emitted by the network reply.
 */
void HttpDownload::slotReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply->property("headerread").toBool()) {
        QVariant title = reply->header(QNetworkRequest::ContentDispositionHeader);
        if (title.isValid()) {
            ContentDispositionParser contentDisposition(title.toString());
            contentDisposition.pharse();
            QString fileName = contentDisposition.fileName();
            if (!fileName.isEmpty()) {
                setTitleFromFilename(fileName);
                reply->setProperty("headerread", true);
            }
        }
        m_setCookie = reply->header(QNetworkRequest::SetCookieHeader);
    }
    bool ok;
    auto optionIndex = reply->property("optionindex").toUInt(&ok);
    if (ok) {
        reportNewDataToBeWritten(optionIndex, reply);
    }
}

/*!
 * \brief Handles the download progress signal emitted by the network reply.
 */
void HttpDownload::slotDownloadProgress(qint64 bytesReceived, qint64 bytesToReceive)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    bool ok;
    auto optionIndex = reply->property("optionindex").toUInt(&ok);
    if (ok) {
        reportDownloadProgressUpdate(optionIndex, bytesReceived, bytesToReceive);
    }
}
}
