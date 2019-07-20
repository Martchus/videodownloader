#include "./downloadmodel.h"

#include "../network/download.h"

#include <c++utilities/conversion/stringconversion.h>

using namespace CppUtilities;
using namespace Network;

namespace QtGui {

/*!
 * \class DownloadModel
 * \brief The DownloadModel class provides a model interface for a list of downloads.
 */

/*!
 * \brief Constructs a new download model.
 */
DownloadModel::DownloadModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (Download *download = this->download(index)) {
            switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                case initialUrlColumn():
                    if (download->initialUrl().isEmpty()) {
                        // show ID if no URL is available
                        return infoString(download->id());
                    } else {
                        return download->initialUrl();
                    }
                case downloadUrlColumn():
                    return infoString(download->downloadUrl().toString());
                case titleColumn():
                    return infoString(download->title());
                case uploaderColumn():
                    return infoString(download->uploader());
                case optionsColumn():
                    return infoString(download->chosenOptionName());
                case typeColumn():
                    return download->typeName();
                case statusColumn():
                    return statusString(download);
                case progressColumn():
                    return progressString(download);
                }
                break;
            case Qt::ToolTipRole:
                switch (index.column()) {
                case statusColumn():
                    return download->statusInfo();
                default:;
                }
                break;
            case DownloadModel::ProgressPercentageRole:
                return download->progressPercentage();
            case DownloadModel::AvailableOptionsRole: {
                QStringList optionNames;
                for (const OptionData &optionData : download->options()) {
                    optionNames << optionData.name();
                }
                return optionNames;
            }
            case DownloadModel::ChosenOptionRole:
                return QVariant::fromValue(download->chosenOption());
            default:;
            }
        }
    }
    return QVariant();
}

bool DownloadModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        switch (role) {
        case DownloadModel::ChosenOptionRole: {
            bool ok;
            int chosenOption = value.toInt(&ok);
            if (ok) {
                if (Download *download = this->download(index)) {
                    if (download->setChosenOption(chosenOption)) {
                        QModelIndex downloadUrlIndex = this->index(index.row(), downloadUrlColumn());
                        QModelIndex optionsIndex = this->index(index.row(), optionsColumn());
                        emit dataChanged(downloadUrlIndex, downloadUrlIndex, QVector<int>() << Qt::DisplayRole);
                        emit dataChanged(optionsIndex, optionsIndex, QVector<int>() << Qt::DisplayRole << DownloadModel::ChosenOptionRole);
                        return true;
                    }
                }
            }
        } break;
        default:;
        }
    }
    return false;
}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case initialUrlColumn():
                return tr("Initial URL/ID");
            case downloadUrlColumn():
                return tr("Download URL");
            case titleColumn():
                return tr("Title");
            case uploaderColumn():
                return tr("Uploader/creator");
            case optionsColumn():
                return tr("Options");
            case typeColumn():
                return tr("Type");
            case statusColumn():
                return tr("Status");
            case progressColumn():
                return tr("Progress");
            default:;
            }
            break;
        default:;
        }
        break;
    default:;
    }
    return QVariant();
}

Qt::ItemFlags DownloadModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        switch (index.column()) {
        case optionsColumn():
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        default:;
        }
    }
    return QAbstractItemModel::flags(index);
}

QModelIndex DownloadModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, m_downloads.at(row)) : QModelIndex();
}

QModelIndex DownloadModel::index(Download *download, int column)
{
    int row = m_downloads.indexOf(download);
    return row >= 0 ? index(row, column) : QModelIndex();
}

QModelIndex DownloadModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int DownloadModel::rowCount(const QModelIndex &) const
{
    return m_downloads.size();
}

bool DownloadModel::hasChildren(const QModelIndex &index) const
{
    return !index.isValid();
}

int DownloadModel::columnCount(const QModelIndex &) const
{
    return lastColumn() + 1;
}

/*!
 * \brief Adds the specified \a download to the model.
 *
 * Does nothing if the download is already added.
 */
void DownloadModel::addDownload(Download *download)
{
    if (!m_downloads.contains(download)) {
        int index = m_downloads.size();
        beginInsertRows(QModelIndex(), index, index);
        connect(download, &Download::statusChanged, this, &DownloadModel::downloadChangedStatus);
        connect(download, &Download::progressChanged, this, &DownloadModel::downloadProgressChanged);
        connect(download, &Download::statusInfoChanged, this, &DownloadModel::downloadInfoChanged);
        m_downloads << download;
        endInsertRows();
    }
}

/*!
 * \brief Removes the specified \a download from the model.
 *
 * Does nothing if the download is not in the model.
 */
void DownloadModel::removeDownload(Download *download)
{
    int index = m_downloads.indexOf(download);
    while (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        download->disconnect(this);
        m_downloads.removeAt(index);
        endRemoveRows();
        index = m_downloads.indexOf(download); // should be -1
    }
}

/*!
 * \brief Returns the download for the specified \a index.
 * \remarks Returns nullptr if the index is invalid.
 */
Download *DownloadModel::download(const QModelIndex &index) const
{
    if (index.isValid()) {
        return static_cast<Download *>(index.internalPointer());
    }
    return nullptr;
}

/*!
 * \brief Returns the download for the specified \a row.
 * \remarks Returns nullptr if the row is invalid.
 */
Download *DownloadModel::download(int row) const
{
    if (row < rowCount()) {
        return m_downloads.at(row);
    }
    return nullptr;
}

/*!
 * \brief Handles a download status change.
 */
void DownloadModel::downloadChangedStatus(Download *download)
{
    QModelIndex topLeft = index(download, 0);
    QModelIndex bottomRight = index(download, lastColumn());
    if (topLeft.isValid() && bottomRight.isValid()) {
        emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
    }
}

/*!
 * \brief Handles a download progress change.
 */
void DownloadModel::downloadProgressChanged(Download *download)
{
    // emit the data changed signal
    QModelIndex topLeft = index(download, statusColumn());
    QModelIndex bottomRight = index(download, lastColumn());
    if (topLeft.isValid() && bottomRight.isValid()) {
        emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
    }
}

/*!
 * \brief Handles a status description change.
 */
void DownloadModel::downloadInfoChanged(Download *download)
{
    QModelIndex topLeft = index(download, statusColumn());
    if (topLeft.isValid()) {
        emit dataChanged(topLeft, topLeft, QVector<int>() << Qt::ToolTipRole);
    }
}

/*!
 * \brief Returns the \a infostring or a placeholder if \a infostring is empty.
 */
const QString &DownloadModel::infoString(const QString &infostring)
{
    static const QString emptyField(QStringLiteral("-"));
    return infostring.isEmpty() ? emptyField : infostring;
}

/*!
 * \brief Returns a status string for the specified \a download.
 */
QString DownloadModel::statusString(Download *download)
{
    switch (download->status()) {
    case DownloadStatus::None:
        return tr("just added");
    case DownloadStatus::Initiating:
        return tr("retrieving initial information");
    case DownloadStatus::Ready:
        return tr("ready to start");
    case DownloadStatus::Waiting:
        return tr("waiting");
    case DownloadStatus::Downloading:
        return tr("downloading, %1").arg(QString::fromStdString(bitrateToString(download->speed(), true)));
    case DownloadStatus::FinishOuputFile:
        return tr("download ended, still writing to output device");
    case DownloadStatus::Interrupting:
        return tr("being interrupted");
    case DownloadStatus::Interrupted:
        return tr("interrupted");
    case DownloadStatus::Aborting:
        return tr("being aborted");
    case DownloadStatus::Failed:
        return tr("failed");
    case DownloadStatus::Finished:
        return tr("finished");
    }
    return QString();
}

/*!
 * \brief Returns a progress string for the specified \a download.
 */
QString DownloadModel::progressString(Download *download)
{
    if (download->bytesReceived() > 0) {
        if (download->bytesToReceive() > download->bytesReceived()) {
            return tr("%1 of %2 received, %3 %")
                .arg(QString::fromStdString(dataSizeToString(download->bytesReceived())))
                .arg(QString::fromStdString(dataSizeToString(download->bytesToReceive())))
                .arg(download->progressPercentage());
        } else {
            return tr("%1 received").arg(QString::fromStdString(dataSizeToString(download->bytesReceived())));
        }
    }
    return QString();
}
} // namespace QtGui
