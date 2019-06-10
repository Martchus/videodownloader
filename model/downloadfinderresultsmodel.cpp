#include "./downloadfinderresultsmodel.h"

#include "../network/download.h"
#include "../network/finder/downloadfinder.h"

#include <functional>

using namespace CppUtilities;
using namespace Network;

namespace QtGui {

/*!
 * \class DownloadFinderResultsModel
 * \brief The DownloadFinderResultsModel class provides a model interface the results of a DownloadFinder.
 */

/*!
 * \brief Constructs a new download finder model.
 */
DownloadFinderResultsModel::DownloadFinderResultsModel(DownloadFinder *finder, QObject *parent)
    : QAbstractTableModel(parent)
    , m_finder(nullptr)
{
    setFinder(finder);
}

/*!
 * \brief Sets the finder. Causes a model reset.
 */
void DownloadFinderResultsModel::setFinder(DownloadFinder *finder)
{
    if (finder != m_finder) {
        beginResetModel();
        if ((m_finder = finder)) {
            // results are about to be cleared
            connect(m_finder, &DownloadFinder::aboutToClearResults, std::bind(&DownloadFinderResultsModel::beginResetModel, this));
            // results have been cleared
            connect(m_finder, &DownloadFinder::clearResults, std::bind(&DownloadFinderResultsModel::endResetModel, this));
            // new results are about to be available
            connect(m_finder, &DownloadFinder::aboutToMakeNewResultsAvailable, [this](unsigned int count) {
                if (count > 0) {
                    beginInsertRows(QModelIndex(), rowCount(), count - 1);
                }
            });
            // new results are available
            connect(m_finder, &DownloadFinder::newResultsAvailable, [this](const QList<Download *> &results) {
                foreach (Download *download, results) {
                    connect(download, &Download::statusChanged, this, &DownloadFinderResultsModel::downloadChangedStatus);
                    // initiate the download if not done yet and instant initiation is recommendable
                    if (!download->isInitiated() && download->isInitiatingInstantlyRecommendable()) {
                        download->init();
                    }
                }
                endInsertRows();
            });
        }
        endResetModel();
    }
}

QVariant DownloadFinderResultsModel::data(const QModelIndex &index, int role) const
{
    if (m_finder && index.isValid() && index.row() >= 0 && index.row() < m_finder->results().size()) {
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case titleColumn():
                return infoString(m_finder->results().at(index.row())->title());
            case uploaderColumn():
                return infoString(m_finder->results().at(index.row())->uploader());
            case indexColumn():
                return infoString(m_finder->results().at(index.row())->positionInCollection());
            case durationColumn():
                return infoString(m_finder->results().at(index.row())->duration());
            case idColumn():
                if (m_finder->results().at(index.row())->id().isEmpty()) {
                    return infoString(m_finder->results().at(index.row())->initialUrl().toString());
                }
                return infoString(m_finder->results().at(index.row())->id());
            default:;
            }
            break;
        default:;
        }
    }
    return QVariant();
}

QVariant DownloadFinderResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (orientation) {
    case Qt::Horizontal:
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case titleColumn():
                return tr("Title");
            case uploaderColumn():
                return tr("Uploader/creator");
            case indexColumn():
                return tr("#");
            case durationColumn():
                return tr("Duration");
            case idColumn():
                return tr("ID/URL");
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

int DownloadFinderResultsModel::rowCount(const QModelIndex &parent) const
{
    if (m_finder && !parent.isValid()) {
        return m_finder->resultCount();
    }
    return 0;
}

int DownloadFinderResultsModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return lastColumn() + 1;
    }
    return 0;
}

bool DownloadFinderResultsModel::canFetchMore(const QModelIndex &) const
{
    return false;
}

/*!
 * \brief Returns the placeholder for empty fields.
 */
const QString &emptyFieldPlaceholder()
{
    static const QString placeholder(QStringLiteral("-"));
    return placeholder;
}

/*!
 * \brief Returns the \a infostring or a placeholder if \a infostring is empty.
 */
const QString &DownloadFinderResultsModel::infoString(const QString &string)
{
    return string.isEmpty() ? emptyFieldPlaceholder() : string;
}

/*!
 * \brief Returns \a num as string or a placeholder if \a num is negative or zero.
 */
QString DownloadFinderResultsModel::infoString(int num)
{
    return num > 0 ? QString::number(num) : emptyFieldPlaceholder();
}

/*!
 * \brief Returns \a timeSpan as string or a placeholder if \a timeSpan is null.
 */
QString DownloadFinderResultsModel::infoString(const TimeSpan &timeSpan)
{
    return timeSpan.isNull() ? emptyFieldPlaceholder() : QString::fromStdString(timeSpan.toString(TimeSpanOutputFormat::WithMeasures));
}

/*!
 * \brief Handles a download status change.
 */
void DownloadFinderResultsModel::downloadChangedStatus(Download *download)
{
    if (m_finder) {
        int row = m_finder->results().indexOf(download);
        QModelIndex topLeft = index(row, 0);
        QModelIndex bottomRight = index(row, lastColumn());
        if (topLeft.isValid() && bottomRight.isValid()) {
            emit dataChanged(topLeft, bottomRight, QVector<int>() << Qt::DisplayRole);
        }
    }
}
}
