#ifndef GROOVESHARKALBUM_H
#define GROOVESHARKALBUM_H

#include "./downloadfinder.h"

#include <QStringList>

namespace Network {

/*!
 * \brief Specifies the role of the search term.
 */
enum class GroovesharkSearchTermRole {
    AlbumId, /**< album ID */
    AlbumName, /**< album name */
    PlaylistId, /**< playlist ID */
    PlaylistName /**< playlist name */
};

class GroovesharkSearcher : public DownloadFinder {
    Q_OBJECT

public:
    explicit GroovesharkSearcher(const QString &searchTerm, GroovesharkSearchTermRole searchTermRole, bool verified, QObject *parent = nullptr);

    const QString &searchTerm() const;
    GroovesharkSearchTermRole searchTermRole() const;
    bool verified() const;

protected:
    Download *createRequest(QString &reasonForFail);

protected slots:
    ParsingResult parseResults(const QByteArray &data, QString &reasonForFail);

private:
    QString m_searchTerm;
    GroovesharkSearchTermRole m_searchType;
    QStringList m_ids;
    int m_currentId;
    bool m_verified;
};

/*!
 * \brief Returns the search term.
 */
inline const QString &GroovesharkSearcher::searchTerm() const
{
    return m_searchTerm;
}

/*!
 * \brief Returns the role of the search term.
 */
inline GroovesharkSearchTermRole GroovesharkSearcher::searchTermRole() const
{
    return m_searchType;
}

/*!
 * \brief Returns an indication whether the search is limited to verified songs.
 */
inline bool GroovesharkSearcher::verified() const
{
    return m_verified;
}
} // namespace Network

#endif // GROOVESHARKALBUM_H
