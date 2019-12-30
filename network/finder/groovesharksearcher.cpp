#include "./groovesharksearcher.h"

#include "../groovesharkdownload.h"

#include <c++utilities/conversion/binaryconversion.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

using namespace CppUtilities;

namespace Network {

/*!
 * \class GroovesharkSearcher
 * \brief The GroovesharkSearcher class searches for albums or playlists on Grooveshark.
 */

/*!
 * \brief Constructs a new grooveshark searcher with the specified \a searchTerm and \a searchType.
 * \param searchTerm Specifies the search term.
 * \param searchTermRole Specifies the role of the search term.
 * \param verified Indicates whether the search should be limited to verified songs.
 * \param parent Specifies the parent object.
 */
GroovesharkSearcher::GroovesharkSearcher(const QString &searchTerm, GroovesharkSearchTermRole searchTermRole, bool verified, QObject *parent)
    : DownloadFinder(parent)
    , m_searchTerm(searchTerm)
    , m_searchType(searchTermRole)
    , m_currentId(0)
    , m_verified(verified)
{
    switch (searchTermRole) {
    case GroovesharkSearchTermRole::AlbumId:
    case GroovesharkSearchTermRole::PlaylistId:
        m_ids << searchTerm;
        break;
    case GroovesharkSearchTermRole::AlbumName:
        reportCollectionTitle(searchTerm);
        break;
    case GroovesharkSearchTermRole::PlaylistName:
        reportCollectionTitle(searchTerm);
        break;
    default:;
    }
}

Download *GroovesharkSearcher::createRequest(QString &reasonForFail)
{
    Download *res = nullptr;
    switch (m_searchType) {
    case GroovesharkSearchTermRole::AlbumId:
    case GroovesharkSearchTermRole::AlbumName:
        if (!m_ids.isEmpty()) {
            if (m_currentId >= m_ids.size()) {
                reasonForFail = tr("Given id index is out of range.");
            } else {
                GroovesharkGetSongsRequestData requestData;
                requestData.id = m_ids.at(m_currentId);
                requestData.verified = m_verified;
                res = new GroovesharkDownload(GroovesharkRequestType::AlbumGetSongs, QVariant::fromValue(requestData));
            }
        } else if (!m_searchTerm.isEmpty()) {
            res = new GroovesharkDownload(GroovesharkRequestType::SearchForAlbum, QVariant::fromValue(m_searchTerm));
        }
        break;
    case GroovesharkSearchTermRole::PlaylistId:
    case GroovesharkSearchTermRole::PlaylistName:
        if (!m_ids.isEmpty()) {
            if (m_currentId >= m_ids.size()) {
                reasonForFail = tr("Given id index is out of range.");
            } else {
                res = new GroovesharkDownload(GroovesharkRequestType::PlaylistGetSongs, QVariant::fromValue(m_ids.at(m_currentId)));
            }
        } else if (!m_searchTerm.isEmpty()) {
            res = new GroovesharkDownload(GroovesharkRequestType::SearchForPlaylist, QVariant::fromValue(m_searchTerm));
        }
        break;
    default:
        reasonForFail = tr("Given search type isn't supported.");
    }
    return res;
}

DownloadFinder::ParsingResult GroovesharkSearcher::parseResults(const QByteArray &data, QString &reasonForFail)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (!jsonDoc.isObject()) {
        reasonForFail = tr("The response of Grooveshark is no valid Json object (%1).").arg(error.errorString());
        return (++m_currentId) < m_ids.size() ? ParsingResult::AnotherRequestRequired : ParsingResult::Error;
    }
    QJsonObject mainObj = jsonDoc.object();
    QJsonValue resVal = mainObj.value(QStringLiteral("result"));
    if (!resVal.isObject() && !resVal.isArray()) {
        reasonForFail = tr("No results found (no results array/object found).");
        return (++m_currentId) < m_ids.size() ? ParsingResult::AnotherRequestRequired : ParsingResult::Error;
    }
    if (m_ids.isEmpty()) { // read albums/playlists
        if (resVal.isObject()) {
            QJsonValue subResVal = resVal.toObject().value("result");
            if (subResVal.isObject()) {
                resVal = subResVal;
            }
        }
        QJsonValue collectionsVal;
        QString collectionObjName;
        QString idValName;
        switch (m_searchType) {
        case GroovesharkSearchTermRole::AlbumId:
        case GroovesharkSearchTermRole::AlbumName:
            collectionObjName = QStringLiteral("Albums");
            idValName = QStringLiteral("AlbumID");
            break;
        case GroovesharkSearchTermRole::PlaylistId:
        case GroovesharkSearchTermRole::PlaylistName:
            collectionObjName = QStringLiteral("Playlists");
            idValName = QStringLiteral("PlaylistID");
            break;
        default:
            reasonForFail = tr("The response can't be parsed because the given search type isn't supported.");
            return ParsingResult::Error;
        }
        collectionsVal = resVal.isObject() ? resVal.toObject().value(collectionObjName) : resVal.toArray();
        if (!collectionsVal.isArray()) {
            reasonForFail = tr("No collections found (relevant json value is no array).");
            return ParsingResult::Error;
        }
        QJsonArray collectionArray = collectionsVal.toArray();
        QJsonObject itemObj;
        QJsonValue idVal;
        QJsonValue verifiedVal;
        for (const QJsonValue &itemVal : collectionArray) {
            if (itemVal.isObject()) {
                itemObj = itemVal.toObject();
                if (m_verified) { // skip unverified items if verified option used
                    verifiedVal = itemObj.value(QStringLiteral("IsVerified"));
                    if (verifiedVal.isString() && verifiedVal.toString() == "0")
                        continue;
                }
                idVal = itemObj.value(idValName);
                if (idVal.isString()) {
                    m_ids << idVal.toString();
                }
            }
        }
        m_currentId = 0;
        if (!m_ids.isEmpty()) {
            return ParsingResult::AnotherRequestRequired;
        }
        reasonForFail = tr("No collections found (relevant json array contains no parsable items).");
        return ParsingResult::Error;
    } else { // read songs
        QJsonValue songsVal;
        if (resVal.isObject()) {
            switch (m_searchType) {
            case GroovesharkSearchTermRole::AlbumId:
            case GroovesharkSearchTermRole::AlbumName:
                songsVal = resVal.toObject().value(QStringLiteral("songs"));
                break;
            case GroovesharkSearchTermRole::PlaylistId:
            case GroovesharkSearchTermRole::PlaylistName:
                songsVal = resVal.toObject().value(QStringLiteral("Songs"));
                break;
            default:
                reasonForFail = tr("The response can't be parsed because the given search type isn't supported.");
                return (++m_currentId) < m_ids.size() ? ParsingResult::AnotherRequestRequired : ParsingResult::Error;
            }
        } else {
            songsVal = resVal;
        }
        if (!songsVal.isArray()) {
            reasonForFail = tr("No songs found (no songs object found).");
            return (++m_currentId) < m_ids.size() ? ParsingResult::AnotherRequestRequired : ParsingResult::Error;
        }
        GroovesharkDownload *res = nullptr;
        QJsonArray songsArray = songsVal.toArray();
        QJsonObject songObj;
        QJsonValue albumNameVal;
        QJsonValue artistNameVal;
        QJsonValue idVal;
        QJsonValue titleVal;
        QJsonValue durationVal;
        QJsonValue trackNumVal;
        for (const QJsonValue songVal : songsArray) {
            if (songVal.isObject()) {
                songObj = songVal.toObject();
                idVal = songObj.value(QStringLiteral("SongID"));
                if (idVal.isString()) {
                    titleVal = songObj.value(QStringLiteral("Name"));
                    if (titleVal.isString()) {
                        durationVal = songObj.value(QStringLiteral("EstimateDuration"));
                        trackNumVal = songObj.value(QStringLiteral("TrackNum"));
                        albumNameVal = songObj.value(QStringLiteral("AlbumName"));
                        artistNameVal = songObj.value(QStringLiteral("ArtistName"));
                        res = new GroovesharkDownload(idVal.toString());
                        res->provideMetaData(titleVal.toString(), artistNameVal.toString(),
                            durationVal.isString() ? TimeSpan::fromString(durationVal.toString().toStdString()) : TimeSpan(), albumNameVal.toString(),
                            trackNumVal.isString() ? trackNumVal.toString().toInt() : 0);
                        reportResult(res);
                    }
                    if ((collectionTitle().isEmpty() || collectionTitle().compare(tr("Unknown Album"), Qt::CaseInsensitive) == 0)
                        && albumNameVal.isString()) {
                        reportCollectionTitle(albumNameVal.toString());
                    }
                }
            }
        }
        if ((++m_currentId) < m_ids.size()) {
            return ParsingResult::AnotherRequestRequired;
        } else {
            if (!results().isEmpty()) {
                return ParsingResult::Success;
            }
        }
        reasonForFail = tr("No songs found (relevant json array contains no parsable items).");
        return ParsingResult::Error;
    }
}
} // namespace Network
