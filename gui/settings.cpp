#include "./settings.h"

#include "../network/download.h"
#include "../network/groovesharkdownload.h"

#include <qtutilities/settingsdialog/optioncategory.h>
#include <qtutilities/settingsdialog/optioncategorymodel.h>
#include <qtutilities/widgets/clearlineedit.h>

#include <c++utilities/conversion/stringconversion.h>

#include <QDir>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QApplication>
#include <QSettings>
#include <QNetworkProxy>

#include <functional>

using namespace ConversionUtilities;
using namespace Dialogs;
using namespace Widgets;
using namespace Network;

namespace QtGui {

GeneralTargetOptionPage::GeneralTargetOptionPage(QWidget *parentWindow) :
    Dialogs::UiFileBasedOptionPage<Ui::TargetPage>(parentWindow)
{}

GeneralTargetOptionPage::~GeneralTargetOptionPage()
{}

QString GeneralTargetOptionPage::displayName() const
{
    return QApplication::translate("QtGui::GeneralTargetOptionPage", "Target directory");
}

bool GeneralTargetOptionPage::apply()
{
    if(hasBeenShown()) {
        targetDirectory() = ui()->defaultTargetLineEdit->text();
        overwriteWithoutAsking() = ui()->overwriteCheckBox->isChecked();
        determineTargetFileWithoutAsking() = ui()->askOnlyWhenThereIsNoAppropriateFilenameCheckBox->isChecked();
    }
    return true;
}

void GeneralTargetOptionPage::reset()
{
    if(hasBeenShown()) {
        ui()->defaultTargetLineEdit->setText(targetDirectory());
        ui()->overwriteCheckBox->setChecked(overwriteWithoutAsking());
        ui()->askOnlyWhenThereIsNoAppropriateFilenameCheckBox->setChecked(determineTargetFileWithoutAsking());
    }
}

QString &GeneralTargetOptionPage::targetDirectory()
{
    static QString dir;
    return dir;
}

bool &GeneralTargetOptionPage::overwriteWithoutAsking()
{
    static bool val = false;
    return val;
}

bool &GeneralTargetOptionPage::determineTargetFileWithoutAsking()
{
    static bool val = true;
    return val;
}

QWidget *GeneralTargetOptionPage::setupWidget()
{
    QWidget *widget = UiFileBasedOptionPage<Ui::TargetPage>::setupWidget();
    // draw icon to info icon graphics view
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, widget);
    QGraphicsScene *scene = new QGraphicsScene();
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(icon.pixmap(32, 32));
    scene->addItem(item);
    ui()->infoIconGraphicsView->setScene(scene);
    // connect signals and slots
    QObject::connect(ui()->selectDefaultDirPushButton, &QPushButton::clicked, std::bind(&GeneralTargetOptionPage::selectTargetDirectory, this));
    return widget;
}

void GeneralTargetOptionPage::selectTargetDirectory()
{
    QFileDialog *dlg = new QFileDialog(parentWindow());
#ifdef Q_OS_WIN
    // the native dialog can only be shown modal under Windows
    dlg->setModal(true);
#else
    dlg->setModal(false);
#endif
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->setWindowTitle(QApplication::translate("QtGui::GeneralTargetOptionPage", "Select download target directory"));
    dlg->setDirectory(ui()->defaultTargetLineEdit->text());
    QObject::connect(dlg, &QFileDialog::accepted, [this, dlg] {
        if(dlg->selectedFiles().size() == 1) {
            ui()->defaultTargetLineEdit->setText(dlg->selectedFiles().at(0));
        }
    });
    dlg->show();
}

GeneralUiOptionPage::GeneralUiOptionPage() :
    m_multiSelectionCheckBox(nullptr)
{}

GeneralUiOptionPage::~GeneralUiOptionPage()
{}

QString GeneralUiOptionPage::displayName() const
{
    return QApplication::translate("QtGui::GeneralUiOptionPage", "User interface");
}

bool GeneralUiOptionPage::apply()
{
    if(hasBeenShown()) {
        multiSelection() = m_multiSelectionCheckBox->isChecked();
    }
    return true;
}

void GeneralUiOptionPage::reset()
{
    if(hasBeenShown()) {
        m_multiSelectionCheckBox->setChecked(multiSelection());
    }
}

QByteArray &GeneralUiOptionPage::mainWindowGeometry()
{
    static QByteArray geometry;
    return geometry;
}

QByteArray &GeneralUiOptionPage::mainWindowState()
{
    static QByteArray state;
    return state;
}

bool &GeneralUiOptionPage::multiSelection()
{
    static bool val = false;
    return val;
}

QWidget *GeneralUiOptionPage::setupWidget()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QLabel *mainWindowLabel = new QLabel(QApplication::translate("QtGui::GeneralUiOptionPage", "Main window"));
    mainWindowLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    layout->addWidget(mainWindowLabel);
    layout->addWidget(m_multiSelectionCheckBox = new QCheckBox(QApplication::translate("QtGui::GeneralUiOptionPage", "enable multi-selection"), widget));
    widget->setLayout(layout);
    return widget;
}

NetworkProxyOptionPage::NetworkProxyOptionPage()
{}

NetworkProxyOptionPage::~NetworkProxyOptionPage()
{}

QString NetworkProxyOptionPage::displayName() const
{
    return QApplication::translate("QtGui::NetworkProxyOptionPage", "Proxy server");
}

bool NetworkProxyOptionPage::apply()
{
    if(hasBeenShown()) {
        // set entered values to proxy
        if(ui()->enableCheckBox->isChecked()) {
            switch(ui()->typeComboBox->currentIndex()) {
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

void NetworkProxyOptionPage::reset()
{
    if(hasBeenShown()) {
        switch(proxy().type()) {
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
        default:
            ;
        }
        ui()->hostNameLineEdit->setText(proxy().hostName());
        ui()->portSpinBox->setValue(proxy().port());
        ui()->userNameLineEdit->setText(proxy().user());
        ui()->passwordLineEdit->setText(proxy().password());
    }
}

QNetworkProxy &NetworkProxyOptionPage::proxy()
{
    static QNetworkProxy proxy;
    return proxy;
}

QWidget *NetworkProxyOptionPage::setupWidget()
{
    QWidget *widget = Dialogs::UiFileBasedOptionPage<Ui::ProxyPage>::setupWidget();
    ui()->widget->setEnabled(false);
    // connect signals and slots
    QObject::connect(ui()->enableCheckBox, &QCheckBox::clicked, ui()->widget, &QWidget::setEnabled);
    QObject::connect(ui()->hostNameLineEdit, &QLineEdit::editingFinished, std::bind(&NetworkProxyOptionPage::updateProxy, this));
    return widget;
}

void NetworkProxyOptionPage::updateProxy()
{
    QStringList parts = ui()->hostNameLineEdit->text().split(":", QString::SkipEmptyParts);
    if(parts.count() == 2) {
        bool ok;
        int port = parts.at(1).toInt(&ok);
        if(ok) {
            ui()->hostNameLineEdit->setText(parts.at(0));
            ui()->portSpinBox->setValue(port);
        }
    }
}

NetworkUserAgentOptionPage::NetworkUserAgentOptionPage()
{}

NetworkUserAgentOptionPage::~NetworkUserAgentOptionPage()
{}

QString NetworkUserAgentOptionPage::displayName() const
{
    return QApplication::translate("QtGui::NetworkUserAgentOptionPage", "HTTP user agent");
}

bool NetworkUserAgentOptionPage::apply()
{
    if(hasBeenShown()) {
        useCustomUserAgent() = ui()->customRadioButton->isChecked();
        customUserAgent() = ui()->customLineEdit->text();
    }
    return true;
}

void NetworkUserAgentOptionPage::reset()
{
    if(hasBeenShown()) {
        if(useCustomUserAgent()) {
            ui()->customRadioButton->setChecked(true);
        } else {
            ui()->defaultRadioButton->setChecked(true);
        }
        ui()->customLineEdit->setText(customUserAgent());
    }
}

bool &NetworkUserAgentOptionPage::useCustomUserAgent()
{
    static bool useCustomUserAgent = false;
    return useCustomUserAgent;
}

QString &NetworkUserAgentOptionPage::customUserAgent()
{
    static QString userAgent;
    return userAgent;
}

NetworkMiscOptionPage::NetworkMiscOptionPage() :
    m_redirectCheckBox(nullptr)
{}

NetworkMiscOptionPage::~NetworkMiscOptionPage()
{}

QString NetworkMiscOptionPage::displayName() const
{
    return QApplication::translate("QtGui::NetworkMiscOptionPage", "Misc");
}

bool NetworkMiscOptionPage::apply()
{
    if(hasBeenShown()) {
        redirectWithoutAsking() = m_redirectCheckBox->isChecked();
    }
    return true;
}

void NetworkMiscOptionPage::reset()
{
    if(hasBeenShown()) {
        m_redirectCheckBox->setChecked(redirectWithoutAsking());
    }
}

bool &NetworkMiscOptionPage::redirectWithoutAsking()
{
    static bool val = false;
    return val;
}

QWidget *NetworkMiscOptionPage::setupWidget()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(m_redirectCheckBox = new QCheckBox(QApplication::translate("QtGui::NetworkMiscOptionPage", "follow redirections without asking"), widget));
    widget->setLayout(layout);
    return widget;
}

NetworkStatsOptionPage::NetworkStatsOptionPage() :
    m_receivedLabel(nullptr)
{}

NetworkStatsOptionPage::~NetworkStatsOptionPage()
{}

QString NetworkStatsOptionPage::displayName() const
{
    return QApplication::translate("QtGui::NetworkStatsOptionPage", "Statistics");
}

bool NetworkStatsOptionPage::apply()
{
    return true;
}

void NetworkStatsOptionPage::reset()
{
    if(hasBeenShown()) {
        m_receivedLabel->setText(QStringLiteral("%1 (%2 bytes)").arg(QString::fromStdString(ConversionUtilities::dataSizeToString(bytesReceived())), QString::number(bytesReceived())));
    }
}

quint64 &NetworkStatsOptionPage::bytesReceived()
{
    static quint64 received;
    return received;
}

QWidget *NetworkStatsOptionPage::setupWidget()
{
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    QFormLayout *formLayout = new QFormLayout(widget);
    formLayout->addRow(QApplication::translate("QtGui::NetworkStatsOptionPage", "Received data"), m_receivedLabel = new QLabel());
    QPushButton *refreshButton = new QPushButton(QApplication::translate("QtGui::NetworkStatsOptionPage", "Refresh"));
    refreshButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(refreshButton, &QPushButton::clicked, std::bind(&NetworkStatsOptionPage::reset, this));
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(refreshButton);
    widget->setLayout(mainLayout);
    return widget;
}

SettingsDialog::SettingsDialog(QWidget *parent) :
    Dialogs::SettingsDialog(parent)
{
    // setup categories
    QList<Dialogs::OptionCategory *> categories;
    Dialogs::OptionCategory *category;

    category = new Dialogs::OptionCategory(this);
    category->setDisplayName(tr("General"));
    category->assignPages(QList<Dialogs::OptionPage *>() << new GeneralTargetOptionPage(this) << new GeneralUiOptionPage());
    category->setIcon(QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-general.png")));
    categories << category;

    category = new Dialogs::OptionCategory(this);
    category->setDisplayName(tr("Network"));
    category->setIcon(QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-network.png")));
    category->assignPages(QList<Dialogs::OptionPage *>() << new NetworkProxyOptionPage() << new NetworkUserAgentOptionPage() << new NetworkMiscOptionPage() << new NetworkStatsOptionPage());
    categories << category;

    category = new Dialogs::OptionCategory(this);
    category->setDisplayName(tr("Specific"));
    category->setIcon(QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-specific.png")));
    category->assignPages(QList<Dialogs::OptionPage *>());
    categories << category;
    categoryModel()->setCategories(categories);

    setMinimumSize(800, 450);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-other"), QIcon(QStringLiteral(":/icons/hicolor/32x32/categories/preferences-general.png"))));
}

SettingsDialog::~SettingsDialog()
{}

void restoreSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,  QApplication::organizationName(), QApplication::applicationName());

    settings.beginGroup("application");
    GeneralTargetOptionPage::targetDirectory() = settings.value("defaulttargetdirectory").toString();
    GeneralTargetOptionPage::overwriteWithoutAsking() = settings.value("overwritewithoutasking", false).toBool();
    GeneralTargetOptionPage::determineTargetFileWithoutAsking() = settings.value("determinetargetfilewithoutasking", true).toBool();
    NetworkMiscOptionPage::redirectWithoutAsking() = settings.value("redirectwithoutasking", true).toBool();
    NetworkUserAgentOptionPage::useCustomUserAgent() = settings.value("usecustomuseragent", false).toBool();
    NetworkUserAgentOptionPage::customUserAgent() = settings.value("customuseragent").toString();

    settings.beginGroup("proxy");
    bool validProxyType;
    int proxyType = settings.value("type", QVariant(QNetworkProxy::NoProxy)).toInt(&validProxyType);
    validProxyType = proxyType >= 0 && proxyType <= 5;
    QNetworkProxy &proxy = NetworkProxyOptionPage::proxy();
    proxy.setType(validProxyType ? static_cast<QNetworkProxy::ProxyType>(proxyType) : QNetworkProxy::NoProxy);
    proxy.setHostName(settings.value("hostname").toString());
    proxy.setPort(settings.value("port", QVariant(0)).toUInt());
    proxy.setUser(settings.value("user").toString());
    proxy.setPassword(settings.value("password").toString());
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("statistics");
    NetworkStatsOptionPage::bytesReceived() = settings.value("totalbytesreceived", 0).toLongLong();
    settings.endGroup();

    settings.beginGroup("mainwindow");
    GeneralUiOptionPage::mainWindowGeometry() = settings.value("geometry").toByteArray();
    GeneralUiOptionPage::mainWindowState() = settings.value("state").toByteArray();
    GeneralUiOptionPage::multiSelection() = settings.value("multiselection").toBool();

    // load grooveshark authentication file
    QString groovesharkAuthenticationFile = QStringLiteral("groovesharkauthenticationinfo.json");
    QString errorMsg = QApplication::translate("QtGui::Settings", "Unable to read Grooveshark authentication information file.\n\nReason: %1\n\nThe values stored in this file are required when connection to Grooveshark. Built-in will values be used instead, but these might be deprecated.");
    QString reason;
    if(!QFile::exists(groovesharkAuthenticationFile)) {
        groovesharkAuthenticationFile = QStringLiteral("./res/groovesharkauthenticationinfo.json");
        if(!QFile::exists(groovesharkAuthenticationFile)) {
            groovesharkAuthenticationFile = QStringLiteral("/etc/videodownloader/json/groovesharkauthenticationinfo.json");
            if(!QFile::exists(groovesharkAuthenticationFile)) {
                groovesharkAuthenticationFile = QStringLiteral("/usr/share/videodownloader/json/groovesharkauthenticationinfo.json");
            }
        }
    }
    if(QFile::exists(groovesharkAuthenticationFile)) {
        if(!GroovesharkDownload::loadAuthenticationInformationFromFile(groovesharkAuthenticationFile, &reason)) {
            QMessageBox::warning(nullptr, QApplication::applicationName(), errorMsg.arg(errorMsg));
        }
    } else {
        QDir settingsDir = QFileInfo(settings.fileName()).absoluteDir();
        QString groovesharkAuthenticationFilePath = settingsDir.absoluteFilePath(groovesharkAuthenticationFile);
        if(QFile::exists(groovesharkAuthenticationFilePath)) {
            if(!GroovesharkDownload::loadAuthenticationInformationFromFile(groovesharkAuthenticationFilePath, &reason)) {
                QMessageBox::warning(nullptr, QApplication::applicationName(), errorMsg.arg(reason));
            }
        } else {
            QMessageBox::warning(nullptr, QApplication::applicationName(), errorMsg.arg(QApplication::translate("QtGui::Settings", "Unable to find \"groovesharkauthenticationinfo.json\".")));
        }
    }
}

void saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,  QApplication::organizationName(), QApplication::applicationName());

    settings.beginGroup("application");
    settings.setValue("defaulttargetdirectory", GeneralTargetOptionPage::targetDirectory());
    settings.setValue("overwritewithoutasking", GeneralTargetOptionPage::overwriteWithoutAsking());
    settings.setValue("determinetargetfilewithoutasking", GeneralTargetOptionPage::determineTargetFileWithoutAsking());
    settings.setValue("redirectwithoutasking", NetworkMiscOptionPage::redirectWithoutAsking());
    settings.setValue("usecustomuseragent", NetworkUserAgentOptionPage::useCustomUserAgent());
    settings.setValue("customuseragent", NetworkUserAgentOptionPage::customUserAgent());

    settings.beginGroup("proxy");
    const QNetworkProxy &proxy = NetworkProxyOptionPage::proxy();
    settings.setValue("type", proxy.type());
    settings.setValue("hostname", proxy.hostName());
    settings.setValue("port", proxy.port());
    settings.setValue("user", proxy.user());
    settings.setValue("password", proxy.password());
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("statistics");
    settings.setValue("totalbytesreceived", NetworkStatsOptionPage::bytesReceived());
    settings.endGroup();

    settings.beginGroup("mainwindow");
    settings.setValue("geometry", GeneralUiOptionPage::mainWindowGeometry());
    settings.setValue("state", GeneralUiOptionPage::mainWindowState());
    settings.setValue("multiselection", GeneralUiOptionPage::multiSelection());
    settings.endGroup();
}

void applySettingsToDownload(Download *download)
{
    download->setDefaultUserAgentUsed(!NetworkUserAgentOptionPage::useCustomUserAgent());
    download->setCustomUserAgent(NetworkUserAgentOptionPage::useCustomUserAgent() ? NetworkUserAgentOptionPage::customUserAgent() : QString());
    download->setProxy(NetworkProxyOptionPage::proxy());
}

}
