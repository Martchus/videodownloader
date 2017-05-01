#ifndef SETRANGEDIALOG_H
#define SETRANGEDIALOG_H

#include <QDialog>

#include <memory>

namespace Network {
class DownloadRange;
}

namespace QtGui {

namespace Ui {
class SetRangeDialog;
}

class SetRangeDialog : public QDialog {
    Q_OBJECT

public:
    explicit SetRangeDialog(Network::DownloadRange &range, QWidget *parent = nullptr);
    ~SetRangeDialog();

private slots:
    void confirm();

private:
    std::unique_ptr<Ui::SetRangeDialog> m_ui;
    Network::DownloadRange &m_range;
};
}

#endif // SETRANGEDIALOG_H
