#include "./mainwindow.h"
#include "./downloadinteraction.h"
#include "./setrangedialog.h"
#include "./adddownloaddialog.h"
#include "./addmultipledownloadswizard.h"
#include "./settings.h"

#include "../network/download.h"
#include "../network/youtubedownload.h"
#include "../network/socksharedownload.h"
#include "../network/groovesharkdownload.h"
#include "../network/bitsharedownload.h"
#ifdef CONFIG_TESTDOWNLOAD
#include "../network/testdownload.h"
#endif

#include "../model/downloadmodel.h"

#include "../itemdelegates/progressbaritemdelegate.h"
#include "../itemdelegates/comboboxitemdelegate.h"

#include "ui_mainwindow.h"

#include <qtutilities/aboutdialog/aboutdialog.h>
#include <qtutilities/enterpassworddialog/enterpassworddialog.h>
#include <qtutilities/misc/desktoputils.h>

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/chrono/timespan.h>
#include <c++utilities/io/path.h>

#include <QDesktopServices>
#include <QCloseEvent>
#include <QClipboard>
#include <QInputDialog>
#include <QList>
#include <QDir>
#include <QLabel>
#include <QSslError>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>
#include <QNetworkProxy>
#include <QFileSystemModel>
#include <QTimer>

#include <functional>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;
using namespace Network;

namespace QtGui {

// C'tor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_trayIcon(nullptr),
    m_trayIconMenu(nullptr),
    m_internalClipboardChange(false),
    m_activeDownloads(0),
    m_downloadsToStart(0),
    m_initiatingDownloads(0),
    m_totalSpeed(0),
    m_stillToReceive(0),
    m_remainingTime(TimeSpan()),
    m_downloadInteraction(new DownloadInteraction(this)),
    m_addDownloadDlg(nullptr),
    m_addMultipleDownloadsWizard(nullptr),
    m_settingsDlg(nullptr),
    m_aboutDlg(nullptr)
{
    // setup ui
    m_ui->setupUi(this);

    // load settings
    restoreGeometry(UiPage::mainWindowGeometry());

    // setup tray icon and its context menu
    setupTrayIcon();

    // setup main toolbar
    //  add downloads tool button
    QToolButton *addDownloadToolButton = new QToolButton(m_ui->toolBar);
    addDownloadToolButton->setMenu(m_ui->menuAdd);
    addDownloadToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    addDownloadToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addDownloadToolButton->setText(tr("Add download"));
    addDownloadToolButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_ui->toolBar->insertWidget(m_ui->actionStart_selected, addDownloadToolButton);
#ifdef CONFIG_TESTDOWNLOAD
    QAction *action = new QAction(QStringLiteral("Test download"), m_ui->menuAdd);
    action->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_ui->menuAdd->addAction(action);
#endif
    //  Supervise clipboard tool button
    m_superviseClipboardToolButton = new QToolButton(m_ui->toolBar);
    m_superviseClipboardToolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_superviseClipboardToolButton->setText(tr("Supervise clipboard"));
    m_superviseClipboardToolButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
    m_superviseClipboardToolButton->setCheckable(true);
    m_ui->toolBar->insertWidget(m_ui->actionStart_selected, m_superviseClipboardToolButton);

    // setup auto spinbox
    m_autoSpinBox = new QSpinBox(m_ui->autoToolBar);
    m_autoSpinBox->setObjectName(QStringLiteral("autoSpinBox"));
    m_autoSpinBox->setEnabled(true);
    m_autoSpinBox->setMinimumSize(QSize(245, 0));
    m_autoSpinBox->setMaximum(10);
    m_autoSpinBox->setSuffix(tr(" download(s) automatically"));
    m_autoSpinBox->setPrefix(tr("start up to "));
    QWidget *spacer = new QWidget(m_ui->autoToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setVisible(true);
    m_ui->autoToolBar->addWidget(spacer);
    m_ui->autoToolBar->addWidget(m_autoSpinBox);

    // setup downloads tree view and model
    // delegates, model
    m_progressBarDelegate = new ProgressBarItemDelegate(this);
    m_comboBoxDelegate = new ComboBoxItemDelegate(this);
    m_model = new DownloadModel(this);
    m_ui->downloadsTreeView->setItemDelegateForColumn(DownloadModel::optionsColumn(), m_comboBoxDelegate);
    m_ui->downloadsTreeView->setItemDelegateForColumn(DownloadModel::progressColumn(), m_progressBarDelegate);
    m_ui->downloadsTreeView->setModel(m_model);
    updateSelectionMode();
    // context menu
    m_downloadsTreeViewContextMenu = new QMenu(this);
    m_downloadsTreeViewContextMenu->addMenu(m_ui->menuAdd);
    m_ui->menuAdd->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionStart_selected);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionResume_selected_downloads);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionRemove_selected_downloads_from_list);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionCopy_download_url);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionSet_range);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionSet_target);
    m_downloadsTreeViewContextMenu->addAction(m_ui->actionClear_target);
    m_ui->downloadsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->downloadsTreeView, &QTreeView::customContextMenuRequested, this, &MainWindow::showDownloadsTreeViewContextMenu);
    // column widths
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::initialUrlColumn(), 200);
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::downloadUrlColumn(), 200);
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::titleColumn(), 200);
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::uploaderColumn(), 100);
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::optionsColumn(), 80);
    m_ui->downloadsTreeView->setColumnWidth(DownloadModel::progressColumn(), 50);
    // update controls when selection changes
    QItemSelectionModel *selectionModel = m_ui->downloadsTreeView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::updateStartStopControls);

    // setup download status label and timer for updating it
    m_downloadStatusLabel = new QLabel(this);
    m_downloadStatusLabel->setVisible(false);
    m_ui->statusBar->addWidget(m_downloadStatusLabel);
    m_elapsedTime.start();

    // Connect signals and slots
    // Application
    connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(m_ui->actionQuit, &QAction::triggered, &QApplication::quit);
    // Add
    connect(addDownloadToolButton, &QToolButton::clicked, this, &MainWindow::showAddDownloadDialog);
    connect(m_ui->actionAdd_Download, &QAction::triggered, this, &MainWindow::showAddDownloadDialog);
    connect(m_ui->actionAdd_multiple_downloads, &QAction::triggered, this, &MainWindow::showAddMultipleDownloadsDialog);
#ifdef CONFIG_TESTDOWNLOAD
    connect(action, &QAction::triggered, [this] { this->addDownload(new TestDownload()); });
#endif
    // Downloads
    connect(m_ui->actionStart_selected, &QAction::triggered, this, &MainWindow::startOrStopSelectedDownloads);
    connect(m_ui->actionResume_selected_downloads, &QAction::triggered, this, &MainWindow::interruptOrResumeSelectedDownloads);
    connect(m_ui->actionRemove_selected_downloads_from_list, &QAction::triggered, this, &MainWindow::removeSelectedDownloads);
    connect(m_ui->actionCopy_download_url, &QAction::triggered, this, &MainWindow::copyDownloadUrl);
    connect(m_ui->actionSet_range, &QAction::triggered, this, &MainWindow::setDownloadRange);
    connect(m_ui->actionSet_target, &QAction::triggered, this, &MainWindow::setTargetPath);
    connect(m_ui->actionClear_target, &QAction::triggered, this, &MainWindow::clearTargetPath);
    connect(m_ui->actionReset_grooveshark_session, &QAction::triggered, this, &MainWindow::resetGroovesharkSession);
    // ?
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(m_ui->actionYoutube_itags, &QAction::triggered, this, &MainWindow::showYoutubeItagsInfo);
    // other
    connect(m_autoSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::checkForDownloadsToStartAutomatically);
    connect(m_ui->actionExplore_target_directory, &QAction::triggered, this, &MainWindow::exploreDownloadsDir);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::clipboardDataChanged);
}

MainWindow::~MainWindow()
{}

// Methods to show several dialogs
void MainWindow::showAboutDialog()
{
    if(!m_aboutDlg) {
        m_aboutDlg = new Dialogs::AboutDialog(this, tr("A video downloader with Qt GUI (currently only YouTube is maintained)."), QImage(QStringLiteral(":/icons/hicolor/128x128/apps/videodownloader.png")));
    }
    if(m_aboutDlg->isHidden()) {
        m_aboutDlg->showNormal();
    } else {
        m_aboutDlg->activateWindow();
    }
}

void MainWindow::addDownload(Download *download)
{
    // set parent, behavior and proxy
    download->setParent(this);
    connect(download, &Download::statusChanged, this, &MainWindow::downloadChangedStatus);
    connect(download, &Download::progressChanged, this, &MainWindow::downloadChangedProgress);
    m_downloadInteraction->connectDownload(download);
    // add download to model
    m_model->addDownload(download);
    // Begin to fetch initial info
    if(!download->isInitiated() && download->isInitiatingInstantlyRecommendable()) {
        applySettingsToDownload(download);
        download->init();
    }
}

void MainWindow::showAddDownloadDialog()
{
    if(!m_addDownloadDlg) {
        m_addDownloadDlg = new AddDownloadDialog(this);
        connect(m_addDownloadDlg, &AddDownloadDialog::addDownloadClicked, this, &MainWindow::addDownloadDialogResults);
    }
    if(m_addDownloadDlg->isHidden()) {
        m_addDownloadDlg->showNormal();
    } else {
        m_addDownloadDlg->activateWindow();
    }
}

void MainWindow::addDownloadDialogResults()
{
    if(isHidden()) {
        show();
    }
    if(m_addDownloadDlg) {
        if(Download *result = m_addDownloadDlg->result()) {
            addDownload(result);
        }
        m_addDownloadDlg->reset();
    }
}

void MainWindow::showAddMultipleDownloadsDialog()
{
    if(!m_addMultipleDownloadsWizard) {
        m_addMultipleDownloadsWizard = new AddMultipleDownloadsWizard(this);
        connect(m_addMultipleDownloadsWizard, &AddMultipleDownloadsWizard::finished, this, &MainWindow::addMultipleDownloadsWizardResults);
    }
    if(m_addMultipleDownloadsWizard->isHidden()) {
        m_addMultipleDownloadsWizard->showNormal();
    } else {
        m_addMultipleDownloadsWizard->activateWindow();
    }
}

void MainWindow::addMultipleDownloadsWizardResults()
{
    if(isHidden()) {
        show();
    }
    if(m_addMultipleDownloadsWizard) {
        foreach(Download *res, m_addMultipleDownloadsWizard->results()) {
            addDownload(res);
        }
        m_addMultipleDownloadsWizard->restart();
    }
}

void MainWindow::showSettingsDialog()
{
    if(!m_settingsDlg) {
        m_settingsDlg = new SettingsDialog(this);
        connect(m_settingsDlg, &SettingsDialog::applied, this, &MainWindow::settingsAccepted);
    }
    if(m_settingsDlg->isHidden()) {
        m_settingsDlg->showNormal();
    } else {
        m_settingsDlg->activateWindow();
    }
}

void MainWindow::settingsAccepted()
{
    updateSelectionMode();
}

void MainWindow::showTrayIconMessage()
{
    m_trayIcon->showMessage(windowTitle(), tr("The downloader will keep running in the system tray. To terminate the program, "
                          "choose \"Quit\" in the context menu of the system tray entry."));
}

void MainWindow::checkForDownloadsToStartAutomatically()
{
    for(int row = 0, rowCount = m_model->rowCount(),
        toStart = m_autoSpinBox->value() - m_activeDownloads - m_initiatingDownloads;
        row < rowCount && toStart > 0; ++row) {
        Download *download = m_model->download(row);
        switch(download->status()) {
        case DownloadStatus::None:
            applySettingsToDownload(download);
            download->init();
            --toStart;
            break;
        case DownloadStatus::Ready:
            applySettingsToDownload(download);
            download->start();
            --toStart;
            break;
        default:
            ;
        }
    }
}

// Methods to start and stop downloads
void MainWindow::startOrStopSelectedDownloads()
{
    QList<Download *> selectedDownloads = this->selectedDownloads();

    // check if there are downloads selected
    if(!selectedDownloads.count()) {
        QMessageBox::warning(this, windowTitle(), tr("There are no downloads selected."));
        return;
    }

    foreach(Download *download, selectedDownloads) {
        switch(download->status()) {
        case DownloadStatus::None:
            // retrieve initial information when that still has to be done
            applySettingsToDownload(download);
            download->init();
            break;
        case DownloadStatus::Initiating:
            // when the download is retireving initial information it can be stopped
            download->stop();
            break;
        case DownloadStatus::Downloading:
        case DownloadStatus::FinishOuputFile:
            // when the download is downloading it can be stopped
            download->stop();
            break;
        case DownloadStatus::Ready:
        case DownloadStatus::Failed:
        case DownloadStatus::Finished:
        case DownloadStatus::Interrupted:
            // the download can be (re)started, but if initial information has been retireved has to be checked
            if(download->isInitiated()) {
                applySettingsToDownload(download);
                // reset the current offset to ensure the download is (re)started from the beginning
                download->range().resetCurrentOffset();
                download->start();
            } else {
                QString downloadUrl;
                if(selectedDownloads.count() > 1)
                    downloadUrl = tr("(%1) ").arg(download->initialUrl().toString());

                switch(download->status()) {
                case DownloadStatus::Ready:
                    // download without initial information shouldn't be prepared to start!
                    QMessageBox::warning(this, windowTitle(), tr("The download %1can't be started because the initial information hasn't been retrieved yet. That shouldn't happen.").arg(downloadUrl));
                    break;
                case DownloadStatus::Failed:
                    applySettingsToDownload(download);
                    download->init(); // try to retrieve initial information again
                    //QMessageBox::warning(this, windowTitle(), QStringLiteral("The download %1can't be started because it couldn't be initialized. Readd the download to try it again. See the description below for more detail:\n\n") + status.getDescription());
                    break;
                default:
                    ;
                }
            }
            break;
        default:
            break; // nothing to do here
        }
    }

    // repeal selection to avoid double-click problem
    m_ui->downloadsTreeView->selectionModel()->clearSelection();
}

void MainWindow::interruptOrResumeSelectedDownloads()
{
    QList<Download *> selectedDownloads = this->selectedDownloads();

    // check if there are downloads selected
    if(!selectedDownloads.count()) {
        QMessageBox::warning(this, windowTitle(), tr("There are no downloads selected."));
        return;
    }

    foreach(Download *download, selectedDownloads) {
        switch(download->status()) {
        case DownloadStatus::Downloading:
            download->interrupt();
            // the range will be set automatically by the download to be able to resume the download
            break;
        case DownloadStatus::Ready:
        case DownloadStatus::Interrupted:
        case DownloadStatus::Failed:
            if(download->isInitiated()) {
                download->start();
            } else {
                QMessageBox::warning(this, windowTitle(), tr("The download %1 can't be started because it couldn't be initialized. Readd the download to try it again. See the description below for more detail:\n\n").arg((download->title().isEmpty() ? download->initialUrl().toString() : download->title())) + download->statusInfo());
            }
            break;
        default:
            // nothing to do here
            break;
        }
    }

    // repeal selection to avoid double-click problem
    m_ui->downloadsTreeView->selectionModel()->clearSelection();
}

void MainWindow::removeSelectedDownloads()
{
    int removed = 0;
    QModelIndexList selectedIndexes = m_ui->downloadsTreeView->selectionModel()->selectedRows();
    QList<QPersistentModelIndex> persistendIndexes;
    foreach(QModelIndex selectedIndex, selectedIndexes)
        persistendIndexes << QPersistentModelIndex(selectedIndex);
    foreach(QPersistentModelIndex selectedIndex, persistendIndexes) {
        if(Download *download = m_model->download(selectedIndex)) {
            switch(download->status()) {
            case DownloadStatus::Downloading:
            case DownloadStatus::Initiating:
            case DownloadStatus::FinishOuputFile:
                break;
            default:
                m_model->removeDownload(download);
                delete download;
                --m_downloadsToStart;
                ++removed;
                break;
            }
        }
    }
    if(removed == 1) {
        m_downloadStatusLabel->setText(tr("the download has been removed"));
    } else if(removed > 1) {
        m_downloadStatusLabel->setText(tr("%1 downloads have been removed").arg(removed));
    }
}

// methods for several gui features
void MainWindow::updateSelectionMode()
{
    m_ui->downloadsTreeView->setSelectionMode(UiPage::multiSelection() ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
}

void MainWindow::updateStartStopControls()
{
    QList<Download *> selectedDownloads = this->selectedDownloads();
    int toStart = 0;
    int toRestart = 0;
    int toResume = 0;
    int toInit = 0;
    int toStop = 0;
    int toInterrupt = 0;
    int noCommand = 0;
    int downloadLinksAvailable = 0;
    int removeable = 0;
    int withTargetPath = 0;
    bool downloadsSelected = !selectedDownloads.isEmpty();
    if(downloadsSelected) {
        foreach(Download *download, selectedDownloads) {
            switch(download->status()) {
            case DownloadStatus::None:
                ++toInit;
                ++removeable;
                break;
            case DownloadStatus::Initiating:
                ++toStop;
                break;
            case DownloadStatus::Downloading:
            case DownloadStatus::FinishOuputFile:
                ++downloadLinksAvailable;
                ++toStop;
                if(download->supportsRange()) {
                    ++toInterrupt;
                }
                break;
            case DownloadStatus::Ready:
                ++downloadLinksAvailable;
                ++removeable;
                if(download->supportsRange() && download->range().currentOffset() > 0) {
                    ++toResume;
                } else {
                    ++toStart;
                }
                break;
            case DownloadStatus::Failed:
                if(download->isInitiated()) {
                    ++toResume; // downloads that can be resumed can also be restarted, so toRestart is "included" here, too
                    ++downloadLinksAvailable;
                } else {
                    ++toInit;
                }
                ++removeable;
                break;
            case DownloadStatus::Interrupted:
                ++removeable;
                ++downloadLinksAvailable;
                if(download->supportsRange()) {
                    ++toResume;
                }
                break;
            case DownloadStatus::Finished:
                ++downloadLinksAvailable;
                ++toRestart;
                ++removeable;
                break;
            case DownloadStatus::Interrupting:
            case DownloadStatus::Aborting:
                ++noCommand;
                break;
            default:
                ++noCommand;
            }
            if(!download->targetPath().isEmpty()) {
                ++withTargetPath;
            }
        }
    }
    QString text;
    QString themeName;
    QString interruptResumeText;
    QString interruptResumeThemeName;
    if(noCommand == 0 && toStart && toRestart == 0 && toInit == 0 && toStop == 0 && toResume == 0) {
        // can start?
        if(toStart == 1) {
            text.append(tr("Start selected download"));
        } else {
            text = tr("Start selected downloads (%1)").arg(toStart);
        }
        themeName = QStringLiteral("media-playback-start");
    } else if(noCommand == 0 && (toRestart || toResume) && toStart == 0 && toInit == 0 && toStop == 0) {
        // can restart?  (if a download can be resumed, it can be also restarted)
        int sum = toRestart + toResume;
        if(sum == 1) {
            text.append(tr("Restart the selected download"));
        } else {
            text = tr("Restart the selected downloads (%1)").arg(sum);
        }
        themeName = QStringLiteral("media-playback-start");
        if(!toRestart) {
            // can resume?
            if(toResume == 1) {
                interruptResumeText.append(tr("Resume the selected download"));
            } else if(toResume > 1) {
                interruptResumeText = tr("Resume the selected downloads (%1)").arg(toInterrupt);
            }
            interruptResumeThemeName = QStringLiteral("media-playback-start");
        }
    } else if(noCommand == 0 && toInit && toStart == 0 && toRestart == 0 && toStop == 0 && toResume == 0) {
        // can initialize?
        if(toInit == 1) {
            text.append(tr("Load initial information for selected download"));
        } else {
            text = tr("Load initial information for selected downloads (%1)").arg(toInit);
        }
        themeName = QStringLiteral("media-playback-start");
    } else if(noCommand == 0 && toStop && toStart == 0 && toRestart == 0 && toInit == 0 && toResume == 0) {
        // can stop?
        if(toStop == 1) {
            text.append(tr("Stop selected download"));
        } else {
            text = tr("Stop selected downloads (%1)").arg(toStop);
        }
        themeName = QStringLiteral("media-playback-stop");
        // can interrupt?
        if(toInterrupt) {
            if(toInterrupt == 1) {
                interruptResumeText.append(tr("Interrupt the selected download"));
            } else {
                interruptResumeText = tr("Interrupt the selected downloads (%1)").arg(toInterrupt);
            }
            interruptResumeThemeName = QStringLiteral("media-playback-pause");
        }
    }
    if(text.isEmpty()) {
        m_ui->actionStart_selected->setEnabled(false);
    } else {
        m_ui->actionStart_selected->setEnabled(true);
        m_ui->actionStart_selected->setText(text);
        m_ui->actionStart_selected->setIcon(QIcon::fromTheme(themeName));
    }
    if(interruptResumeText.isEmpty()) {
        m_ui->actionResume_selected_downloads->setEnabled(false);
    } else {
        m_ui->actionResume_selected_downloads->setEnabled(true);
        m_ui->actionResume_selected_downloads->setText(interruptResumeText);
        m_ui->actionResume_selected_downloads->setIcon(QIcon::fromTheme(interruptResumeThemeName));
    }
    if(m_ui->toolBar->minimumWidth() < m_ui->toolBar->width()) {
        m_ui->toolBar->setMinimumWidth(m_ui->toolBar->width());
    }
    if(removeable == 1) {
        m_ui->actionRemove_selected_downloads_from_list->setText(tr("Remove selected download from list"));
    } else if(removeable > 1) {
        m_ui->actionRemove_selected_downloads_from_list->setText(tr("Remove selected downloads (%1) from list").arg(removeable));
    }
    m_ui->actionRemove_selected_downloads_from_list->setEnabled(removeable > 0);
    if(downloadLinksAvailable > 0) {
        if(downloadLinksAvailable == 1) {
            m_ui->actionCopy_download_url->setText(tr("Copy download url"));
        } else {
            m_ui->actionCopy_download_url->setText(tr("Copy download urls (%1)").arg(downloadLinksAvailable));
        }
        m_ui->actionCopy_download_url->setEnabled(true);
    } else {
        m_ui->actionCopy_download_url->setEnabled(false);
    }
    bool exactlyOneNoneActiveDownloadSelected = selectedDownloads.size() == 1 && toStop == 0 && toInterrupt == 0;
    m_ui->actionSet_range->setEnabled(exactlyOneNoneActiveDownloadSelected);
    m_ui->actionSet_target->setEnabled(exactlyOneNoneActiveDownloadSelected);
    m_ui->actionClear_target->setEnabled(downloadsSelected && toStop == 0 && toInterrupt == 0 && withTargetPath > 0);
}

void MainWindow::showDownloadsTreeViewContextMenu(const QPoint &pos)
{
    m_downloadsTreeViewContextMenu->exec(m_ui->downloadsTreeView->mapToGlobal(pos));
}

void MainWindow::trayIconActivated()
{
    trayIconActivated(QSystemTrayIcon::Trigger);
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showNormal();
#ifndef CONFIG_SHOW_TRAY_ICON_ALWAYS
        if(m_trayIcon && m_trayIcon->isVisible()) {
            m_trayIcon->hide();
        }
#endif
        break;
    default:
        ;
    }
}

void MainWindow::trayIconDestroyed(QObject *)
{
    m_trayIcon = nullptr;
    setupTrayIcon();
}

void MainWindow::clipboardDataChanged()
{
    if(!m_internalClipboardChange && m_superviseClipboardToolButton->isChecked()) {
        QString data = QApplication::clipboard()->text();
        QStringList lines = data.split(QChar('\n'), QString::SkipEmptyParts);
        foreach(QString line, lines) {
            if(Download *download = Download::fromUrl(line)) {
                addDownload(download);
                if(m_trayIcon && m_trayIcon->isVisible()) {
                    m_trayIcon->showMessage(windowTitle(), tr("The download \"%1\" has been added.").arg(data));
                }
            }
        }
    }
}

void MainWindow::copyDownloadUrl()
{
    m_internalClipboardChange = true;
    QList<Download *> downloads = selectedDownloads();
    QString urls;
    QListIterator<Download *> i(downloads);
    while(i.hasNext()) {
        if(i.hasPrevious()) {
            urls.append("\n");
        }
        urls.append(i.next()->downloadUrl().toString());
    }
    if(!urls.isEmpty()) {
        QApplication::clipboard()->setText(urls);
    } else {
        QMessageBox::warning(this, windowTitle(), tr("There are no downloads selected."));
    }
    m_internalClipboardChange = false;
}

void MainWindow::setDownloadRange()
{
    QList<Download *> downloads = selectedDownloads();
    if(downloads.size() == 1) {
        if(SetRangeDialog(downloads.at(0)->range(), this).exec() == QDialog::Accepted) {
            updateStartStopControls(); // download (not) might be resumable now
        }
    } else if(!downloads.size()) {
        QMessageBox::warning(this, windowTitle(), tr("There is no download selected."));
    } else {
        QMessageBox::warning(this, windowTitle(), tr("You can only set the range of a singe download at once."));
    }
}

void MainWindow::setTargetPath()
{
    QList<Download *> downloads = selectedDownloads();
    if(downloads.size() == 1) {
        QString path = QFileDialog::getSaveFileName(this, tr("Select target path for download"), downloads.front()->targetPath());
        if(!path.isEmpty()) {
            downloads.front()->setTargetPath(path);
            updateStartStopControls();
        }
    } else if(!downloads.size()) {
        QMessageBox::warning(this, windowTitle(), tr("There is no download selected."));
    } else {
        QMessageBox::warning(this, windowTitle(), tr("You can only set the target of a singe download at once."));
    }
}

void MainWindow::clearTargetPath()
{
    QList<Download *> downloads = selectedDownloads();
    if(!downloads.isEmpty()) {
        foreach(Download *download, downloads) {
            download->setTargetPath(QString());
        }
        updateStartStopControls();
    } else if(!downloads.size()) {
        QMessageBox::warning(this, windowTitle(), tr("There is no download selected."));
    }
}

void MainWindow::showYoutubeItagsInfo()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://en.wikipedia.org/wiki/YouTube#Quality_and_formats")));
}

void MainWindow::resetGroovesharkSession()
{
    GroovesharkDownload::resetSession();
    QMessageBox::information(this, windowTitle(), tr("Grooveshark session has been reset."));
}

void MainWindow::exploreDownloadsDir()
{
    if(TargetPage::targetDirectory().isEmpty()) {
        QMessageBox::warning(this, windowTitle(), tr("There is no download target selected."));
    } else {
        if(QDir(TargetPage::targetDirectory()).exists()) {
            DesktopUtils::openLocalFileOrDir(TargetPage::targetDirectory());
        } else {
            QMessageBox::warning(this, windowTitle(), tr("The selected download directory doesn't exist anymore."));
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_activeDownloads) {
        if(QSystemTrayIcon::isSystemTrayAvailable() && m_trayIcon) {
            if(!m_trayIcon->isVisible()) {
                m_trayIcon->show();
                QApplication::sendPostedEvents(m_trayIcon);
            }
            QTimer::singleShot(1000, Qt::CoarseTimer, this, SLOT(showTrayIconMessage()));
            event->ignore();
            setHidden(true);
        } else {
            if(QMessageBox::warning(this, windowTitle(), tr("Do you really want to exit?\nThere are still downloads running."),
                                    QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) {
                event->ignore();
            }
        }
    }
    UiPage::mainWindowGeometry() = saveGeometry();
    UiPage::mainWindowState() = saveState();
}

void MainWindow::downloadChangedStatus(Download *download)
{
    // update overall download information
    switch(download->status()) {
    case DownloadStatus::None:
        break;
    case DownloadStatus::Downloading:
        ++m_activeDownloads;
        break;
    case DownloadStatus::Initiating:
        ++m_initiatingDownloads;
        break;
    default:
        ++m_downloadsToStart;
    }
    switch(download->lastStatus()) {
    case DownloadStatus::None:
        break;
    case DownloadStatus::Downloading:
        --m_activeDownloads;
        if(m_activeDownloads <= 0 && m_trayIcon && m_trayIcon->isVisible()) {
            m_trayIcon->showMessage(windowTitle(), tr("All downloads have been finished."));
        }
        break;
    case DownloadStatus::Initiating:
        --m_initiatingDownloads;
        break;
    default:
        --m_downloadsToStart;
    }
    updateOverallStatus(download);
    updateStartStopControls();
    checkForDownloadsToStartAutomatically();
}

void MainWindow::downloadChangedProgress(Download *download)
{
    if(download->status() == DownloadStatus::Downloading && m_elapsedTime.elapsed() > 1000) {
        updateOverallStatus(download);
    }
}

void MainWindow::updateOverallStatus(Download *download)
{
    qint64 newBytesReceived = download->newBytesReceived();
    qint64 newBytesToReceive = download->newBytesToReceive();
    m_totalSpeed += download->shiftSpeed();
    StatsPage::bytesReceived() += newBytesReceived;
    m_stillToReceive += newBytesToReceive - newBytesReceived;
    m_remainingTime = m_totalSpeed > 0
            ? TimeSpan::fromSeconds(static_cast<double>(m_stillToReceive) / (m_totalSpeed * 125.0))
            : TimeSpan();
    if(m_activeDownloads >= 1) {
        // if there are active downloads, show it in the status bar and as tray icon tooltip
        QString status = tr("%1 active download").arg(m_activeDownloads);
        if(m_activeDownloads != 1) {
            status.append("s");
        }
        status.append(QStringLiteral(" - "));
        status.append(QString::fromStdString(ConversionUtilities::bitrateToString(m_totalSpeed, true)));
        if(!m_remainingTime.isNull()) {
            status.append(tr(", about %1 remaining").arg(QString::fromStdString(m_remainingTime.toString(TimeSpanOutputFormat::WithMeasures, true))));
        }
        if(m_trayIcon) {
            m_trayIcon->setToolTip(status);
        }
        m_downloadStatusLabel->setText(status);
        m_downloadStatusLabel->setVisible(true);
    } else if(m_downloadsToStart >= 1) {
        // if there are downloads that can be started, shot it in the status bar an as tray icon tooltip
        QString status = tr("%1 download").arg(m_downloadsToStart);
        if(m_downloadsToStart != 1)
            status.append(tr("s"));
        status.append(tr(" can be started"));
        if(m_trayIcon) {
            m_trayIcon->setToolTip(status);
        }
        m_downloadStatusLabel->setText(status);
        m_downloadStatusLabel->setVisible(true);
    } else {
        // if not, hide the status bar and clear tooltip of tray icon
        if(m_trayIcon) {
            m_trayIcon->setToolTip(QString());
        }
        m_downloadStatusLabel->setVisible(false);
    }
    m_elapsedTime.restart();
}

// methods to get specific downloads/rows

QList<Download *> MainWindow::selectedDownloads() const
{
    QList<Download *> res;
    QModelIndexList selectedRows = m_ui->downloadsTreeView->selectionModel()->selectedRows();
    foreach(QModelIndex index, selectedRows) {
        if(Download *download = m_model->download(index)) {
            res << download;
        }
    }
    return res;
}

void MainWindow::setupTrayIcon()
{
#ifdef CONFIG_USE_TRAY_ICON
    if(!m_trayIcon) {
#ifdef OS_WIN32
        m_trayIcon =  new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/hicolor/16x16/misc/trayicon.ico")));
#else
        m_trayIcon =  new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/hicolor/128x128/apps/videodownloader.png")));
#endif
        connect(m_trayIcon, &QSystemTrayIcon::activated, this, static_cast<void(MainWindow::*)(QSystemTrayIcon::ActivationReason)>(&MainWindow::trayIconActivated));
    }
    if(!m_trayIconMenu) {
        m_trayIconMenu = new QMenu(this);
        QAction *action;
        m_trayIconMenu->addMenu(m_ui->menuAdd);
        //m_trayIconMenu->addAction(m_ui->actionStart_selected);
        //m_trayIconMenu->addAction(m_ui->actionResume_selected_downloads);
        m_trayIconMenu->addSeparator();
        action = m_trayIconMenu->addAction(tr("Show window"));
        connect(action, &QAction::triggered, this, static_cast<void(MainWindow::*)(void)>(&MainWindow::trayIconActivated));
        action = m_trayIconMenu->addAction(tr("Quit"));
        connect(action, &QAction::triggered, &QApplication::quit);
        connect(m_trayIconMenu, &QMenu::destroyed, this, &MainWindow::trayIconDestroyed);
    }
    m_trayIcon->setContextMenu(m_trayIconMenu);
#ifdef CONFIG_SHOW_TRAY_ICON_ALWAYS
    m_trayIcon->show();
#endif
#endif
}

}
