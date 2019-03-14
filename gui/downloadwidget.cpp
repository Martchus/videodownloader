#include "./downloadwidget.h"

#include "ui_downloadwidget.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOption>

namespace QtGui {

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::DownloadWidget)
{
    m_ui->setupUi(this);
}

DownloadWidget::~DownloadWidget()
{
}

void DownloadWidget::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    QStyleOptionFocusRect option;
    option.initFrom(this);
    option.backgroundColor = palette().color(QPalette::Highlight);
    painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
}
}
