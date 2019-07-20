#include "./addmultipledownloadswizard.h"
#include "./downloadinteraction.h"
#include "./settings.h"

#include "../network/download.h"
#include "../network/finder/downloadfinder.h"
#include "../network/finder/groovesharksearcher.h"
#include "../network/finder/linkfinder.h"
#include "../network/finder/youtubeplaylist.h"

#include "../model/downloadfinderresultsmodel.h"

#include <qtutilities/widgets/clearlineedit.h>

#include <QCheckBox>
#include <QCommandLinkButton>
#include <QHeaderView>
#include <QScrollBar>
#include <QTreeView>
#include <QVBoxLayout>

#include <functional>

using namespace QtUtilities;
using namespace Network;

namespace QtGui {

enum { AddMultipleDownloadsSelectSourcePageId, AddMultipleDownloadsEnterSearchTermPageId, AddMultipleDownloadsResultsPageId };

DownloadSource downloadSourceFromField(const QVariant &fieldValue)
{
    bool ok = false;
    int value = fieldValue.toInt(&ok);
    if (ok && value > 0 && value < 4) {
        return static_cast<DownloadSource>(value);
    }
    return DownloadSource::None;
}

AddMultipleDownloadsSelectSourcePage::AddMultipleDownloadsSelectSourcePage(QWidget *parent)
    : QWizardPage(parent)
    , m_selectedSource(DownloadSource::None)
{
    // create buttons
    QCommandLinkButton *webpageLinksButton = new QCommandLinkButton(tr("Links in ordinary webpage"));
    QCommandLinkButton *youtubePlaylistButton = new QCommandLinkButton(tr("YouTube playlist"));
    QCommandLinkButton *groovesharkAlbumButton = new QCommandLinkButton(tr("Grooveshark album"));
    QCommandLinkButton *groovesharkPlaylistButton = new QCommandLinkButton(tr("Grooveshark playlist"));
    // add layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(webpageLinksButton);
    layout->addWidget(youtubePlaylistButton);
    layout->addWidget(groovesharkAlbumButton);
    layout->addWidget(groovesharkPlaylistButton);
    setLayout(layout);
    // connect signals and slots
    connect(webpageLinksButton, &QCommandLinkButton::clicked,
        std::bind(&AddMultipleDownloadsSelectSourcePage::changeSource, this, DownloadSource::WebpageLinks));
    connect(youtubePlaylistButton, &QCommandLinkButton::clicked,
        std::bind(&AddMultipleDownloadsSelectSourcePage::changeSource, this, DownloadSource::YoutubePlaylist));
    connect(groovesharkAlbumButton, &QCommandLinkButton::clicked,
        std::bind(&AddMultipleDownloadsSelectSourcePage::changeSource, this, DownloadSource::GroovesharkAlbum));
    connect(groovesharkPlaylistButton, &QCommandLinkButton::clicked,
        std::bind(&AddMultipleDownloadsSelectSourcePage::changeSource, this, DownloadSource::GroovesharkPlaylist));
    // set title
    setTitle(tr("Source"));
    setSubTitle(tr("Select the source you want to add downloads from."));
}

bool AddMultipleDownloadsSelectSourcePage::isComplete() const
{
    return m_selectedSource != DownloadSource::None;
}

AddMultipleDownloadsEnterSearchTermPage::AddMultipleDownloadsEnterSearchTermPage(QWidget *parent)
    : QWizardPage(parent)
{
    // create line edit
    m_searchTermLineEdit = new ClearLineEdit(this);
    // create check boxes
    m_byIdCheckBox = new QCheckBox(tr("search by ID"), this);
    m_verifiedOnlyCheckBox = new QCheckBox(tr("show only verified songs if possible"), nullptr);
    // add layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_searchTermLineEdit);
    layout->addWidget(m_byIdCheckBox);
    layout->addWidget(m_verifiedOnlyCheckBox);
    setLayout(layout);
    // register fields
    registerField(QStringLiteral("term*"), m_searchTermLineEdit);
    registerField(QStringLiteral("byid"), m_byIdCheckBox);
    registerField(QStringLiteral("verifiedonly"), m_verifiedOnlyCheckBox);
}

QString AddMultipleDownloadsEnterSearchTermPage::searchTerm() const
{
    return m_searchTermLineEdit->text();
}

void AddMultipleDownloadsEnterSearchTermPage::initializePage()
{
    DownloadSource source = DownloadSource::None;
    if (AddMultipleDownloadsWizard *w = qobject_cast<AddMultipleDownloadsWizard *>(wizard())) {
        source = w->source();
    }
    // source = downloadSourceFromField(field(QStringLiteral("source"))); // does not work for some reason
    switch (source) {
    case DownloadSource::WebpageLinks:
        setTitle(tr("Specify the webpage"));
        setSubTitle(tr("Enter the URL."));
        setEnabled(true);
        m_searchTermLineEdit->setPlaceholderText(QStringLiteral("e. g. http://localhost"));
        m_byIdCheckBox->setHidden(true);
        m_verifiedOnlyCheckBox->setHidden(true);
        break;
    case DownloadSource::YoutubePlaylist:
        setTitle(tr("Specify YouTube playlist"));
        setSubTitle(tr("Enter the playlist URL or ID."));
        setEnabled(true);
        m_searchTermLineEdit->setPlaceholderText(QStringLiteral("e. g. https://www.youtube.com/playlist?list=PLg2u737x4TjfLFFEbEK9p77V2NRcLcTkr"));
        m_byIdCheckBox->setHidden(false);
        m_verifiedOnlyCheckBox->setHidden(true);
        break;
    case DownloadSource::GroovesharkAlbum:
        setTitle(tr("Specify Grooveshark album"));
        setSubTitle(tr("Enter a search term or the album ID."));
        setEnabled(true);
        m_searchTermLineEdit->setPlaceholderText(tr("search term or ID"));
        m_byIdCheckBox->setHidden(false);
        m_verifiedOnlyCheckBox->setHidden(false);
        break;
    case DownloadSource::GroovesharkPlaylist:
        setTitle(tr("Specify Grooveshark playlist"));
        setSubTitle(tr("Enter a search term or the playlist ID."));
        setEnabled(true);
        m_searchTermLineEdit->setPlaceholderText(tr("search term or ID"));
        m_byIdCheckBox->setHidden(false);
        m_verifiedOnlyCheckBox->setHidden(false);
        break;
    default:
        setTitle(tr("No source selected"));
        setSubTitle(tr("There is no source selected."));
        setEnabled(false);
        m_byIdCheckBox->setHidden(true);
        m_verifiedOnlyCheckBox->setHidden(true);
    }
}

AddMultipleDownloadsResultsPage::AddMultipleDownloadsResultsPage(QWidget *parent)
    : QWizardPage(parent)
    , m_finder(nullptr)
    , m_model(nullptr)
    , m_interaction(new DownloadInteraction(parent))
    , m_complete(false)
{
    // setup view
    m_view = new QTreeView(this);
    m_view->setSelectionMode(QTreeView::MultiSelection);
    m_view->setIndentation(0);
    m_model = new DownloadFinderResultsModel(nullptr, this);
    m_view->setModel(m_model);
    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AddMultipleDownloadsResultsPage::selectionChanged);
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &AddMultipleDownloadsResultsPage::scrollBarValueChanged);
    // add layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_view);
    setLayout(layout);
}

void AddMultipleDownloadsResultsPage::initializePage()
{
    DownloadSource source = DownloadSource::None;
    if (AddMultipleDownloadsWizard *w = qobject_cast<AddMultipleDownloadsWizard *>(wizard())) {
        source = w->source();
        w->setButtonText(QWizard::CustomButton1, tr("Select all"));
        w->button(QWizard::CustomButton1)->setIcon(QIcon::fromTheme(QStringLiteral("edit-select-all")));
        w->setOption(QWizard::HaveCustomButton1, true);
        connect(w, &QWizard::customButtonClicked, this, &AddMultipleDownloadsResultsPage::customButtonClicked);
    }
    // source = downloadSourceFromField(field(QStringLiteral("source"))); // does not work for some reason
    if (m_finder) {
        delete m_finder;
        m_finder = nullptr;
    }
    bool byId = field(QStringLiteral("byid")).toBool();
    QString term = field(QStringLiteral("term")).toString();
    bool verifiedOnly = field(QStringLiteral("verfiedonly")).toBool();
    switch (source) {
    case DownloadSource::WebpageLinks:
        setTitle(tr("Select links to be added"));
        m_collectionKind = tr("Webpage");
        m_collectionContent = tr("links");
        m_finder = new LinkFinder(QUrl(term), this);
        break;
    case DownloadSource::YoutubePlaylist:
        setTitle(tr("Select videos to be added"));
        m_collectionKind = tr("YouTube playlist");
        m_collectionContent = tr("videos");
        m_finder = byId ? new YoutubePlaylist(term, this) : new YoutubePlaylist(QUrl(term), this);
        break;
    case DownloadSource::GroovesharkAlbum:
        setTitle(tr("Select songs to be added"));
        m_collectionKind = tr("Grooveshark album");
        m_collectionContent = tr("songs");
        m_finder
            = new GroovesharkSearcher(term, byId ? GroovesharkSearchTermRole::AlbumId : GroovesharkSearchTermRole::AlbumName, verifiedOnly, this);
        break;
    case DownloadSource::GroovesharkPlaylist:
        setTitle(tr("Select songs to be added"));
        m_collectionKind = tr("Grooveshark playlist");
        m_collectionContent = tr("songs");
        m_finder = new GroovesharkSearcher(
            term, byId ? GroovesharkSearchTermRole::PlaylistId : GroovesharkSearchTermRole::PlaylistName, verifiedOnly, this);
        break;
    default:
        setTitle(tr("No source selected"));
    }
    m_model->setFinder(m_finder);
    updateSubTitle();
    selectionChanged(QItemSelection(), QItemSelection());
    if (m_finder) {
        connect(m_finder, &DownloadFinder::requestCreated, m_interaction, &DownloadInteraction::connectDownload);
        connect(m_finder, &DownloadFinder::finished, this, &AddMultipleDownloadsResultsPage::finderFinished);
        connect(m_finder, &DownloadFinder::newResultsAvailable, this, &AddMultipleDownloadsResultsPage::finderHasResults);
        m_finder->setContinueAutomatically(false);
        m_finder->setProxy(ProxyPage::proxy());
        m_finder->start();
    }
}

void AddMultipleDownloadsResultsPage::cleanupPage()
{
    if (wizard()) {
        wizard()->disconnect(this);
        wizard()->setOption(QWizard::HaveCustomButton1, false);
    }
    m_model->setFinder(nullptr);
    if (m_finder) {
        delete m_finder;
        m_finder = nullptr;
    }
}

bool AddMultipleDownloadsResultsPage::isComplete() const
{
    return m_complete;
}

QList<Download *> AddMultipleDownloadsResultsPage::results() const
{
    QList<Download *> results;
    if (m_finder) {
        QModelIndexList selectedRows = m_view->selectionModel()->selectedRows();
        const QList<Download *> &allResults = m_finder->results();
        foreach (const QModelIndex &index, selectedRows) {
            if (index.row() < allResults.size()) {
                results << allResults.at(index.row());
            }
        }
    }
    return results;
}

void AddMultipleDownloadsResultsPage::finderHasResults(const QList<Download *> &newResults)
{
    if (m_finder && !newResults.isEmpty()) {
        updateSubTitle();
        if (!m_finder->hasFinished() && !m_finder->isDownloading()) {
            QScrollBar *sb = m_view->verticalScrollBar();
            if (!sb || ((sb->maximum() + sb->minimum()) == 0)) {
                m_finder->start();
            }
        }
    }
}

void AddMultipleDownloadsResultsPage::finderFinished(bool success, const QString &reason)
{
    if (m_finder) {
        if (success) {
            updateSubTitle();
        } else {
            if (reason.isEmpty()) {
                setSubTitle(tr("Failed to retrieve %1.").arg(m_collectionContent));
            } else {
                setSubTitle(tr("Failed to retrieve %1: %2").arg(m_collectionContent, reason));
            }
        }
    }
}

void AddMultipleDownloadsResultsPage::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    int count = m_view->selectionModel()->selectedRows().count();
    if (wizard()) {
        if (count == 1) {
            wizard()->setButtonText(QWizard::FinishButton, tr("Add selected download").arg(count));
            setComplete(true);
        } else if (count > 0) {
            wizard()->setButtonText(QWizard::FinishButton, tr("Add selected downloads (%1)").arg(count));
            setComplete(true);
        } else {
            setComplete(false);
        }
        if (count >= m_model->rowCount()) {
            wizard()->setButtonText(QWizard::CustomButton1, tr("Repeal selection"));
        } else {
            wizard()->setButtonText(QWizard::CustomButton1, tr("Select all"));
        }
    }
}

void AddMultipleDownloadsResultsPage::scrollBarValueChanged()
{
    if (m_finder && !m_finder->hasFinished() && !m_finder->isDownloading()) {
        QScrollBar *sb = m_view->verticalScrollBar();
        if (!sb || (sb->value() == sb->maximum()) || ((sb->maximum() + sb->minimum()) == 0)) {
            m_finder->start();
        }
    }
}

void AddMultipleDownloadsResultsPage::customButtonClicked(int which)
{
    if (which == QWizard::CustomButton1) {
        selectAll();
    }
}

void AddMultipleDownloadsResultsPage::setComplete(bool complete)
{
    if (m_complete != complete) {
        m_complete = complete;
        emit completeChanged();
    }
}

void AddMultipleDownloadsResultsPage::selectAll()
{
    QItemSelectionModel *selectionModel = m_view->selectionModel();
    if (selectionModel->selectedRows().length() >= m_model->rowCount()) {
        selectionModel->clearSelection();
    } else {
        m_view->selectAll();
    }
}

void AddMultipleDownloadsResultsPage::updateSubTitle()
{
    QString subTitle;
    if (m_finder) {
        if (m_finder->resultCount()) {
            const QString &collectionTitle = m_finder->collectionTitle();
            if (collectionTitle.isEmpty()) {
                subTitle = tr("%1 %2 of the %3 have been retrieved.").arg(m_finder->resultCount()).arg(m_collectionContent, m_collectionKind);
            } else {
                subTitle = tr("%1 %2 of the %3 »%4« have been retrieved.")
                               .arg(m_finder->resultCount())
                               .arg(m_collectionContent, m_collectionKind, collectionTitle);
            }
            if (!m_finder->hasFinished()) {
                subTitle.append(' ');
                subTitle.append(tr("More %1 are available.").arg(m_collectionContent));
            }
        } else {
            if (m_finder->hasFinished()) {
                subTitle = tr("No %1 could be found.").arg(m_collectionContent);
            } else {
                subTitle = tr("Retrieving %1 ...").arg(m_collectionContent);
            }
        }
    } else {
        subTitle = tr("There is no source selected.");
    }
    setSubTitle(subTitle);
}

AddMultipleDownloadsWizard::AddMultipleDownloadsWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Add multiple downloads"));
    setPage(AddMultipleDownloadsSelectSourcePageId, new AddMultipleDownloadsSelectSourcePage);
    setPage(AddMultipleDownloadsEnterSearchTermPageId, new AddMultipleDownloadsEnterSearchTermPage);
    setPage(AddMultipleDownloadsResultsPageId, new AddMultipleDownloadsResultsPage);
    setButtonText(FinishButton, tr("Add selected downloads"));
    setButtonText(NextButton, tr("Next"));
    setButtonText(BackButton, tr("Back"));
    button(FinishButton)->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    button(NextButton)->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    button(BackButton)->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    button(CancelButton)->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
    setOption(WizardOption::HelpButtonOnRight, false);
}

DownloadSource AddMultipleDownloadsWizard::source() const
{
    return qobject_cast<AddMultipleDownloadsSelectSourcePage *>(page(AddMultipleDownloadsSelectSourcePageId))->selectedSource();
}

QList<Download *> AddMultipleDownloadsWizard::results() const
{
    return qobject_cast<AddMultipleDownloadsResultsPage *>(page(AddMultipleDownloadsResultsPageId))->results();
}
} // namespace QtGui
