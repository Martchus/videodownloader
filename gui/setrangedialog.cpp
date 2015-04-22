#include "setrangedialog.h"
#include "ui_setrangedialog.h"

#include "network/downloadrange.h"

#include <QMessageBox>

using namespace Network;

namespace QtGui {

SetRangeDialog::SetRangeDialog(DownloadRange &range, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SetRangeDialog),
    m_range(range)
{
    m_ui->setupUi(this);
#ifdef Q_OS_WIN32
    setStyleSheet(QStringLiteral("* { font: 9pt \"Segoe UI\"; } #mainWidget { color: black; background-color: white; border: none; } #bottomWidget { background-color: #F0F0F0; border-top: 1px solid #DFDFDF; } QMessageBox QLabel, QInputDialog QLabel { font-size: 12pt; color: #003399; }"));
#endif
    m_ui->fromSpinBox->setValue(range.startOffset());
    m_ui->toSpinBox->setValue(range.endOffset());
    m_ui->currentPosSpinBox->setValue(range.currentOffset());
    connect(m_ui->abortPushButton, &QPushButton::clicked, this, &SetRangeDialog::reject);
    connect(m_ui->confirmPushButton, &QPushButton::clicked, this, &SetRangeDialog::confirm);
}

SetRangeDialog::~SetRangeDialog()
{}

void SetRangeDialog::confirm()
{
    if((m_ui->fromSpinBox->value() < m_ui->toSpinBox->value() || m_ui->toSpinBox->value() < 0) && (m_ui->fromSpinBox->value() <= m_ui->currentPosSpinBox->value()) && (m_ui->toSpinBox->value() > m_ui->currentPosSpinBox->value() || m_ui->toSpinBox->value() < 0)) {
        m_range.setStartOffset(m_ui->fromSpinBox->value());
        m_range.setEndOffset(m_ui->toSpinBox->value());
        m_range.setCurrentOffset(m_ui->currentPosSpinBox->value());
        accept();
    } else {
        QMessageBox::warning(this, this->windowTitle(), tr("Values are invalid."));
    }
}

}
