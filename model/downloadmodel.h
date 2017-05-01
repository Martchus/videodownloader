#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractItemModel>
#include <QList>

namespace Network {
class Download;
}

namespace QtGui {

class DownloadModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum ItemDataRole { ProgressPercentageRole = Qt::UserRole + 1, AvailableOptionsRole = Qt::UserRole + 2, ChosenOptionRole = Qt::UserRole + 3 };

    explicit DownloadModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(Network::Download *download, int column = 0);
    QModelIndex parent(const QModelIndex &) const;
    int rowCount(const QModelIndex & = QModelIndex()) const;
    bool hasChildren(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    void addDownload(Network::Download *download);
    void removeDownload(Network::Download *download);
    Network::Download *download(const QModelIndex &index) const;
    Network::Download *download(int row) const;

    static constexpr int initialUrlColumn();
    static constexpr int downloadUrlColumn();
    static constexpr int titleColumn();
    static constexpr int uploaderColumn();
    static constexpr int optionsColumn();
    static constexpr int typeColumn();
    static constexpr int statusColumn();
    static constexpr int progressColumn();
    static constexpr int lastColumn();

private slots:
    void downloadChangedStatus(Network::Download *download);
    void downloadProgressChanged(Network::Download *download);
    void downloadInfoChanged(Network::Download *download);

private:
    static const QString &infoString(const QString &infostring);
    static QString statusString(Network::Download *download);
    static QString progressString(Network::Download *download);

    QList<Network::Download *> m_downloads;
};

constexpr int DownloadModel::initialUrlColumn()
{
    return 0;
}

constexpr int DownloadModel::downloadUrlColumn()
{
    return 1;
}

constexpr int DownloadModel::titleColumn()
{
    return 2;
}

constexpr int DownloadModel::uploaderColumn()
{
    return 3;
}

constexpr int DownloadModel::optionsColumn()
{
    return 4;
}

constexpr int DownloadModel::typeColumn()
{
    return 5;
}

constexpr int DownloadModel::statusColumn()
{
    return 6;
}

constexpr int DownloadModel::progressColumn()
{
    return 7;
}

constexpr int DownloadModel::lastColumn()
{
    return progressColumn();
}
}

#endif // DOWNLOADMODEL_H
