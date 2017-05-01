#ifndef GROOVESHARKDOWNLOAD_H
#define GROOVESHARKDOWNLOAD_H

#include "./httpdownloadwithinforequst.h"

namespace Network {

/*!
 * \brief The GroovesharkRequestType enum defines the request types for Grooveshark downloads.
 */
enum class GroovesharkRequestType {
    SongStream, /**< requests a song stream */
    AlbumGetSongs, /**< requests song IDs of an album */
    PlaylistGetSongs, /**< requests song IDs of a playlist */
    ArtistGetSongs, /**< requests song IDs of an artist */
    SearchForAlbum, /**< requests album IDs matching a search term */
    SearchForPlaylist, /**< requests playlist IDs matching a search term */
    SearchForArtists /**< request artist IDs matching a search term */
};

/*!
 * \brief The GroovesharkGetSongsRequestData struct holds the request data for Grooveshark downloads.
 */
struct GroovesharkGetSongsRequestData {
    QString id;
    bool verified;
};

class GroovesharkDownload : public HttpDownloadWithInfoRequst {
    Q_OBJECT

public:
    explicit GroovesharkDownload(const QString &songId, QObject *parent = nullptr);
    explicit GroovesharkDownload(GroovesharkRequestType requestType, const QVariant &requestData, QObject *parent = nullptr);

    Download *infoRequestDownload(bool &success, QString &reasonForFail);
    bool isInitiatingInstantlyRecommendable() const;

    static QJsonValue sessionId();
    static QString communicationToken();
    static void resetSession();
    static QJsonValue generateTokenHash(QString method, int mode = 0);
    static QJsonValue generateSecretKey();
    static QJsonValue generateUuid();
    static QJsonValue generateDefaultCountry();
    static bool loadAuthenticationInformationFromFile(const QString &path, QString *errorMessage = nullptr);
    GroovesharkRequestType requestType() const;
    QString suitableFilename() const;
    QString typeName() const;

protected:
    void evalVideoInformation(Download *, QBuffer *videoInfoBuffer);

private:
    static HttpDownload *createJsonPostRequest(const QString &method, const QJsonObject &header, const QJsonObject &parameters, bool https = false);
    void setupFinalRequest();

    static QJsonValue m_sessionId;
    static QString m_token;
    const static QJsonValue m_uuid;
    const static QJsonValue m_country;
    static QJsonValue m_htmlClient;
    static QJsonValue m_jsClient;
    static QJsonValue m_clientRevision;
    static QJsonValue m_jsClientRevision;
    static QString m_htmlRandomizer;
    static QString m_jsRandomizer;
    const static QString m_anyRandomizer;
    const static QJsonValue m_privacy;
    static QByteArray m_referer;
    const static QByteArray m_accept;
    QByteArray m_json;
    QString m_cookie;
    QString m_streamHost;
    QString m_streamKey;
    int m_currentStep;
    GroovesharkRequestType m_requestType;
    QVariant m_requestData;
};
}

Q_DECLARE_METATYPE(Network::GroovesharkGetSongsRequestData)

#endif // GROOVESHARKDOWNLOAD_H
