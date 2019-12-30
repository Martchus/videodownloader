#include "./settings.h"

#include "../network/download.h"
#include "../network/groovesharkdownload.h"

#include "resources/config.h"

#include "ui_proxypage.h"
#include "ui_targetpage.h"
#include "ui_useragentpage.h"

#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/optioncategory.h>
#include <qtutilities/settingsdialog/optioncategorymodel.h>
#include <qtutilities/settingsdialog/qtsettings.h>
#include <qtutilities/widgets/clearlineedit.h>

#include <c++utilities/conversion/stringconversion.h>

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGraphicsPixmapItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QSettings>
#include <QVBoxLayout>

#include <functional>

using namespace CppUtilities;
using namespace QtUtilities;
using namespace Network;

namespace QtGui {

TargetPage::TargetPage(QWidget *parentWindow)
    : TargetPageBase(parentWindow)
{
}

TargetPage::~TargetPage()
{
}

bool TargetPage::apply()
{
    if (hasBeenShown()) {
        targetDirectory() = ui()->defaultTargetLineEdit->text();
        overwriteWithoutAsking() = ui()->overwriteCheckBox->isChecked();
        determineTargetFileWithoutAsking() = ui()->askOnlyWhenThereIsNoAppropriateFilenameCheckBox->isChecked();
    }
    return true;
}

void TargetPage::reset()
{
    if (hasBeenShown()) {
        ui()->defaultTargetLineEdit->setText(targetDirectory());
        ui()->overwriteCheckBox->setChecked(overwriteWithoutAsking());
        ui()->askOnlyWhenThereIsNoAppropriateFilenameCheckBox->setChecked(determineTargetFileWithoutAsking());
    }
}

QString &TargetPage::targetDirectory()
{
    static QString dir;
    return dir;
}

bool &TargetPage::overwriteWithoutAsking()
{
    static bool val = false;
    return val;
}

bool &TargetPage::determineTargetFileWithoutAsking()
{
    static bool val = true;
    return val;
}

QWidget *TargetPage::setupWidget()
{
    QWidget *widget = TargetPageBase::setupWidget();
    // draw icon to info icon graphics view
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, widget);
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(icon.pixmap(32, 32));
    scene->addItem(item);
    ui()->infoIconGraphicsView->setScene(scene);
    // connect signals and slots
    QObject::connect(ui()->selectDefaultDirPushButton, &QPushButton::clicked, std::bind(&TargetPage::selectTargetDirectory, this));
    return widget;
}

void TargetPage::selectTargetDirectory()
{
    QFileDialog *dlg = new QFileDialog(parentWindow());
#ifdef Q_OS_WIN
    // the native dialog can only be shown modal under Windows
    dlg->setModal(true);
#else
    dlg->setModal(false);
#endif
    dlg->setOption(QFileDialog::ShowDirsOnly, true);
    dlg->setWindowTitle(QApplication::translate("QtGui::GeneralTargetOptionPage", "Select download target directory"));
    dlg->setDirectory(ui()->defaultTargetLineEdit->text());
    QObject::connect(dlg, &QFileDialog::accepted, [this, dlg] {
        if (dlg->selectedFiles().size() == 1) {
            ui()->defaultTargetLineEdit->setText(dlg->selectedFiles().at(0));
        }
    });
    dlg->show();
}

UiPage::UiPage(QWidget *parentWidget)
    : OptionPage(parentWidget)
    , m_multiSelectionCheckBox(nullptr)
{
}

UiPage::~UiPage()
{
}

bool UiPage::apply()
{
    if (hasBeenShown()) {
        multiSelection() = m_multiSelectionCheckBox->isChecked();
    }
    return true;
}

void UiPage::reset()
{
    if (hasBeenShown()) {
        m_multiSelectionCheckBox->setChecked(multiSelection());
    }
}

QByteArray &UiPage::mainWindowGeometry()
{
    static QByteArray geometry;
    return geometry;
}

QByteArray &UiPage::mainWindowState()
{
    static QByteArray state;
    return state;
}

bool &UiPage::multiSelection()
{
    static bool val = false;
    return val;
}

QWidget *UiPage::setupWidget()
{
    QWidget *widget = new QWidget();
    widget->setWindowTitle(QApplication::translate("QtGui::GeneralUiOptionPage", "User interface"));
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QLabel *mainWindowLabel = new QLabel(QApplication::translate("QtGui::GeneralUiOptionPage", "Main window"));
    mainWindowLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    layout->addWidget(mainWindowLabel);
    layout->addWidget(
        m_multiSelectionCheckBox = new QCheckBox(QApplication::translate("QtGui::GeneralUiOptionPage", "enable multi-selection"), widget));
    widget->setLayout(layout);
    return widget;
}

ProxyPage::ProxyPage(QWidget *parentWidget)
    : ProxyPageBase(parentWidget)
{
}

ProxyPage::~ProxyPage()
{
}

bool ProxyPage::apply()
{
    if (hasBeenShown()) {
        // set entered values to proxy
        if (ui()->enableCheckBox->isChecked()) {
            switch (ui()->typeComboBox->currentIndex()) {
            case 0:
                proxy().setType(QNetworkProxy::HttpProxy);
                break;
            case 1:
                proxy().setType(QNetworkProxy::Socks5Proxy);
                break;
            }
        } else {
            proxy().setType(QNetworkProxy::NoProxy);
        }
        proxy().setHostName(ui()->hostNameLineEdit->text());
        proxy().setPort(ui()->portSpinBox->value());
        proxy().setUser(ui()->userNameLineEdit->text());
        proxy().setPassword(ui()->passwordLineEdit->text());
    }
    return true;
}

void ProxyPage::reset()
{
    if (hasBeenShown()) {
        switch (proxy().type()) {
        case QNetworkProxy::HttpProxy:
            ui()->typeComboBox->setCurrentIndex(1);
            ui()->enableCheckBox->setChecked(true);
            ui()->widget->setEnabled(true);
            break;
        case QNetworkProxy::Socks5Proxy:
            ui()->typeComboBox->setCurrentIndex(2);
            ui()->enableCheckBox->setChecked(true);
            ui()->widget->setEnabled(true);
            break;
        case QNetworkProxy::NoProxy:
            ui()->typeComboBox->setCurrentIndex(0);
            ui()->enableCheckBox->setChecked(false);
            ui()->widget->setEnabled(false);
            break;
        default:;
        }
        ui()->hostNameLineEdit->setText(proxy().hostName());
        ui()->portSpinBox->setValue(proxy().port());
        ui()->userNameLineEdit->setText(proxy().user());
        ui()->passwordLineEdit->setText(proxy().password());
    }
}

QNetworkProxy &ProxyPage::proxy()
{
    static QNetworkProxy proxy;
    return proxy;
}

QWidget *ProxyPage::setupWidget()
{
    QWidget *widget = ProxyPageBase::setupWidget();
    widget->setWindowTitle(QApplication::translate("QtGui::NetworkProxyOptionPage", "Proxy server"));
    ui()->widget->setEnabled(false);
    // connect signals and slots
    QObject::connect(ui()->enableCheckBox, &QCheckBox::clicked, ui()->widget, &QWidget::setEnabled);
    QObject::connect(ui()->hostNameLineEdit, &QLineEdit::editingFinished, std::bind(&ProxyPage::updateProxy, this));
    return widget;
}

void ProxyPage::updateProxy()
{
    QStringList parts = ui()->hostNameLineEdit->text().split(":", QString::SkipEmptyParts);
    if (parts.count() == 2) {
        bool ok;
        int port = parts.at(1).toInt(&ok);
        if (ok) {
            ui()->hostNameLineEdit->setText(parts.at(0));
            ui()->portSpinBox->setValue(port);
        }
    }
}

UserAgentPage::UserAgentPage(QWidget *parentWidget)
    : UserAgentPageBase(parentWidget)
{
}

UserAgentPage::~UserAgentPage()
{
}

bool UserAgentPage::apply()
{
    if (hasBeenShown()) {
        useCustomUserAgent() = ui()->customRadioButton->isChecked();
        customUserAgent() = ui()->customLineEdit->text();
    }
    return true;
}

void UserAgentPage::reset()
{
    if (hasBeenShown()) {
        if (useCustomUserAgent()) {
            ui()->customRadioButton->setChecked(true);
        } else {
            ui()->defaultRadioButton->setChecked(true);
        }
        ui()->customLineEdit->setText(customUserAgent());
    }
}

bool &UserAgentPage::useCustomUserAgent()
{
    static bool useCustomUserAgent = false;
    return useCustomUserAgent;
}

QString &UserAgentPage::customUserAgent()
{
    static QString userAgent;
    return userAgent;
}

MiscPage::MiscPage(QWidget *parentWidget)
    : OptionPage(parentWidget)
    , m_redirectCheckBox(nullptr)
{
}

MiscPage::~MiscPage()
{
}

bool MiscPage::apply()
{
    if (hasBeenShown()) {
        redirectWithoutAsking() = m_redirectCheckBox->isChecked();
    }
    return true;
}

void MiscPage::reset()
{
    if (hasBeenShown()) {
        m_redirectCheckBox->setChecked(redirectWithoutAsking());
    }
}

bool &MiscPage::redirectWithoutAsking()
{
    static bool val = false;
    return val;
}

QWidget *MiscPage::setupWidget()
{
    QWidget *widget = new QWidget();
    widget->setWindowTitle(QApplication::translate("QtGui::NetworkMiscOptionPage", "Misc"));
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(
        m_redirectCheckBox = new QCheckBox(QApplication::translate("QtGui::NetworkMiscOptionPage", "follow redirections without asking"), widget));
    widget->setLayout(layout);
    return widget;
}

StatsPage::StatsPage(QWidget *parentWidget)
    : OptionPage(parentWidget)
    , m_receivedLabel(nullptr)
{
}

StatsPage::~StatsPage()
{
}

bool StatsPage::apply()
{
    return true;
}

void StatsPage::reset()
{
    if (hasBeenShown()) {
        m_receivedLabel->setText(QString::fromStdString(dataSizeToString(bytesReceived(), true)));
    }
}

quint64 &StatsPage::bytesReceived()
{
    static quint64 received;
    return received;
}

QWidget *StatsPage::setupWidget()
{
    QWidget *widget = new QWidget();
    widget->setWindowTitle(QApplication::translate("QtGui::NetworkStatsOptionPage", "Statistics"));
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    QFormLayout *formLayout = new QFormLayout(widget);
    formLayout->addRow(QApplication::translate("QtGui::NetworkStatsOptionPage", "Received data"), m_receivedLabel = new QLabel());
    QPushButton *refreshButton = new QPushButton(QApplication::translate("QtGui::NetworkStatsOptionPage", "Refresh"));
    refreshButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(refreshButton, &QPushButton::clicked, std::bind(&StatsPage::reset, this));
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(refreshButton);
    widget->setLayout(mainLayout);
    return widget;
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QtUtilities::SettingsDialog(parent)
{
    // setup categories
    QList<OptionCategory *> categories;
    OptionCategory *category;

    category = new OptionCategory(this);
    category->setDisplayName(tr("General"));
    category->assignPages({ new TargetPage(this), new UiPage() });
    category->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-other"), QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-general.png"))));
    categories << category;

    category = new OptionCategory(this);
    category->setDisplayName(tr("Network"));
    category->setIcon(QIcon::fromTheme(
        QStringLiteral("preferences-system-network"), QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-network.png"))));
    category->assignPages({ new ProxyPage(), new UserAgentPage(), new MiscPage(), new StatsPage() });
    categories << category;

    categories << qtSettings().category();

    categoryModel()->setCategories(categories);

    setMinimumSize(800, 450);
    setWindowIcon(
        QIcon::fromTheme(QStringLiteral("preferences-other"), QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-general.png"))));
}

SettingsDialog::~SettingsDialog()
{
}

QtSettings &qtSettings()
{
    static QtSettings v;
    return v;
}

void restoreSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(PROJECT_NAME));
    // move old config to new location
    const QString oldConfig
        = QSettings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName()).fileName();
    QFile::rename(oldConfig, settings.fileName()) || QFile::remove(oldConfig);
    settings.sync();

    settings.beginGroup("application");
    TargetPage::targetDirectory() = settings.value("defaulttargetdirectory").toString();
    TargetPage::overwriteWithoutAsking() = settings.value("overwritewithoutasking", false).toBool();
    TargetPage::determineTargetFileWithoutAsking() = settings.value("determinetargetfilewithoutasking", true).toBool();
    MiscPage::redirectWithoutAsking() = settings.value("redirectwithoutasking", true).toBool();
    UserAgentPage::useCustomUserAgent() = settings.value("usecustomuseragent", false).toBool();
    UserAgentPage::customUserAgent() = settings.value("customuseragent").toString();

    settings.beginGroup("proxy");
    bool validProxyType;
    int proxyType = settings.value("type", QVariant(QNetworkProxy::NoProxy)).toInt(&validProxyType);
    validProxyType = proxyType >= 0 && proxyType <= 5;
    QNetworkProxy &proxy = ProxyPage::proxy();
    proxy.setType(validProxyType ? static_cast<QNetworkProxy::ProxyType>(proxyType) : QNetworkProxy::NoProxy);
    proxy.setHostName(settings.value("hostname").toString());
    proxy.setPort(settings.value("port", QVariant(0)).toUInt());
    proxy.setUser(settings.value("user").toString());
    proxy.setPassword(settings.value("password").toString());
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("statistics");
    StatsPage::bytesReceived() = settings.value("totalbytesreceived", 0).toLongLong();
    settings.endGroup();

    settings.beginGroup("mainwindow");
    UiPage::mainWindowGeometry() = settings.value("geometry").toByteArray();
    UiPage::mainWindowState() = settings.value("state").toByteArray();
    UiPage::multiSelection() = settings.value("multiselection").toBool();

    // load grooveshark authentication file
    QString reason;
    if (!GroovesharkDownload::loadAuthenticationInformationFromFile(QStringLiteral(":/jsonobjects/groovesharkauthenticationinfo"), &reason)) {
        QMessageBox::warning(nullptr, QApplication::applicationName(),
            QCoreApplication::translate("QtGui::restoreSettings", "Unable to load Grooveshark authentication file: %1").arg(reason));
    }
}

void saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(PROJECT_NAME));

    settings.beginGroup("application");
    settings.setValue("defaulttargetdirectory", TargetPage::targetDirectory());
    settings.setValue("overwritewithoutasking", TargetPage::overwriteWithoutAsking());
    settings.setValue("determinetargetfilewithoutasking", TargetPage::determineTargetFileWithoutAsking());
    settings.setValue("redirectwithoutasking", MiscPage::redirectWithoutAsking());
    settings.setValue("usecustomuseragent", UserAgentPage::useCustomUserAgent());
    settings.setValue("customuseragent", UserAgentPage::customUserAgent());

    settings.beginGroup("proxy");
    const QNetworkProxy &proxy = ProxyPage::proxy();
    settings.setValue("type", proxy.type());
    settings.setValue("hostname", proxy.hostName());
    settings.setValue("port", proxy.port());
    settings.setValue("user", proxy.user());
    settings.setValue("password", proxy.password());
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("statistics");
    settings.setValue("totalbytesreceived", StatsPage::bytesReceived());
    settings.endGroup();

    settings.beginGroup("mainwindow");
    settings.setValue("geometry", UiPage::mainWindowGeometry());
    settings.setValue("state", UiPage::mainWindowState());
    settings.setValue("multiselection", UiPage::multiSelection());
    settings.endGroup();
}

void applySettingsToDownload(Download *download)
{
    download->setDefaultUserAgentUsed(!UserAgentPage::useCustomUserAgent());
    download->setCustomUserAgent(UserAgentPage::useCustomUserAgent() ? UserAgentPage::customUserAgent() : QString());
    download->setProxy(ProxyPage::proxy());
}
} // namespace QtGui

INSTANTIATE_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, TargetPage)
INSTANTIATE_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, ProxyPage)
INSTANTIATE_UI_FILE_BASED_OPTION_PAGE_NS(QtGui, UserAgentPage)
