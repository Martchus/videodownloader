#ifndef ADDDOWNLOADDIALOG_H
#define ADDDOWNLOADDIALOG_H

#include <QDialog>
#include <QInputDialog>

#include <memory>

namespace Network {
class Download;
}

namespace QtGui {

namespace Ui {
class AddDownloadDialog;
}

class AddDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddDownloadDialog(QWidget *parent = nullptr);
    ~AddDownloadDialog();

    Network::Download *result() const;
    void reset();
    bool hasValidInput() const;

signals:
    void addDownloadClicked();

private slots:
    void textChanged(const QString &text);
    void adjustDetectedDownloadType();
    void setLastUrl();
    void back();
    void returnPressed();
    void insertTextFromClipboard();

private:
    std::unique_ptr<Ui::AddDownloadDialog> m_ui;
    static QStringList s_knownDownloadTypeNames;
    int m_downloadTypeIndex;
    bool m_downloadTypeIndexAdjustedManually;
    bool m_validInput;
    QInputDialog *m_selectDownloadTypeInputDialog;
    QString m_lastUrl;
};
} // namespace QtGui

#endif // ADDDOWNLOADDIALOG_H
