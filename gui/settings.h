#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// is not required here when building with GCC 4.9.1 or Clan 3.5 - MinGW 4.9.1 fails without including UI headers here
#include "gui/ui_targetpage.h"
#include "gui/ui_proxypage.h"
#include "gui/ui_useragentpage.h"

#include <qtutilities/settingsdialog/settingsdialog.h>
#include <qtutilities/settingsdialog/optionpage.h>

QT_BEGIN_NAMESPACE
class QByteArray;
class QLabel;
class QLineEdit;
class QCheckBox;
class QNetworkProxy;
QT_END_NAMESPACE

namespace Network {
class Download;
}

namespace QtGui {

namespace Ui {
class TargetPage;
class ProxyPage;
class UserAgentPage;
}

class GeneralTargetOptionPage : public Dialogs::UiFileBasedOptionPage<Ui::TargetPage>
{
public:
    explicit GeneralTargetOptionPage(QWidget *parentWindow);
    ~GeneralTargetOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static QString &targetDirectory();
    static bool &overwriteWithoutAsking();
    static bool &determineTargetFileWithoutAsking();
protected:
    QWidget *setupWidget();
private:
    void selectTargetDirectory();
};

class GeneralUiOptionPage : public Dialogs::OptionPage
{
public:
    explicit GeneralUiOptionPage();
    ~GeneralUiOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static QByteArray &mainWindowGeometry();
    static QByteArray &mainWindowState();
    static bool &multiSelection();
protected:
    QWidget *setupWidget();
private:
    QCheckBox *m_multiSelectionCheckBox;
};

class NetworkProxyOptionPage : public Dialogs::UiFileBasedOptionPage<Ui::ProxyPage>
{
public:
    explicit NetworkProxyOptionPage();
    ~NetworkProxyOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static QNetworkProxy &proxy();
protected:
    QWidget *setupWidget();
private:
    void updateProxy();
};

class NetworkUserAgentOptionPage : public Dialogs::UiFileBasedOptionPage<Ui::UserAgentPage>
{
public:
    explicit NetworkUserAgentOptionPage();
    ~NetworkUserAgentOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static bool &useCustomUserAgent();
    static QString &customUserAgent();
};

class NetworkMiscOptionPage : public Dialogs::OptionPage
{
public:
    explicit NetworkMiscOptionPage();
    ~NetworkMiscOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static bool &redirectWithoutAsking();
protected:
    QWidget *setupWidget();
private:
    QCheckBox *m_redirectCheckBox;
};

class NetworkStatsOptionPage : public Dialogs::OptionPage
{
public:
    explicit NetworkStatsOptionPage();
    ~NetworkStatsOptionPage();

    QString displayName() const;
    bool apply();
    void reset();
    static quint64 &bytesReceived();
protected:
    QWidget *setupWidget();
private:
    QLabel *m_receivedLabel;
};

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
