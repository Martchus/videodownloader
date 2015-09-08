#include "./comboboxitemdelegate.h"

#include "../model/downloadmodel.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QApplication>
#include <QMessageBox>

namespace QtGui {

ComboBoxItemDelegate::ComboBoxItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{}

ComboBoxItemDelegate::~ComboBoxItemDelegate()
{}

bool ComboBoxItemDelegate::ComboBoxItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    switch(event->type())
    {
    case QEvent::MouseButtonPress:
        return false;
    case QEvent::GraphicsSceneWheel:
    case QEvent::Wheel:
        return false;
    default:
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
}

QWidget *ComboBoxItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    if(const DownloadModel *model = qobject_cast<const DownloadModel *>(index.model())) {
        QStringList availableOptions = model->data(index, DownloadModel::AvailableOptionsRole).toStringList();
        if(availableOptions.size() > 0) {
            QComboBox *editor = new QComboBox(parent);
            editor->addItems(availableOptions);
            editor->setCurrentIndex(model->data(index, DownloadModel::ChosenOptionRole).toInt());
            return editor;
        }
    }
    return nullptr;
}

void ComboBoxItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(const DownloadModel *model = qobject_cast<const DownloadModel *>(index.model())) {
        if(QComboBox *comboBox = qobject_cast<QComboBox *>(editor)) {
            comboBox->setCurrentIndex(model->data(index, DownloadModel::ChosenOptionRole).toInt());
        }
    }
}

void ComboBoxItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(QComboBox *comboBox = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, comboBox->currentIndex(), DownloadModel::ChosenOptionRole);
    }
}

void ComboBoxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const//(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

}
