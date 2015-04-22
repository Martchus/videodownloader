#include "addmultipledownloadsdialog.h"
#include "ui_addmultipledownloadsdialog.h"

#include "download/finder/youtubeplaylist.h"
#include "download/finder/groovesharksearcher.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItem>
#include <QScrollBar>

#define checkForExistingQuery if(m_finder) { return; }

using namespace ChronoUtilities;

const int AddMultipleDownloadsDialog::m_maxInfoRequests = 3;

const int AddMultipleDownloadsDialog::m_titleCol = 0;
const int AddMultipleDownloadsDialog::m_uploaderCol = 1;
const int AddMultipleDownloadsDialog::m_nrCol = 2;
const int AddMultipleDownloadsDialog::m_collectionCol = 3;
const int AddMultipleDownloadsDialog::m_durationCol = 4;
const int AddMultipleDownloadsDialog::m_idCol = 5;

const QString &emptyField()
{
    static const QString value = QStringLiteral("-");
    return value;
}

const QString &retrievingField()
{
    static const QString value = QApplication::translate("AddMultipleDownloadsDialog", "retrieving ...");
    return value;
}

AddMultipleDownloadsDialog::AddMultipleDownloadsDialog(QWidget *parent, const QNetworkProxy &proxy) :
    QDialog(parent),
    m_ui(new Ui::AddMultipleDownloadsDialog),
    m_selectedSource(SelectedSource::YoutubePlaylist),
    m_proxy(proxy)
{
    // setup ui
    m_ui->setupUi(this);
#ifdef Q_OS_WIN32
    setStyleSheet(QStringLiteral("* { font: 9pt \"Segoe UI\"; } #mainWidget { color: black; background-color: white; border: none; } #bottomWidget { background-color: #F0F0F0; border-top: 1px solid #DFDFDF; } QMessageBox QLabel, QInputDialog QLabel, QCommandLinkButton { font-size: 12pt; color: #003399; } #buttonsTabWidget, #resultsTabWidget { background-color: white; } #tabWidget:pane { border: none; }"));
#else
    setStyleSheet(QStringLiteral("#tabWidget:pane { border: none; }"));
#endif

    m_ui->tabWidget->tabBar()->setHidden(true);
    m_ui->tabWidget->setCurrentIndex(0);
    m_ui->progressBar->setHidden(true);
    m_ui->selectAllPushButton->setHidden(true);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(true);
    m_ui->moreDownloadsAvailableLabel->setHidden(true);

    // setup tree view
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(QStringList() << "Title" << "Uploader or Artist" << "Nr." << "Collection name" << "Duration" << "Id");
    m_ui->downloadsTreeView->setModel(m_model);
    if(QScrollBar *sb = m_ui->downloadsTreeView->verticalScrollBar())
        connect(sb, &QScrollBar::valueChanged, this, &AddMultipleDownloadsDialog::scrollBarValueChanged);

    // connect signals and slots
    connect(m_ui->backPushButton, &QPushButton::clicked, this, &AddMultipleDownloadsDialog::back);
    connect(m_ui->addPushButton, &QPushButton::clicked, this, &AddMultipleDownloadsDialog::addResultsClicked);
    connect(m_ui->addPushButton, &QPushButton::clicked, this, &AddMultipleDownloadsDialog::accept);
    connect(m_ui->searchPushButton, &QPushButton::clicked, this, &AddMultipleDownloadsDialog::startSearch);
    connect(m_ui->termLineEdit, &QLineEdit::returnPressed, this, &AddMultipleDownloadsDialog::startSearch);
    connect(m_ui->youtubePlaylistCommandLinkButton, &QCommandLinkButton::clicked, this, &AddMultipleDownloadsDialog::youtubePlaylistSelected);
    connect(m_ui->groovesharkAlbumCommandLinkButton, &QCommandLinkButton::clicked, this, &AddMultipleDownloadsDialog::groovesharkAlbumSelected);
    connect(m_ui->groovesharkPlaylistCommandLinkButton, &QCommandLinkButton::clicked, this, &AddMultipleDownloadsDialog::groovesharkPlaylistSelected);
    connect(m_ui->selectAllPushButton, &QPushButton::clicked, this, &AddMultipleDownloadsDialog::selectAll);
    connect(m_ui->downloadsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AddMultipleDownloadsDialog::updateControls);

    // set fields
    m_currentInfoRequests = 0;
}

AddMultipleDownloadsDialog::~AddMultipleDownloadsDialog()
{ }

QList<Download *> AddMultipleDownloadsDialog::results()
{
    QList<Download *> res;
    Download *download;
    int rowCount = m_model->rowCount();
    QModelIndexList selectedIndexes = m_ui->downloadsTreeView->selectionModel()->selectedRows();
    QModelIndex index;
    QStandardItem *item;

    for(int i = 0; i < rowCount; ++i) {
        index = m_model->index(i, 0);
        if(selectedIndexes.contains(index)) {
            item = m_model->item(i, 0);
            download = item->data(Qt::UserRole + 1).value<Download *>();
            if(download->status() != DownloadStatus::Failed)
                res.append(download);
        }
    }
    return res;
}

void AddMultipleDownloadsDialog::downloadChangedStatus(Download *download)
{
    switch(download->status()) {
    case DownloadStatus::Failed:
        showDownloadError(download);
        --m_currentInfoRequests;
        break;
    case DownloadStatus::Ready:
        addDownloadInformation(download);
        --m_currentInfoRequests;
        break;
    default:
        ;
    }

    while(m_currentInfoRequests < m_maxInfoRequests
          && !m_pendingDownloads.isEmpty()) {
        Download *download = m_pendingDownloads.takeFirst();
        if(!download->isInitiated() && download->isInitiatingInstantlyRecommendable()) {
            ++m_currentInfoRequests;
            download->init();
        }
    }
}

void AddMultipleDownloadsDialog::addFinder(DownloadFinder *finder)
{
    m_finder.reset(finder);
    m_finder->setContinueAutomatically(false);

    connect(finder, &DownloadFinder::newResultsAvailable, this, &AddMultipleDownloadsDialog::addResults);
    connect(finder, &DownloadFinder::finished, this, &AddMultipleDownloadsDialog::retrievingFinished);

    m_ui->tabWidget->setCurrentIndex(2);
    m_ui->progressBar->setHidden(false);
    m_ui->addPushButton->setHidden(false);
    m_ui->selectAllPushButton->setHidden(false);
    m_ui->searchPushButton->setHidden(true);
    m_pendingDownloads.clear();
    m_currentInfoRequests = 0;
    m_ui->titleLabel->setText(tr("Retrieving downloads ..."));

    finder->start();
}

void AddMultipleDownloadsDialog::addDownloadInformation(Download *download)
{
    QStandardItem *item;
    for(int row = 0; row < m_model->rowCount(); ++row) {
        item = m_model->item(row, 0);

        if(item->data(Qt::UserRole + 1).value<Download *>() == download) {
            const QString &title = download->title();
            const QString &id = download->id();
            const QString &uploader = download->uploader();
            const QString &collectionName = download->collectionName();
            int nr = download->positionInCollection();
            TimeSpan duration = download->duration();

            m_model->item(row, m_titleCol)->setText(title.isEmpty() ? emptyField() : title);
            m_model->item(row, m_uploaderCol)->setText(uploader.isEmpty() ? emptyField() : uploader);
            m_model->item(row, m_nrCol)->setText(nr == 0 ? emptyField() : QStringLiteral("%1").arg(nr));
            m_model->item(row, m_collectionCol)->setText(collectionName.isEmpty() ? emptyField() : collectionName);
            m_model->item(row, m_durationCol)->setText(duration.isNull() ? emptyField() : QString::fromStdString(duration.toString(TimeSpanOutputFormat::WithMeasures)));
            m_model->item(row, m_idCol)->setText(id.isEmpty() ? emptyField() : id);
            break;
        }
    }
}

void AddMultipleDownloadsDialog::showDownloadError(Download *download)
{
    QStandardItem *item;
    for(int row = 0; row < m_model->rowCount(); ++row) {
        item = m_model->item(row, 0);

        if(item->data(Qt::UserRole +1).value<Download *>() == download) {
            for(int i = m_titleCol; i <= m_idCol; i++) {
                item = m_model->item(row, i);
                if(i != m_titleCol)
                    item->setText(QStringLiteral("not found"));
                item->setSelectable(false);
            }
            break;
        }
    }
}

void AddMultipleDownloadsDialog::youtubePlaylistSelected()
{
    checkForExistingQuery;
    m_ui->enterTermLabel->setText(tr("Enter the url of the playlist or the id:"));
    m_ui->termLineEdit->setInputMethodHints(Qt::ImhUrlCharactersOnly);
    m_ui->termLineEdit->setText(m_ui->byIdCheckBox->isChecked() ? QString() : QStringLiteral("http://www.youtube.com/playlist?list="));
    m_ui->termLineEdit->selectAll();
    m_ui->byIdCheckBox->setHidden(false);
    m_ui->verifiedCheckBox->setHidden(true);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(false);
    m_selectedSource = SelectedSource::YoutubePlaylist;
    m_ui->tabWidget->setCurrentIndex(1);
}

void AddMultipleDownloadsDialog::groovesharkAlbumSelected()
{
    checkForExistingQuery;
    m_ui->enterTermLabel->setText(tr("Enter a search term or the album id:"));
    m_ui->termLineEdit->setInputMethodHints(Qt::ImhNone);
    m_ui->termLineEdit->clear();
    m_ui->byIdCheckBox->setHidden(false);
    m_ui->verifiedCheckBox->setHidden(false);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(false);
    m_selectedSource = SelectedSource::GroovesharkAlbum;
    m_ui->tabWidget->setCurrentIndex(1);
    // "124486"
}

void AddMultipleDownloadsDialog::groovesharkPlaylistSelected()
{
    checkForExistingQuery;
    m_ui->enterTermLabel->setText(tr("Enter a search term or the playlist id:"));
    m_ui->termLineEdit->setInputMethodHints(Qt::ImhNone);
    m_ui->termLineEdit->clear();
    m_ui->byIdCheckBox->setHidden(false);
    m_ui->verifiedCheckBox->setHidden(false);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(false);
    m_selectedSource = SelectedSource::GroovesharkPlaylist;
    m_ui->tabWidget->setCurrentIndex(1);
}

void AddMultipleDownloadsDialog::startSearch()
{
    if(m_ui->tabWidget->currentIndex() != 1)
        return;

    if(m_ui->termLineEdit->text().isEmpty())
        return;

    m_ui->searchPushButton->setHidden(false);

    switch(m_selectedSource) {
    case SelectedSource::YoutubePlaylist:
        addFinder(new YoutubePlaylist(
                      m_ui->byIdCheckBox->isChecked() ?
                          QUrl(QStringLiteral("http://www.youtube.com/playlist?list=%1").arg(m_ui->termLineEdit->text())) :
                          m_ui->termLineEdit->text(),
                      parent()));
        break;
    case SelectedSource::GroovesharkAlbum:
        addFinder(new GroovesharkSearcher(
                      m_ui->termLineEdit->text(),
                      m_ui->byIdCheckBox->isChecked() ? GroovesharkSearchTermRole::AlbumId : GroovesharkSearchTermRole::AlbumName,
                      m_ui->verifiedCheckBox->isChecked(),
                      parent()));
        break;
    case SelectedSource::GroovesharkPlaylist:
        addFinder(new GroovesharkSearcher(
                      m_ui->termLineEdit->text(),
                      m_ui->byIdCheckBox->isChecked() ? GroovesharkSearchTermRole::PlaylistId : GroovesharkSearchTermRole::PlaylistName,
                      m_ui->verifiedCheckBox->isChecked(),
                      parent()));
        break;
    }
}

void AddMultipleDownloadsDialog::addResults(const QList<Download *> &results)
{
    if(m_ui->tabWidget->currentIndex() != 2)
        return;

    foreach(Download *download, results) {
        QList<QStandardItem *> items;
        QStandardItem *item;

        item = new QStandardItem(download->title());
        item->setEditable(false);
        item->setData(QVariant::fromValue(download), Qt::UserRole + 1);
        items << item;

        item = new QStandardItem(download->uploader().isEmpty() ? retrievingField() : download->uploader());
        item->setEditable(false);
        items << item;

        item = new QStandardItem(download->positionInCollection() == 0 ? retrievingField() : QString("%1").arg(download->positionInCollection()));
        item->setEditable(false);
        items << item;

        item = new QStandardItem(download->collectionName().isEmpty() ? retrievingField() : download->collectionName());
        item->setEditable(false);
        items << item;

        item = new QStandardItem(download->duration().isNull() ? retrievingField() : QString::fromStdString(download->duration().toString()));
        item->setEditable(false);
        items << item;

        item = new QStandardItem(download->id().isEmpty() ? retrievingField() : download->id());
        item->setEditable(false);
        items << item;

        m_model->appendRow(items);

        connect(download, &Download::statusChanged, this, &AddMultipleDownloadsDialog::downloadChangedStatus);

        if(download->isInitiated() || (!download->isInitiatingInstantlyRecommendable()))
            addDownloadInformation(download);
        else {
            if(m_currentInfoRequests < m_maxInfoRequests) {
                ++m_currentInfoRequests;
                download->setProxy(m_proxy);
                download->init();
            } else {
                m_pendingDownloads.append(download);
            }
        }
    }

    m_ui->selectAllPushButton->setText(tr("Select all"));
    if(m_finder) {
        if(!m_finder->collectionTitle().isEmpty()) {
            m_ui->titleLabel->setText(m_listName + m_finder->collectionTitle());
        }

        if(!m_finder->hasFinished() && !m_finder->isDownloading()) {
            QScrollBar *sb = m_ui->downloadsTreeView->verticalScrollBar();
            if(!sb || ((sb->maximum() + sb->minimum()) == 0)) {
                m_ui->progressBar->setHidden(false);
                m_finder->start();
            } else {
                m_ui->progressBar->setHidden(true);
            }
        }
    }
}

void AddMultipleDownloadsDialog::scrollBarValueChanged()
{
    if(m_finder && !m_finder->hasFinished() && !m_finder->isDownloading()) {
        QScrollBar *sb = m_ui->downloadsTreeView->verticalScrollBar();
        if(!sb || (sb->value() == sb->maximum()) || ((sb->maximum() + sb->minimum()) == 0)) {
            m_ui->progressBar->setHidden(false);
            m_finder->start();
        } else {
            m_ui->progressBar->setHidden(true);
        }
    } else {
        m_ui->progressBar->setHidden(true);
    }
}

void AddMultipleDownloadsDialog::retrievingFinished(bool sucess, const QString &reasonForFail)
{
    if(m_finder) {
        m_ui->progressBar->setHidden(true);
        m_ui->selectAllPushButton->setHidden(false);
        m_ui->addPushButton->setHidden(false);
        m_ui->titleLabel->setText(sucess
                                 ? (m_listName + m_finder->collectionTitle())
                                 : tr("Failed to retrieve downloads: %1").arg(reasonForFail));
        m_finder.release()->deleteLater();
    }
}

void AddMultipleDownloadsDialog::updateControls(const QItemSelection &, const QItemSelection &)
{
    int count = m_ui->downloadsTreeView->selectionModel()->selectedRows().count();
    if(count > 0) {
        m_ui->addPushButton->setText(tr("Add selected downloads (%1)").arg(count));
        m_ui->addPushButton->setEnabled(true);
    } else {
        m_ui->addPushButton->setText(tr("Add selected downloads"));
        m_ui->addPushButton->setEnabled(false);
    }

    if(count >= m_model->rowCount())
        m_ui->selectAllPushButton->setText(tr("Repeal selection"));
    else
        m_ui->selectAllPushButton->setText(tr("Select all"));
}

void AddMultipleDownloadsDialog::selectAll()
{
    QItemSelectionModel *selectionModel = m_ui->downloadsTreeView->selectionModel();
    if(selectionModel->selectedRows().length() >= m_model->rowCount())
        selectionModel->clearSelection();
    else
        m_ui->downloadsTreeView->selectAll();
}

void AddMultipleDownloadsDialog::back()
{
    m_finder.reset();
    switch(m_ui->tabWidget->currentIndex()) {
    case 0:
        close();
        break;
    case 1:
        reset();
        break;
    case 2:
        abortSearch();
        break;
    }
}

void AddMultipleDownloadsDialog::reset()
{
    m_finder.reset();
    m_ui->progressBar->setHidden(true);
    m_ui->selectAllPushButton->setHidden(true);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(true);
    m_ui->tabWidget->setCurrentIndex(0);
    m_model->removeRows(0, m_model->rowCount());
    m_pendingDownloads.clear();
    m_currentInfoRequests = 0;
}


void AddMultipleDownloadsDialog::abortSearch()
{
    m_finder.reset();
    m_ui->tabWidget->setCurrentIndex(1);
    m_ui->progressBar->setHidden(true);
    m_ui->selectAllPushButton->setHidden(true);
    m_ui->addPushButton->setHidden(true);
    m_ui->searchPushButton->setHidden(false);
    m_model->removeRows(0, m_model->rowCount());
    m_pendingDownloads.clear();
    m_currentInfoRequests = 0;
    m_ui->titleLabel->setText(tr("Aborted"));
}
