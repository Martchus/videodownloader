#include "./groovesharkdownload.h"

#include "../application/utils.h"

// include configuration from separate header file when building with CMake
#ifndef APP_METADATA_AVAIL
#include "resources/config.h"
#endif

#include <QUrlQuery>
#include <QCryptographicHash>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>

using namespace ChronoUtilities;
using namespace Application;

namespace Network {

QJsonValue GroovesharkDownload::m_sessionId = QJsonValue();

QString GroovesharkDownload::m_token = QString();

const QJsonValue GroovesharkDownload::m_uuid = generateUuid();

const QJsonValue GroovesharkDownload::m_country = generateDefaultCountry();

QJsonValue GroovesharkDownload::m_htmlClient = QJsonValue(QStringLiteral("htmlshark"));

QJsonValue GroovesharkDownload::m_jsClient = QJsonValue(QStringLiteral("jsqueue"));

QJsonValue GroovesharkDownload::m_clientRevision = QJsonValue(QStringLiteral("20130520"));

QJsonValue GroovesharkDownload::m_jsClientRevision = QJsonValue(QStringLiteral("20130520"));

QString GroovesharkDownload::m_htmlRandomizer = QStringLiteral(":nuggetsOfBaller:");

QString GroovesharkDownload::m_jsRandomizer = QStringLiteral(":chickenFingers:");

const QString GroovesharkDownload::m_anyRandomizer = QStringLiteral("asdfgh");

const QJsonValue GroovesharkDownload::m_privacy = QJsonValue(0);

QByteArray GroovesharkDownload::m_referer = QByteArray("http://grooveshark.com/JSQueue.swf?20120124.01");

const QByteArray GroovesharkDownload::m_accept = QByteArray("text/html, image/jpeg, *; q=.2, */*; q=.2");

/*!
 * \class GroovesharkDownload
 * \brief Download implementation for Grooveshark songs.
 */

/*!
 * \brief Constructs a new GroovesharkDownload for the specified \a songId.
 */
GroovesharkDownload::GroovesharkDownload(const QString &songId, QObject *parent) :
    GroovesharkDownload(GroovesharkRequestType::SongStream, songId, parent)
{}

GroovesharkDownload::GroovesharkDownload(GroovesharkRequestType requestType, const QVariant &requestData, QObject *parent) :
    HttpDownloadWithInfoRequst(QUrl(), parent),
    m_currentStep(0),
    m_requestType(requestType),
    m_requestData(requestData)
{
    // check if "steps" to establish session can be skiped because session is already set    
    m_currentStep = m_sessionId.isNull()
            ? 0
            : (m_token.isEmpty()
               ? 1
               : (requestType == GroovesharkRequestType::SongStream
                  ? 2
                  : -1));
    switch(requestType) {
    case GroovesharkRequestType::SongStream:
        if(requestData.type() == QVariant::String)
            setId(requestData.toString());
        break;
    case GroovesharkRequestType::AlbumGetSongs:
    case GroovesharkRequestType::PlaylistGetSongs:
    case GroovesharkRequestType::ArtistGetSongs:
        if(requestData.canConvert<GroovesharkGetSongsRequestData>())
            setId(requestData.value<GroovesharkGetSongsRequestData>().id);
        else if(requestData.type() == QVariant::String)
            setId(requestData.toString());
        break;
    case GroovesharkRequestType::SearchForAlbum:
    case GroovesharkRequestType::SearchForPlaylist:
    case GroovesharkRequestType::SearchForArtists:
        break;
    }
}

Download *GroovesharkDownload::infoRequestDownload(bool &success, QString &reasonForFail)
{
    Download *download = nullptr;
    QJsonObject headerObj;
    QJsonObject paramObj;
    switch(m_currentStep) {
    case -1:
        success = true;
        break;
    case 0:
        if(m_sessionId.isNull()) {
            // initiateSession
            headerObj.insert(QStringLiteral("client"), m_htmlClient);
            headerObj.insert(QStringLiteral("clientRevision"), m_clientRevision);
            headerObj.insert(QStringLiteral("privacy"), m_privacy);
            headerObj.insert(QStringLiteral("uuid"), m_uuid);
            headerObj.insert(QStringLiteral("country"), m_country);
            download = createJsonPostRequest(QStringLiteral("initiateSession"), headerObj, paramObj);
        }
        success = true;
        break;
    case 1:
        if(m_token.isEmpty()) {
            // getCommunicationToken
            headerObj.insert(QStringLiteral("client"), m_htmlClient);
            headerObj.insert(QStringLiteral("clientRevision"), m_clientRevision);
            headerObj.insert(QStringLiteral("session"), m_sessionId);
            headerObj.insert(QStringLiteral("token"), generateTokenHash(QStringLiteral("getCommunicationToken")));
            headerObj.insert(QStringLiteral("privacy"), m_privacy);
            headerObj.insert(QStringLiteral("uuid"), m_uuid);
            paramObj.insert(QStringLiteral("secretKey"), generateSecretKey());
            download = createJsonPostRequest(QStringLiteral("getCommunicationToken"), headerObj, paramObj, true);
        }
        success = true;
        break;
    case 2:
        // getStreamKeyFromSongIDEx
        headerObj.insert(QStringLiteral("uuid"), m_uuid);
        headerObj.insert(QStringLiteral("privacy"), m_privacy);
        headerObj.insert(QStringLiteral("session"), m_sessionId);
        headerObj.insert(QStringLiteral("clientRevision"), m_clientRevision);
        headerObj.insert(QStringLiteral("client"), m_jsClient);
        headerObj.insert(QStringLiteral("token"), generateTokenHash(QStringLiteral("getStreamKeyFromSongIDEx"), 1));
        paramObj.insert(QStringLiteral("country"), m_country);
        paramObj.insert(QStringLiteral("mobile"), QJsonValue(false));
        paramObj.insert(QStringLiteral("type"), QJsonValue(0));
        paramObj.insert(QStringLiteral("songID"), QJsonValue(id()));
        paramObj.insert(QStringLiteral("prefetch"), QJsonValue(false));
        download = createJsonPostRequest("getStreamKeyFromSongIDEx", headerObj, paramObj);
        success = true;
        break;
    default:
        reasonForFail = QStringLiteral("Internal error.");
        success = false;
    }
    return download;
}

bool GroovesharkDownload::isInitiatingInstantlyRecommendable() const
{
    return false;
}

/*!
 * \brief Returns the session ID.
 */
QJsonValue GroovesharkDownload::sessionId()
{
    return m_sessionId;
}

/*!
 * \brief Returns the communication token.
 */
QString GroovesharkDownload::communicationToken()
{
    return m_token;
}

/*!
 * \brief Resets the current session (ID and communication token).
 */
void GroovesharkDownload::resetSession()
{
    m_sessionId = QJsonValue();
    m_token.clear();
}

void GroovesharkDownload::evalVideoInformation(Download *, QBuffer *videoInfoBuffer)
{
    QString code;
    if(videoInfoBuffer) { // the buffer might be zero!
        code.append(videoInfoBuffer->readAll());
    }
    switch(m_currentStep) {
    case -1:
        setupFinalRequest();
        reportInitiated(true);
        break;
    case 0: {
        QString value;
        if((substring(code, value, 0, QStringLiteral("\"session\":\""), QStringLiteral("\"")) <= 0) || value.isEmpty()) {
            if((substring(code, value, 0, QStringLiteral("\"message\":\""), QStringLiteral("\"")) <= 0) || value.isEmpty()) {
                reportInitiated(false, tr("The session couldn't be initialized."));
            } else {
                reportInitiated(false, tr("The session couldn't be initialized (%1).").arg(value));
            }
        } else {
            m_sessionId = QJsonValue(value);
            ++m_currentStep;
            doInit();
        }
        break;
    }
    case 1:
        if((substring(code, m_token, 0, QStringLiteral("\"result\":\""), QStringLiteral("\"")) <= 0) || m_token.isEmpty()) {
            reportInitiated(false, tr("The communication token couldn't be retireved."));
        } else {
            if(m_requestType == GroovesharkRequestType::SongStream) {
                ++m_currentStep;
                doInit();
            } else {
                setupFinalRequest();
                reportInitiated(true);
            }
        }
        break;
    case 2:
        if((substring(code, m_streamKey, 0, QStringLiteral("\"streamKey\":\""), QStringLiteral("\"")) <= 0)
                || m_streamKey.isEmpty()) {
            reportInitiated(false, tr("The stream key couldn't be found."));
        } else {
            if((substring(code, m_streamHost, 0, QStringLiteral("\"ip\":\""), QStringLiteral("\"")) <= 0)
                    || m_streamHost.isEmpty()) {
                reportInitiated(false, tr("The stream host couldn't be found."));
            } else {
                setupFinalRequest();
                reportInitiated(true);
            }
        }
        break;
    default:
        reportInitiated(false, tr("Internal error."));
    }
}

/*!
 * \brief Generates a UUID.
 */
QJsonValue GroovesharkDownload::generateUuid()
{
    QString res = QUuid::createUuid().toString();
    int pos = res.startsWith('{') ? 1 : 0;
    int length = res.endsWith('}') ? res.length() - pos - 1: -1;
    return QJsonValue(res.mid(pos, length));
}

/*!
 * \brief Generates the default country information.
 */
QJsonValue GroovesharkDownload::generateDefaultCountry()
{
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("IPR"), QJsonValue(0));
    jsonObj.insert(QStringLiteral("ID"), QJsonValue(0));
    jsonObj.insert(QStringLiteral("CC1"), QJsonValue(0));
    jsonObj.insert(QStringLiteral("CC2"), QJsonValue(0));
    jsonObj.insert(QStringLiteral("CC3"), QJsonValue(0));
    jsonObj.insert(QStringLiteral("CC4"), QJsonValue(0));
    return QJsonValue(jsonObj);
}

/*!
 * \brief Loads the authentication information from the specified file.
 */
bool GroovesharkDownload::loadAuthenticationInformationFromFile(const QString &path, QString *errorMessage)
{
    QJsonObject fileObj = loadJsonObjectFromResource(path, errorMessage);
    if(fileObj.isEmpty()) {
        return false;
    } else {
        QJsonValue clientVal;
        QJsonObject clientObj;
        QJsonValue name;
        QJsonValue revision;
        QJsonValue randomizer;
        clientVal = fileObj.value(QStringLiteral("htmlClient"));
        if(clientVal.isObject()) {
            clientObj = clientVal.toObject();
            name = clientObj.value(QStringLiteral("name"));
            if(name.isString()) {
                m_htmlClient = name;
            }
            revision = clientObj.value(QStringLiteral("revision"));
            if(revision.isString()) {
                m_clientRevision = revision;
            }
            randomizer = clientObj.value(QStringLiteral("randomizer"));
            if(randomizer.isString()) {
                m_htmlRandomizer = randomizer.toString();
            }
        }
        clientVal = fileObj.value(QStringLiteral("jsClient"));
        if(clientVal.isObject()) {
            clientObj = clientVal.toObject();
            name = clientObj.value(QStringLiteral("name"));
            if(name.isString()) {
                m_jsClient = name;
            }
            revision = clientObj.value(QStringLiteral("revision"));
            if(revision.isString()) {
                m_jsClientRevision = revision;
            }
            randomizer = clientObj.value(QStringLiteral("randomizer"));
            if(randomizer.isString()) {
                m_jsRandomizer = randomizer.toString();
            }
        }
        clientVal = fileObj.value(QStringLiteral("referer"));
        if(clientVal.isString()) {
            m_referer.append(clientVal.toString());
        }
        return true;
    }
}

/*!
 * \brief Generates the token hash with the specified \a method and \a mode.
 */
QJsonValue GroovesharkDownload::generateTokenHash(QString method, int mode)
{
    QByteArray toHash;
    toHash.append(method);
    toHash.append(':');
    toHash.append(m_token);
    switch(mode) {
    case 1:
        toHash.append(m_htmlRandomizer);
        break;
    default:
        toHash.append(m_jsRandomizer);
        break;
    }
    toHash.append(m_anyRandomizer);
    QString res;
    res.append(m_anyRandomizer);
    res.append(QString(QCryptographicHash::hash(toHash, QCryptographicHash::Sha1).toHex()).toLower());
    return QJsonValue(res);
}

/*!
 * \brief Generates the secret key.
 */
QJsonValue GroovesharkDownload::generateSecretKey()
{
    QByteArray toHash;
    toHash.append(m_sessionId.toString());
    QString secretKey(QCryptographicHash::hash(toHash, QCryptographicHash::Md5).toHex());
    return QJsonValue(secretKey);
}

/*!
 * \brief Creates a JSON post request.
 */
HttpDownload *GroovesharkDownload::createJsonPostRequest(const QString &method, const QJsonObject &header, const QJsonObject &parameters, bool https)
{
    // create json post data
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("method"), QJsonValue(method));
    jsonObj.insert(QStringLiteral("header"), header);
    jsonObj.insert(QStringLiteral("parameters"), parameters);
    QJsonDocument jsonDoc(jsonObj);
    QByteArray postData = jsonDoc.toJson();
    // create and setup download
    QString url = https ? QStringLiteral("https") : QStringLiteral("http");
    url.append(QStringLiteral("://grooveshark.com/more.php?"));
    url.append(method);
    HttpDownload *download = new HttpDownload(QUrl(url));
    download->setMethod(HttpDownloadMethod::Post);
    download->setPostData(postData);
    download->setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    download->setHeader("Refer", m_referer);
    download->setHeader("Accept", m_accept);
    //download->setHeader("Connection", "keep-alive");
    return download;
}

/*!
 * \brief Sets the final request up.
 */
void GroovesharkDownload::setupFinalRequest()
{
    setMethod(HttpDownloadMethod::Post);
    switch(m_requestType) {
    case GroovesharkRequestType::SongStream: {
        setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        setHeader("Accept", m_accept);
        QUrlQuery query;
        query.addQueryItem("streamKey", m_streamKey);
        QByteArray postData;
        postData.append(query.toString(QUrl::FullyEncoded));
        setPostData(postData);
        addDownloadUrl(tr("MPEG-1 Layer 3"), QUrl(QStringLiteral("http://%1/stream.php").arg(m_streamHost)));
        break;
    }
    default: {
        QString method;
        QJsonObject header;
        QJsonObject params;

        header.insert(QStringLiteral("client"), m_htmlClient);
        header.insert(QStringLiteral("clientRevision"), m_clientRevision);
        header.insert(QStringLiteral("session"), m_sessionId);
        header.insert(QStringLiteral("privacy"), m_privacy);
        header.insert(QStringLiteral("uuid"), m_uuid);
        header.insert(QStringLiteral("country"), m_country);

        switch(m_requestType) {
        case GroovesharkRequestType::AlbumGetSongs:
            method = QStringLiteral("albumGetAllSongs");
            params.insert(QStringLiteral("offset"), QJsonValue(0));
            params.insert(QStringLiteral("albumID"), QJsonValue(id()));
            params.insert(QStringLiteral("isVerified"), QJsonValue(m_requestData.canConvert<GroovesharkGetSongsRequestData>() ? m_requestData.value<GroovesharkGetSongsRequestData>().verified : false));
            break;
        case GroovesharkRequestType::PlaylistGetSongs:
            method = QStringLiteral("playlistGetSongs");
            params.insert(QStringLiteral("playlistID"), QJsonValue(id()));
            break;
        case GroovesharkRequestType::ArtistGetSongs:
            method = QStringLiteral("artistGetArtistSongs");
            params.insert(QStringLiteral("artistID"), QJsonValue(id()));
        case GroovesharkRequestType::SearchForAlbum:
        case GroovesharkRequestType::SearchForPlaylist:
            method = QStringLiteral("getResultsFromSearch");
            params.insert(QStringLiteral("guts"), QJsonValue(QStringLiteral("0")));
            params.insert(QStringLiteral("query"), QJsonValue(m_requestData.toString()));
            params.insert(QStringLiteral("ppOverride"), QStringLiteral("false"));
        { QJsonArray types;
            switch(m_requestType) {
            case GroovesharkRequestType::SearchForAlbum:
                types.append(QJsonValue(QStringLiteral("Albums")));
                break;
            case GroovesharkRequestType::SearchForPlaylist:
                types.append(QJsonValue(QStringLiteral("Playlists")));
                break;
            case GroovesharkRequestType::SearchForArtists:
                types.append(QJsonValue(QStringLiteral("Artists")));
                break;
            default:
                ;
            }
            params.insert(QStringLiteral("type"), QJsonValue(types));
        }
            break;
        default:
            ;
        }
        header.insert(QStringLiteral("token"), generateTokenHash(method, 1));
        QJsonObject mainObj;
        mainObj.insert(QStringLiteral("method"), QJsonValue(method));
        mainObj.insert(QStringLiteral("header"), header);
        mainObj.insert(QStringLiteral("parameters"), params);
        QJsonDocument jsonDoc(mainObj);
        QByteArray postData = jsonDoc.toJson();
        // create and setup download
        setPostData(postData);
        setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        setHeader("Refer", m_referer);
        setHeader("Accept", m_accept);
        //setHeader("Connection", "keep-alive");
        addDownloadUrl(tr("Grooveshark json request"), QUrl(QStringLiteral("http://grooveshark.com/more.php?") + method));
        break;
    }
    }
}

/*!
 * \brief Returns the request type.
 */
GroovesharkRequestType GroovesharkDownload::requestType() const
{
    return m_requestType;
}

QString GroovesharkDownload::suitableFilename() const
{
    auto filename = Download::suitableFilename();
    if(!filename.endsWith(QLatin1String(".mp3"))) {
        filename.append(QStringLiteral(".mp3"));
    }
    return filename;
}

QString GroovesharkDownload::typeName() const
{
    return tr("Grooveshark");
}

}
