#include "./downloadinteraction.h"
#include "./settings.h"

#include "../network/download.h"
#include "../network/permissionstatus.h"

#include <qtutilities/enterpassworddialog/enterpassworddialog.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>

using namespace Network;

namespace QtGui {

DownloadInteraction::DownloadInteraction(QObject *parent) :
    QObject(parent),
    m_parentWidget(nullptr)
{}

DownloadInteraction::DownloadInteraction(QWidget *parent) :
    QObject(parent),
    m_parentWidget(parent)
{}

void DownloadInteraction::connectDownload(Download *download)
{
    connect(download, &Download::outputDeviceRequired, this, static_cast<void (DownloadInteraction::*) (Download *, size_t)>(&DownloadInteraction::downloadRequiresOutputDevice));
    connect(download, &Download::overwriteingPermissionRequired, this, &DownloadInteraction::downloadRequriesOverwritePermission);
    connect(download, &Download::appendingPermissionRequired, this, &DownloadInteraction::downloadRequriesAppendingPermission);
    connect(download, &Download::redirectionPermissonRequired, this, &DownloadInteraction::downloadRequiresRedirectionPermission);
    connect(download, &Download::authenticationRequired, this, &DownloadInteraction::downloadRequiresAuthentication);
    connect(download, &Download::sslErrors, this, &DownloadInteraction::downloadHasSslErrors);
}

void DownloadInteraction::disconnectDownload(Download *download)
{
    download->disconnect(this);
}

void DownloadInteraction::downloadRequiresOutputDevice(Download *download, size_t optionIndex)
{
    downloadRequiresOutputDevice(download, optionIndex, false);
}

void DownloadInteraction::downloadRequiresOutputDevice(Download *download, size_t optionIndex, bool forceFileDialog)
{
    QString fileName = download->suitableFilename();
    // use default directory and the "suitable file name" to determine the target path
    if(GeneralTargetOptionPage::determineTargetFileWithoutAsking() // if correspondent option is set
            && !forceFileDialog // and the caller don't wants to force the file dialog
            && (GeneralTargetOptionPage::targetDirectory().isEmpty() || QDir(GeneralTargetOptionPage::targetDirectory()).exists()) // and the default directory exists or is empty
            && !fileName.isEmpty()) { // and the file name is not empty
        download->provideOutputDevice(optionIndex, new QFile(GeneralTargetOptionPage::targetDirectory() + "/" + fileName), true);
    } else { // aks the user for the target path otherwise
        QFileDialog *dlg = new QFileDialog(m_parentWidget);
        dlg->setModal(false);
        dlg->setFileMode(QFileDialog::AnyFile);
        dlg->setAcceptMode(QFileDialog::AcceptSave);
        dlg->setDirectory(GeneralTargetOptionPage::targetDirectory());
        dlg->selectFile(fileName);
        dlg->setOption(QFileDialog::DontConfirmOverwrite, true);
        if(!download->title().isEmpty()) {
            dlg->setWindowTitle(tr("Where to save »%1«?").arg(download->title()));
        }
        connect(dlg, &QFileDialog::finished, [download, optionIndex, dlg] (int result) {
            if(result == QFileDialog::Accepted && dlg->selectedFiles().size() == 1) {
                download->provideOutputDevice(optionIndex, new QFile(dlg->selectedFiles().at(0)), true);
            } else {
                download->provideOutputDevice(optionIndex, nullptr);
            }
            dlg->deleteLater();
        });
        dlg->show();
    }
}

void DownloadInteraction::downloadRequriesOverwritePermission(Download *download, size_t optionIndex, const QString &file)
{
    if(GeneralTargetOptionPage::overwriteWithoutAsking()) {
        download->setOverwritePermission(optionIndex, PermissionStatus::Allowed);
    } else {
        QString message = tr("<p>The output file <i>%1</i> already exists.</p><p>Do you want to overwrite the existing file?</p>").arg(file);
        QMessageBox *dlg = new QMessageBox(m_parentWidget);
        dlg->setModal(false);
        dlg->setTextFormat(Qt::RichText);
        dlg->setText(message);
        dlg->setIcon(QMessageBox::Warning);
        QPushButton *overwriteButton = dlg->addButton(tr("Overwrite (only this time)"), QMessageBox::YesRole);
        QPushButton *overwriteAlwaysButton = dlg->addButton(tr("Overwrite (always)"), QMessageBox::YesRole);
        QPushButton *selectOtherButton = dlg->addButton(tr("Select other download target"), QMessageBox::ActionRole);
        QPushButton *abortButton = dlg->addButton(tr("Abort"), QMessageBox::NoRole);
        dlg->setEscapeButton(abortButton);
        // show warning message
        connect(dlg, &QMessageBox::finished, [download, optionIndex, dlg, overwriteButton, overwriteAlwaysButton, selectOtherButton, abortButton, this] (int) {
            if(dlg->clickedButton() == overwriteAlwaysButton) {
                // set dontAskBeforeOverwriting to true if the user clicked yes to all
                GeneralTargetOptionPage::overwriteWithoutAsking() = true;
            }
            if(dlg->clickedButton() == overwriteButton || dlg->clickedButton() == overwriteAlwaysButton) {
                download->setOverwritePermission(optionIndex, PermissionStatus::Allowed);
            } else if(dlg->clickedButton() == selectOtherButton) {
                downloadRequiresOutputDevice(download, optionIndex, true);
            } else {
                download->setOverwritePermission(optionIndex, PermissionStatus::Refused);
            }
            dlg->deleteLater();
        });
        dlg->show();
    }
}

void DownloadInteraction::downloadRequriesAppendingPermission(Download *download, size_t optionIndex, const QString &file, quint64 offset, quint64 fileSize)
{
    if(GeneralTargetOptionPage::overwriteWithoutAsking()) {
        download->setAppendPermission(optionIndex, PermissionStatus::Allowed);
    } else {
        QString message = tr("<p>The output file <i>%1</i> already exists. The downloader assumes it contains previously downloaded data.</p><p>Do you want to <b>append</b> the received data to the existing file?</p>").arg(file);
        if(offset != fileSize) {
            message.append(tr("<p><b>The current download offset (%1) does not match the size of the existing file (%2).</b></p>").arg(offset).arg(fileSize));
        }
        QMessageBox *dlg = new QMessageBox(m_parentWidget);
        dlg->setModal(false);
        dlg->setTextFormat(Qt::RichText);
        dlg->setText(message);
        dlg->setIcon(QMessageBox::Warning);
        QPushButton *appendButton = dlg->addButton(tr("Append"), QMessageBox::YesRole);
        QPushButton *abortButton = dlg->addButton(tr("Abort"), QMessageBox::NoRole);
        dlg->setEscapeButton(abortButton);
        // show warning message
        connect(dlg, &QMessageBox::finished, [download, optionIndex, dlg, appendButton, abortButton, this] (int) {
            if(dlg->clickedButton() == appendButton) {
                download->setAppendPermission(optionIndex, PermissionStatus::Allowed);
            } else {
                download->setAppendPermission(optionIndex, PermissionStatus::Refused);
            }
            dlg->deleteLater();
        });
        dlg->show();
    }
}

void DownloadInteraction::downloadRequiresRedirectionPermission(Download *download, size_t optionIndex)
{
    if(NetworkMiscOptionPage::redirectWithoutAsking()) {
        download->setRedirectPermission(optionIndex, PermissionStatus::Allowed);
    } else {
        const QUrl &originalUrl = download->downloadUrl(download->options().at(optionIndex).redirectionOf());
        const QUrl &newUrl  = download->downloadUrl(optionIndex);
        QString message = tr("<p>Do you wnat to redirect form <i>%1</i> to <i>%2</i>?</p><p>The redirection URL will be added to the options so you can it select later, too.</p>").arg(
                    originalUrl.toString(),
                    newUrl.toString());
        QMessageBox *dlg = new QMessageBox(m_parentWidget);
        dlg->setModal(false);
        dlg->setTextFormat(Qt::RichText);
        dlg->setText(message);
        dlg->setIcon(QMessageBox::Question);
        dlg->setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No);
        connect(dlg, &QMessageBox::finished, [download, optionIndex, dlg, this] (int result) {
            switch(result) {
            case QMessageBox::YesToAll:
                NetworkMiscOptionPage::redirectWithoutAsking() = true;
                download->setRedirectPermission(optionIndex, PermissionStatus::AlwaysAllowed);
                break;
            case QMessageBox::Yes:
                download->setRedirectPermission(optionIndex, PermissionStatus::Allowed);
                break;
            default:
                download->setRedirectPermission(optionIndex, PermissionStatus::Refused);
            }
            dlg->deleteLater();
        });
        connect(download, &Download::destroyed, dlg, &QMessageBox::deleteLater);
        dlg->show();
    }
}

void DownloadInteraction::downloadRequiresAuthentication(Download *download, size_t optionIndex, const QString &realm)
{
    Dialogs::EnterPasswordDialog *dlg = new Dialogs::EnterPasswordDialog(m_parentWidget);
    QString downloadName = download->downloadUrl().isEmpty() ? download->id() : download->downloadUrl().toString();
    dlg->setModal(false);
    dlg->setPromptForUserName(true);
    dlg->setPasswordRequired(true);
    dlg->setInstruction(tr("<p>Enter authentication credentials for the download <i>%1</i>.</p>").arg(downloadName));
    if(!realm.isEmpty()) {
        dlg->setDescription(tr("Realm: %1").arg(realm));
    }
    connect(dlg, &Dialogs::EnterPasswordDialog::accepted, [download, optionIndex, dlg] {
        download->provideAuthenticationCredentials(optionIndex, AuthenticationCredentials(dlg->userName(), dlg->password()));
        dlg->deleteLater();
    });
    connect(download, &Download::destroyed, dlg, &QMessageBox::deleteLater);
    dlg->adjustSize();
    dlg->show();
}

void DownloadInteraction::downloadHasSslErrors(Download *download, size_t optionIndex, const QList<QSslError> &sslErrors)
{
    QString downloadName = download->downloadUrl().isEmpty() ? download->id() : download->downloadUrl().toString();
    QString message(QStringLiteral("<p style=\"font-weight: bold;\">"));
    message.append(tr("The download <i>%1</i> has SSL errors:").arg(downloadName));
    message.append(QStringLiteral("</p><ul>"));
    foreach(const QSslError &error, sslErrors) {
        message.append(QStringLiteral("<li><span style=\"font-style: italic;\">"));
        message.append(error.errorString());
        message.append(QStringLiteral("</span>"));
        if(!error.certificate().isNull()) {
            message.append(QStringLiteral("<br>"));
            message.append(error.certificate().toText());
        }
        message.append(QStringLiteral("</li>"));
    }
    message.append(QStringLiteral("</ul><p>"));
    message.append(tr("Do you want to ignore the SSL errors for this download?"));
    message.append(QStringLiteral("</p>"));
    QMessageBox *dlg = new QMessageBox(m_parentWidget);
    dlg->setModal(false);
    dlg->setTextFormat(Qt::RichText);
    dlg->setText(message);
    dlg->setIcon(QMessageBox::Warning);
    dlg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    connect(dlg, &QMessageBox::finished, [download, optionIndex, dlg] (int result) {
        switch(result) {
        case QMessageBox::Yes:
            download->setIgnoreSslErrorsPermission(optionIndex, PermissionStatus::Allowed);
            break;
        default:
            download->setIgnoreSslErrorsPermission(optionIndex, PermissionStatus::Refused);
        }
        dlg->deleteLater();
    });
    connect(download, &Download::destroyed, dlg, &QMessageBox::deleteLater);
    dlg->show();
}

}
