#ifndef PROGRESSBARITEMDELEGATE_H
#define PROGRESSBARITEMDELEGATE_H

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

#endif // PROGRESSBARITEMDELEGATE_H
