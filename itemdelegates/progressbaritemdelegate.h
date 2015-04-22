#ifndef PROGRESSBARDELEGATE_H
#define PROGRESSBARDELEGATE_H

#include <QStyledItemDelegate>

namespace QtGui {

class ProgressBarItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ProgressBarItemDelegate(QObject *parent);
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
};

}

#endif // DOWNLOADSVIEWDELEGATE_H
