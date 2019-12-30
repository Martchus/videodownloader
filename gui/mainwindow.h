#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <c++utilities/chrono/timespan.h>

#include <QElapsedTimer>
#include <QList>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include <memory>

QT_BEGIN_NAMESPACE
class QLabel;
class QMenu;
class QSpinBox;
class QToolButton;
QT_END_NAMESPACE

namespace QtUtilities {
class AboutDialog;
}

namespace Network {
class Download;
}

namespace QtGui {
class DownloadInteraction;
class AddDownloadDialog;
class AddMultipleDownloadsWizard;
class SettingsDialog;
class ProgressBarItemDelegate;
class ComboBoxItemDelegate;
class DownloadModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructor, Destructor
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void downloadChangedStatus(Network::Download *download);
    void downloadChangedProgress(Network::Download *download);

    void updateOverallStatus(Network::Download *download);
    void checkForDownloadsToStartAutomatically();
    void startOrStopSelectedDownloads();
    void interruptOrResumeSelectedDownloads();
    void removeSelectedDownloads();

    // methods to show the dialogs and messages
    void showAboutDialog();
    void showAddDownloadDialog();
    void addDownloadDialogResults();
    void showAddMultipleDownloadsDialog();
    void addMultipleDownloadsWizardResults();
    void showSettingsDialog();
    void settingsAccepted();
    void showTrayIconMessage();

    // methods for misc gui functionalities
    void copyDownloadUrl();
    void setDownloadRange();
    void setTargetPath();
    void clearTargetPath();
    void showYoutubeItagsInfo();
    void resetGroovesharkSession();
    void exploreDownloadsDir();
    void updateSelectionMode();
    void updateStartStopControls();
    void showDownloadsTreeViewContextMenu(const QPoint &pos);
    void trayIconActivated();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayIconDestroyed(QObject *);
    void clipboardDataChanged();

private:
    // methods to handle downloads
    void addDownload(Network::Download *download);
    QList<Network::Download *> selectedDownloads() const;
    void setupTrayIcon();

    // fields
    //  ui
    std::unique_ptr<Ui::MainWindow> m_ui;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;
    QMenu *m_downloadsTreeViewContextMenu;
    QSpinBox *m_autoSpinBox;
    QLabel *m_downloadStatusLabel;
    QToolButton *m_superviseClipboardToolButton;
    bool m_internalClipboardChange;
    //  model, deleagtes
    DownloadModel *m_model;
    ProgressBarItemDelegate *m_progressBarDelegate;
    ComboBoxItemDelegate *m_comboBoxDelegate;
    //  overall download status
    int m_activeDownloads;
    int m_downloadsToStart;
    int m_initiatingDownloads;
    double m_totalSpeed;
    qint64 m_stillToReceive;
    CppUtilities::TimeSpan m_remainingTime;
    QElapsedTimer m_elapsedTime;
    DownloadInteraction *m_downloadInteraction;
    AddDownloadDialog *m_addDownloadDlg;
    AddMultipleDownloadsWizard *m_addMultipleDownloadsWizard;
    SettingsDialog *m_settingsDlg;
    QtUtilities::AboutDialog *m_aboutDlg;
};
} // namespace QtGui

#endif // MAINWINDOW_H
