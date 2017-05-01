#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>

#include <memory>

namespace QtGui {

namespace Ui {
class DownloadWidget;
}

class DownloadWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadWidget(QWidget *parent = nullptr);
    ~DownloadWidget();

private:
    void paintEvent(QPaintEvent *);

    std::unique_ptr<Ui::DownloadWidget> m_ui;
};
}

#endif // DOWNLOADWIDGET_H
