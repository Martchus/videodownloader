#ifndef LINKFINDER_H
#define LINKFINDER_H

#include "./downloadfinder.h"

#include <QUrl>

namespace Network {

class LinkFinder : public DownloadFinder
{
    Q_OBJECT
public:
    explicit LinkFinder(const QUrl &url, QObject *parent = nullptr);

protected:
    Download *createRequest(QString &);

protected slots:
    ParsingResult parseResults(const QByteArray &data, QString &);

private:
    QUrl m_url;
};

}

#endif // LINKFINDER_H
