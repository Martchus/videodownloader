#include "./adddownloaddialog.h"

#include "../network/bitsharedownload.h"
#include "../network/filenukedownload.h"
#include "../network/groovesharkdownload.h"
#include "../network/socksharedownload.h"
#include "../network/vimeodownload.h"
#include "../network/youtubedownload.h"
#ifdef UNDER_CONSTRUCTION
#include "../network/spotifydownload.h"
#endif

#include "ui_adddownloaddialog.h"

#include <qtutilities/misc/dialogutils.h>

#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

using namespace Dialogs;
using namespace Network;

namespace QtGui {

QStringList AddDownloadDialog::m_knownDownloadTypeNames = QStringList()
    << QStringLiteral("standard http(s)/ftp") << QStringLiteral("Youtube") << QStringLiteral("Vimeo") << QStringLiteral("Grooveshark")
    << QStringLiteral("Sockshare/Putlocker") << QStringLiteral("Bitshare") << QStringLiteral("FileNuke")
#ifdef UNDER_CONSTRUCTION
    << QStringLiteral("Spotify")
#endif
    ;

AddDownloadDialog::AddDownloadDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::AddDownloadDialog)
    , m_downloadTypeIndex(0)
    , m_downloadTypeIndexAdjustedManually(false)
    , m_validInput(false)
    , m_selectDownloadTypeInputDialog(nullptr)
{
    m_ui->setupUi(this);
    makeHeading(m_ui->mainInstructionLabel);
    setStyleSheet(dialogStyle());
    connect(m_ui->backPushButton, &QPushButton::clicked, this, &AddDownloadDialog::back);
    connect(m_ui->addPushButton, &QPushButton::clicked, this, &AddDownloadDialog::setLastUrl);
    connect(m_ui->addPushButton, &QPushButton::clicked, this, &AddDownloadDialog::addDownloadClicked);
    connect(m_ui->addPushButton, &QPushButton::clicked, this, &AddDownloadDialog::accept);
    connect(m_ui->urlLineEdit, &QLineEdit::textChanged, this, &AddDownloadDialog::textChanged);
    connect(m_ui->urlLineEdit, &QLineEdit::returnPressed, this, &AddDownloadDialog::returnPressed);
    connect(m_ui->adjustDetectedTypePushButton, &QPushButton::clicked, this, &AddDownloadDialog::adjustDetectedDownloadType);
    connect(m_ui->insertTextFromClipboardPushButton, &QPushButton::clicked, this, &AddDownloadDialog::insertTextFromClipboard);

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("adddownloaddialog");
    m_lastUrl = settings.value("lasturl", QString()).toString();
    m_ui->downloadTypeInfoWidget->setHidden(true);
    m_ui->urlLineEdit->setText(m_lastUrl);
    m_ui->urlLineEdit->setInputMethodHints(Qt::ImhUrlCharactersOnly);
    m_ui->urlLineEdit->selectAll();

    settings.endGroup();
}

AddDownloadDialog::~AddDownloadDialog()
{
}

Download *AddDownloadDialog::result() const
{
    if (!hasValidInput()) {
        return nullptr;
    }

    QString res(m_ui->urlLineEdit->text());
    switch (m_downloadTypeIndex) {
    case 0:
        return new HttpDownload(QUrl(res));
    case 1:
        return new YoutubeDownload(QUrl(res));
    case 2:
        return new VimeoDownload(QUrl(res));
    case 3:
        return new GroovesharkDownload(res);
    case 4:
        return new SockshareDownload(QUrl(res));
    case 5:
        return new BitshareDownload(QUrl(res));
    case 6:
        return new FileNukeDownload(QUrl(res));
#ifdef UNDER_CONSTRUCTION
    case 7:
        return new SpotifyDownload(res);
#endif
    default:
        return nullptr;
    }
}

void AddDownloadDialog::reset()
{
    m_downloadTypeIndexAdjustedManually = false;
    if (m_ui->urlLineEdit->text() != m_lastUrl) {
        m_validInput = false;
        m_ui->urlLineEdit->setText(m_lastUrl);
    }
}

bool AddDownloadDialog::hasValidInput() const
{
    return m_validInput;
}

void AddDownloadDialog::back()
{
    setLastUrl();
    close();
}

void AddDownloadDialog::returnPressed()
{
    if (hasValidInput()) {
        setLastUrl();
        emit addDownloadClicked();
        accept();
    }
}

void AddDownloadDialog::insertTextFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (text.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), tr("The clipboard does not contain any text."));
    } else {
        m_ui->urlLineEdit->setText(text);
    }
}

void AddDownloadDialog::textChanged(const QString &text)
{
    if (!text.isEmpty()) {
        // entered value might be a grooveshark song id
        bool isValidGroovesharkId = true;
        for (const auto c : text) {
            if (!c.isDigit()) {
                isValidGroovesharkId = false;
                break;
            }
        }
        if (isValidGroovesharkId) {
            m_ui->downloadTypeInfoWidget->setHidden(false);
            m_ui->addPushButton->setEnabled(true);
            m_validInput = true;

            m_downloadTypeIndex = 3;
            m_ui->downloadTypeLabel->setText(tr("The entered number will be treated as Grooveshark song id."));
            return;
        }

        // parse entered text as url
        QUrl url(text);
        QString host = url.host();
        QString scheme = url.scheme();

        if (!(host.isEmpty() || scheme.isEmpty())) {
            m_ui->downloadTypeInfoWidget->setHidden(false);

            // check if the protocol is supported
            if (scheme == QLatin1String("http") || scheme == QLatin1String("https") || scheme == QLatin1String("ftp")) {
                m_ui->addPushButton->setEnabled(true);
                m_validInput = true;
                // detect download type (if not adjusted manually)
                if (!m_downloadTypeIndexAdjustedManually) {
                    m_downloadTypeIndex = 0;
                    if (host.contains(QLatin1String("youtube"), Qt::CaseInsensitive) || host.startsWith(QLatin1String("youtu.be"))) {
                        m_downloadTypeIndex = 1;
                    } else if (host.contains(QLatin1String("vimeo"))) {
                        m_downloadTypeIndex = 2;
                    } else if (host.contains(QLatin1String("sockshare")) || host.contains(QLatin1String("putlocker"))) {
                        m_downloadTypeIndex = 5;
                    } else if (host.contains(QLatin1String("bitshare"))) {
                        m_downloadTypeIndex = 6;
                    } else if (host.contains(QLatin1String("filenuke"))) {
                        m_downloadTypeIndex = 7;
                    }
                    if (m_downloadTypeIndex) {
                        m_ui->downloadTypeLabel->setText(tr("The entered url seems to be from %1 so it will be added as %1 download.")
                                                             .arg(m_knownDownloadTypeNames.at(m_downloadTypeIndex)));
                    } else {
                        m_ui->downloadTypeLabel->setText(tr("The entered url will be added as standard %1 download.").arg(scheme));
                    }
                }
            } else {
                m_ui->addPushButton->setEnabled(false);
                m_validInput = false;
                m_ui->downloadTypeLabel->setText(tr("The entered protocol is not supported."));
                m_downloadTypeIndexAdjustedManually = false;
            }
            return;
        }
    }

    m_ui->downloadTypeInfoWidget->setHidden(true);
    m_ui->addPushButton->setEnabled(false);
    m_validInput = false;
    m_downloadTypeIndexAdjustedManually = false;
}

void AddDownloadDialog::adjustDetectedDownloadType()
{
    if (!m_selectDownloadTypeInputDialog) {
        m_selectDownloadTypeInputDialog = new QInputDialog(this);
        m_selectDownloadTypeInputDialog->setWindowTitle(tr("Select download type"));
        m_selectDownloadTypeInputDialog->setLabelText(tr("Select as which kind of download the entered url should be treated."));
        m_selectDownloadTypeInputDialog->setComboBoxItems(m_knownDownloadTypeNames);
        m_selectDownloadTypeInputDialog->setComboBoxEditable(false);
        m_selectDownloadTypeInputDialog->setOption(QInputDialog::UseListViewForComboBoxItems);
    }

    m_selectDownloadTypeInputDialog->setTextValue(m_knownDownloadTypeNames.at(m_downloadTypeIndex));
    if (m_selectDownloadTypeInputDialog->exec()) {
        m_downloadTypeIndex = m_knownDownloadTypeNames.indexOf(m_selectDownloadTypeInputDialog->textValue());
        m_ui->downloadTypeLabel->setText(
            tr("The entered url will be added as %1 download (adjusted).").arg(m_knownDownloadTypeNames.at(m_downloadTypeIndex)));
        m_downloadTypeIndexAdjustedManually = true;
    }
}

void AddDownloadDialog::setLastUrl()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.beginGroup("adddownloaddialog");
    m_lastUrl = m_ui->urlLineEdit->text();
    settings.setValue("lasturl", m_lastUrl);
    settings.endGroup();
}
}
