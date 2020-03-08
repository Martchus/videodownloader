#ifndef DOWNLOADFINDERRESULTSMODEL_H
#define DOWNLOADFINDERRESULTSMODEL_H

#include <QAbstractTableModel>

namespace CppUtilities {
class TimeSpan;
}

namespace Network {
class Download;
class DownloadFinder;
} // namespace Network

namespace QtGui {

class DownloadFinderResultsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DownloadFinderResultsModel(Network::DownloadFinder *finder = nullptr, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Network::DownloadFinder *finder() const;
    void setFinder(Network::DownloadFinder *finder);

    static constexpr int titleColumn();
    static constexpr int uploaderColumn();
    static constexpr int indexColumn();
    static constexpr int durationColumn();
    static constexpr int idColumn();
    static constexpr int lastColumn();

private Q_SLOTS:
    void downloadChangedStatus(Network::Download *download);

private:
    static const QString &infoString(const QString &string);
    static QString infoString(int num);
    static QString infoString(const CppUtilities::TimeSpan &timeSpan);

    Network::DownloadFinder *m_finder;
};

/*!
 * \brief Returns the finder.
 */
inline Network::DownloadFinder *DownloadFinderResultsModel::finder() const
{
    return m_finder;
}

constexpr int DownloadFinderResultsModel::titleColumn()
{
    return 0;
}

constexpr int DownloadFinderResultsModel::uploaderColumn()
{
    return 1;
}

constexpr int DownloadFinderResultsModel::indexColumn()
{
    return 2;
}

constexpr int DownloadFinderResultsModel::durationColumn()
{
    return 3;
}

constexpr int DownloadFinderResultsModel::idColumn()
{
    return 4;
}

constexpr int DownloadFinderResultsModel::lastColumn()
{
    return idColumn();
}
} // namespace QtGui

#endif // DOWNLOADFINDERRESULTSMODEL_H
