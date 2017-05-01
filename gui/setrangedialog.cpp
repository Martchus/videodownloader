#include "./setrangedialog.h"

#include "../network/downloadrange.h"

#include "ui_setrangedialog.h"

#include <qtutilities/misc/dialogutils.h>

#include <QMessageBox>

using namespace Dialogs;
using namespace Network;

namespace QtGui {

SetRangeDialog::SetRangeDialog(DownloadRange &range, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::SetRangeDialog)
    , m_range(range)
{
    m_ui->setupUi(this);
#ifdef Q_OS_WIN32
    setStyleSheet(dialogStyle());
#endif
    m_ui->fromSpinBox->setValue(range.startOffset());
    m_ui->toSpinBox->setValue(range.endOffset());
    m_ui->currentPosSpinBox->setValue(range.currentOffset());
    connect(m_ui->abortPushButton, &QPushButton::clicked, this, &SetRangeDialog::reject);
    connect(m_ui->confirmPushButton, &QPushButton::clicked, this, &SetRangeDialog::confirm);
}

SetRangeDialog::~SetRangeDialog()
{
}

void SetRangeDialog::confirm()
{
    if ((m_ui->fromSpinBox->value() < m_ui->toSpinBox->value() || m_ui->toSpinBox->value() < 0)
        && (m_ui->fromSpinBox->value() <= m_ui->currentPosSpinBox->value())
        && (m_ui->toSpinBox->value() > m_ui->currentPosSpinBox->value() || m_ui->toSpinBox->value() < 0)) {
        m_range.setStartOffset(m_ui->fromSpinBox->value());
        m_range.setEndOffset(m_ui->toSpinBox->value());
        m_range.setCurrentOffset(m_ui->currentPosSpinBox->value());
        accept();
    } else {
        QMessageBox::warning(this, this->windowTitle(), tr("Values are invalid."));
    }
}
}
