#ifndef ADDMULTIPLEDOWNLOADSWIZARD_H
#define ADDMULTIPLEDOWNLOADSWIZARD_H

#include <QNetworkProxy>
#include <QWizard>
#include <QWizardPage>

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QTreeView)
QT_FORWARD_DECLARE_CLASS(QItemSelection)

namespace QtUtilities {
class ClearLineEdit;
}

namespace Network {
class Download;
class DownloadFinder;
}

namespace QtGui {

class DownloadFinderResultsModel;
class DownloadInteraction;

enum class DownloadSource : int { None, WebpageLinks, YoutubePlaylist, GroovesharkAlbum, GroovesharkPlaylist };

extern DownloadSource downloadSourceFromField(const QVariant &fieldValue);

class AddMultipleDownloadsSelectSourcePage : public QWizardPage {
    Q_OBJECT
    Q_PROPERTY(DownloadSource selectedSource READ selectedSource)
public:
    explicit AddMultipleDownloadsSelectSourcePage(QWidget *parent = nullptr);
    DownloadSource selectedSource() const;
    bool isComplete() const;

private:
    void changeSource(DownloadSource source);
    DownloadSource m_selectedSource;
};

inline DownloadSource AddMultipleDownloadsSelectSourcePage::selectedSource() const
{
    return m_selectedSource;
}

inline void AddMultipleDownloadsSelectSourcePage::changeSource(DownloadSource source)
{
    m_selectedSource = source;
    emit completeChanged(); // might change
    if (source != DownloadSource::None && wizard()) {
        setField(QStringLiteral("source"), QVariant(static_cast<int>(source)));
        wizard()->next();
    }
}

class AddMultipleDownloadsEnterSearchTermPage : public QWizardPage {
    Q_OBJECT
public:
    explicit AddMultipleDownloadsEnterSearchTermPage(QWidget *parent = nullptr);
    QString searchTerm() const;
    void initializePage();

private:
    QtUtilities::ClearLineEdit *m_searchTermLineEdit;
    QCheckBox *m_byIdCheckBox;
    QCheckBox *m_verifiedOnlyCheckBox;
};

class AddMultipleDownloadsResultsPage : public QWizardPage {
    Q_OBJECT
public:
    explicit AddMultipleDownloadsResultsPage(QWidget *parent = nullptr);
    void initializePage();
    void cleanupPage();
    bool isComplete() const;
    Network::DownloadFinder *finder() const;
    QList<Network::Download *> results() const;

private slots:
    void finderHasResults(const QList<Network::Download *> &newResults);
    void finderFinished(bool success, const QString &reason = QString());
    void selectionChanged(const QItemSelection &, const QItemSelection &);
    void scrollBarValueChanged();
    void customButtonClicked(int which);
    void setComplete(bool complete);
    void selectAll();

private:
    void updateSubTitle();

    QTreeView *m_view;
    Network::DownloadFinder *m_finder;
    DownloadFinderResultsModel *m_model;
    DownloadInteraction *m_interaction;
    bool m_complete;
    QString m_collectionKind;
    QString m_collectionContent;
};

inline Network::DownloadFinder *AddMultipleDownloadsResultsPage::finder() const
{
    return m_finder;
}

class AddMultipleDownloadsWizard : public QWizard {
    Q_OBJECT
public:
    explicit AddMultipleDownloadsWizard(QWidget *parent = nullptr);
    DownloadSource source() const;
    QList<Network::Download *> results() const;
};
}

#endif // ADDMULTIPLEDOWNLOADSWIZARD_H
