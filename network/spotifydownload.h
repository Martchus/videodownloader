#ifndef SPOTIFYDOWNLOAD_H
#define SPOTIFYDOWNLOAD_H

#include "httpdownloadwithinforequst.h"

namespace Network {

enum class SpotifyRequestType
{
    Invalid,
    GetAuthenticationData,
    Authenticate
};

class SpotifyDownload : public HttpDownloadWithInfoRequst
{
    Q_OBJECT

public:
    explicit SpotifyDownload(const QString &songid, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &sucess, QString &reasonForFail);
    QString typeName() const;

    static void resetSession();

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    // static fields
    static const QUrl m_spotifyUrl;
    static bool m_authenticationCredentialsValidated;
    static QString m_csrftoken;
    static QString m_trackingId;
    static QString m_referrer;
    static QString m_creationFlow;
    static QStringList m_supportedUseragents;

    // fields
    SpotifyRequestType m_currentRequest;
    int m_triesToGetAuthenticationData;
};

}

#endif // SPOTIFYDOWNLOAD_H
