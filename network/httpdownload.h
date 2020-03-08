#ifndef HTTPDOWNLOAD_H
#define HTTPDOWNLOAD_H

#include "./download.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkCookie>

namespace Network {

/*!
 * \brief Specifies the HTTP method.
 */
enum class HttpDownloadMethod {
    Get, /**< GET method */
    Post /**< POST method */
};

class HttpDownloadInfo {
public:
    HttpDownloadInfo(int optionIndex, QNetworkReply *reply);

    int optionIndex() const;
    QNetworkReply *reply() const;
    bool headerRead() const;
    void setHeaderRead(bool read);

private:
    int m_optionIndex;
    QNetworkReply *m_reply;
    bool m_headerRead;
};

/*!
 * \brief Constructs a new HTTP download info.
 */
inline HttpDownloadInfo::HttpDownloadInfo(int optionIndex, QNetworkReply *reply)
    : m_optionIndex(optionIndex)
    , m_reply(reply)
    , m_headerRead(false)
{
}

/*!
 * \brief Returns the option index.
 */
inline int HttpDownloadInfo::optionIndex() const
{
    return m_optionIndex;
}

/*!
 * \brief Returns the reply.
 */
inline QNetworkReply *HttpDownloadInfo::reply() const
{
    return m_reply;
}

/*!
 * \brief Returns whether the header has been read (default is false).
 */
inline bool HttpDownloadInfo::headerRead() const
{
    return m_headerRead;
}

/*!
 * \brief Sets whether the header has been read.
 */
inline void HttpDownloadInfo::setHeaderRead(bool read)
{
    m_headerRead = read;
}

class HttpDownload : public Download {
    Q_OBJECT

public:
    explicit HttpDownload(const QUrl &url, QObject *parent = nullptr);
    virtual ~HttpDownload();

    void doInit();
    void doDownload();
    void abortDownload();
    void checkStatusAndClear(size_t optionIndex);
    bool followRedirection(size_t redirectionOptionIndex);
    HttpDownloadMethod method() const;
    QString rawCookies() const;
    QList<QNetworkCookie> cookies() const;
    QNetworkCookieJar *usedCookieJar() const;
    void setCookieJar(QNetworkCookieJar *cookieJar);
    void setMethod(HttpDownloadMethod method);
    void setPostData(const QByteArray &postData);
    void setHeader(const QByteArray &headerName, const QByteArray &headerValue);
    void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &headerValue);
    bool isInitiatingInstantlyRecommendable() const;
    bool supportsRange() const;
    QString typeName() const;
    //bool isPending(QNetworkReply *reply) const;

private Q_SLOTS:
    void slotFinished();
    void slotReadyRead();
    void slotDownloadProgress(qint64 bytesReceived, qint64 bytesToReceive);
    void slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
#ifndef QT_NO_OPENSSL
    void slotSslErrors(QNetworkReply *reply, const QList<QSslError> &sslErrors);
#endif

private:
    void startRequest(size_t optionIndex);
    static QString readTitleFromUrl(const QUrl &url);
    static QNetworkAccessManager *m_mgr;
    QNetworkRequest m_request;
    QList<QNetworkReply *> m_replies;
    QByteArray m_postData;
    HttpDownloadMethod m_method;
    QVariant m_setCookie;
    int m_redirectionIndex;
    QString m_realm;
};

inline void HttpDownload::doDownload()
{
    if (isValidOptionChosen()) {
        startRequest(chosenOption());
    }
}

inline void HttpDownload::abortDownload()
{
    for (QNetworkReply *reply : m_replies) {
        reply->abort();
    }
}

/*!
 * \brief Returns the HTTP method.
 */
inline HttpDownloadMethod HttpDownload::method() const
{
    return m_method;
}

/*!
 * \brief Returns the raw cookie data.
 * \sa cookies()
 */
inline QString HttpDownload::rawCookies() const
{
    return m_setCookie.isValid() ? m_setCookie.toString() : QString();
}

/*!
 * \brief Returns the cookies as QNetworkCookie.
 */
inline QList<QNetworkCookie> HttpDownload::cookies() const
{
    return m_setCookie.isValid() ? qvariant_cast<QList<QNetworkCookie>>(m_setCookie) : QList<QNetworkCookie>();
}

/*!
 * \brief Returns the used cookie jar.
 */
inline QNetworkCookieJar *HttpDownload::usedCookieJar() const
{
    return m_mgr->cookieJar();
}

/*!
 * \brief Sets the cookie jar to be used.
 *
 * The HttpDownload takes ownership of the cookie jar.
 */
inline void HttpDownload::setCookieJar(QNetworkCookieJar *cookieJar)
{
    m_mgr->setCookieJar(cookieJar);
}

/*!
 * \brief Sets the HTTP method.
 */
inline void HttpDownload::setMethod(HttpDownloadMethod method)
{
    m_method = method;
}

/*!
 * \brief Sets the post data.
 */
inline void HttpDownload::setPostData(const QByteArray &postData)
{
    m_postData = postData;
}

/*!
 * \brief Sets header \a headerName to be of value \a headerValue.
 */
inline void HttpDownload::setHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
    m_request.setRawHeader(headerName, headerValue);
}

/*!
 * \brief Sets the \a header to be of value \a headerValue.
 */
inline void HttpDownload::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &headerValue)
{
    m_request.setHeader(header, headerValue);
}

inline bool HttpDownload::isInitiatingInstantlyRecommendable() const
{
    return true;
}

inline bool HttpDownload::supportsRange() const
{
    return true;
}

inline QString HttpDownload::typeName() const
{
    return initialUrl().scheme();
}
} // namespace Network

#endif // HTTPDOWNLOAD_H
