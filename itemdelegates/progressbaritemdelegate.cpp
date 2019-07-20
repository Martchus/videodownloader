#include "./progressbaritemdelegate.h"

#include "../network/download.h"

#include "../model/downloadmodel.h"

#include <QApplication>

namespace QtGui {

ProgressBarItemDelegate::ProgressBarItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ProgressBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // set up a QStyleOptionProgressBar to precisely mimic the
    // environment of a progress bar.
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = option.rect;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;
    progressBarOption.progress = index.model()->data(index, DownloadModel::ProgressPercentageRole).toInt();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.text = index.model()->data(index, Qt::DisplayRole).toString();
    // draw the progress bar onto the view.
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
} // namespace QtGui
