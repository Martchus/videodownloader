#ifndef ADDMULTIPLEDOWNLOADSDIALOG_H
#define ADDMULTIPLEDOWNLOADSDIALOG_H

#include "download/download.h"
#include "download/finder/downloadfinder.h"

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QNetworkProxy>

namespace Ui {
class AddMultipleDownloadsDialog;
}

class AddMultipleDownloadsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AddMultipleDownloadsDialog(QWidget *parent = nullptr, const QNetworkProxy &proxy = QNetworkProxy());
    ~AddMultipleDownloadsDialog();
    QList<Download *> results();

public slots:
    void selectAll();
    void back();
    void reset();
    void abortSearch();

signals:
    void addResultsClicked();
        
private slots:
    void youtubePlaylistSelected();
    void groovesharkAlbumSelected();
    void groovesharkPlaylistSelected();
    void startSearch();
    void downloadChangedStatus(Download *download);
    void addResults(const QList<Download *> &results);
    void retrievingFinished(bool sucess, const QString &reasonForFail);
    void updateControls(const QItemSelection &selected, const QItemSelection &deselected);
    void scrollBarValueChanged();


private:
    enum class SelectedSource
    {
        YoutubePlaylist,
        GroovesharkAlbum,
        GroovesharkPlaylist
    };

    void addFinder(DownloadFinder *finder);
    void addDownloadInformation(Download *download);
    void showDownloadError(Download *download);

    std::unique_ptr<Ui::AddMultipleDownloadsDialog> m_ui;
    QStandardItemModel *m_model;
    std::unique_ptr<DownloadFinder> m_finder;
    SelectedSource m_selectedSource;
    QString m_listName;
    static const int m_maxInfoRequests;
    int m_currentInfoRequests;
    QList<Download *> m_pendingDownloads;
    const QNetworkProxy &m_proxy;
    static const int m_titleCol;
    static const int m_uploaderCol;
    static const int m_nrCol;
    static const int m_collectionCol;
    static const int m_durationCol;
    static const int m_idCol;

};

#endif // ADDMULTIPLEDOWNLOADSDIALOG_H
