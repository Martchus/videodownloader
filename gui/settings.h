#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// is not required here when building with GCC 4.9.1 or Clan 3.5 - MinGW 4.9.1 fails without including UI headers here
#include "ui_targetpage.h"
#include "ui_proxypage.h"
#include "ui_useragentpage.h"

#include <qtutilities/settingsdialog/settingsdialog.h>
#include <qtutilities/settingsdialog/optionpage.h>

QT_FORWARD_DECLARE_CLASS(QByteArray)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QNetworkProxy)

namespace Network {
class Download;
}

namespace QtGui {

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE(TargetPage)
    DECLARE_SETUP_WIDGETS
public:
    static QString &targetDirectory();
    static bool &overwriteWithoutAsking();
    static bool &determineTargetFileWithoutAsking();
private:
    void selectTargetDirectory();
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_OPTION_PAGE(UiPage)
    DECLARE_SETUP_WIDGETS
public:
    static QByteArray &mainWindowGeometry();
    static QByteArray &mainWindowState();
    static bool &multiSelection();
private:
    QCheckBox *m_multiSelectionCheckBox;
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE(ProxyPage)
    DECLARE_SETUP_WIDGETS
public:
    static QNetworkProxy &proxy();
private:
    void updateProxy();
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_UI_FILE_BASED_OPTION_PAGE(UserAgentPage)
public:
    static bool &useCustomUserAgent();
    static QString &customUserAgent();
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_OPTION_PAGE(MiscPage)
    DECLARE_SETUP_WIDGETS
public:
    static bool &redirectWithoutAsking();
private:
    QCheckBox *m_redirectCheckBox;
END_DECLARE_OPTION_PAGE

BEGIN_DECLARE_OPTION_PAGE(StatsPage)
    DECLARE_SETUP_WIDGETS
public:
    static quint64 &bytesReceived();
private:
    QLabel *m_receivedLabel;
END_DECLARE_OPTION_PAGE

class SettingsDialog : public Dialogs::SettingsDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

};

extern void restoreSettings();
extern void saveSettings();
extern void applySettingsToDownload(Network::Download *download);

}

#endif // SETTINGSDIALOG_H
