#ifndef VIDEOFINDER_H
#define VIDEOFINDER_H

#include <QBuffer>
#include <QNetworkProxy>
#include <QObject>

#include <memory>

namespace Network {

class Download;

class DownloadFinder : public QObject {
    Q_OBJECT

public:
    explicit DownloadFinder(QObject *parent = nullptr);
    virtual ~DownloadFinder();
    const QList<Download *> &results() const;
    void clearResults();
    unsigned resultCount() const;
    const QString &collectionTitle() const;
    bool continuesAutomatically() const;
    const QNetworkProxy &proxy() const;
    void setProxy(const QNetworkProxy &proxy);
    bool hasFinished() const;
    bool isDownloading() const;
    Download *downloadByInitialUrl(const QUrl &url) const;
    bool hasDownloadUrl(const QUrl &url) const;

public Q_SLOTS:
    bool start();
    void stop();
    void setContinueAutomatically(bool continueAutomatically);

signals:
    void requestCreated(Download *request);
    void aboutToMakeNewResultsAvailable(unsigned int count);
    void newResultsAvailable(const QList<Download *> &newResults);
    void aboutToClearResults();
    void resultsCleared();
    void finished(bool success, const QString &reason = QString());

protected:
    /*!
     * \brief Specifies possible return values of the evalResults() method.
     */
    enum class ParsingResult {
        Error, /**< Indicates that an error occured. reasonForFail might hold an error message. */
        Success, /**< Indicates that the results could be parsed correctly. */
        AnotherRequestRequired /**< Indicates that the results could be parsed correctly but there are still results to be fetched. */
    };

    virtual Download *createRequest(QString &reasonForFail) = 0;
    virtual bool finalizeRequest(Download *download, QString &reasonForFail);
    virtual ParsingResult parseResults(const QByteArray &data, QString &reasonForFail) = 0;

    void reportCollectionTitle(const QString &title);
    void reportResult(Download *result);

private Q_SLOTS:
    void downloadChangedStatus(Download *download);
    void downloadRequiresOutputDevice(Download *download, size_t option);

private:
    void emitNewResultsSignal();
    void emitFinishedSignal(bool success, const QString &reason = QString());

    std::unique_ptr<Download> m_download;
    std::unique_ptr<QBuffer> m_buffer;
    QString m_title;
    QNetworkProxy m_proxy;
    QList<Download *> m_results;
    unsigned int m_resultCount;
    bool m_continueAutomatically;
    bool m_finished;
};

/*!
 * \brief Returns the results.
 * \remarks The download finder has the ownership over the returned results but the
 *          caller might take ownership using the setParent() method.
 */
inline const QList<Download *> &DownloadFinder::results() const
{
    return m_results;
}

/*!
 * \brief Returns the collection title.
 */
inline const QString &DownloadFinder::collectionTitle() const
{
    return m_title;
}

/*!
 * \brief Clears all results.
 */
inline void DownloadFinder::clearResults()
{
    emit aboutToClearResults();
    m_results.clear();
    m_resultCount = 0;
    emit resultsCleared();
}

/*!
 * \brief Returns the number of results.
 */
inline unsigned DownloadFinder::resultCount() const
{
    return m_resultCount;
}

/*!
 * \brief Returns an indication whether the search continues automatically.
 */
inline bool DownloadFinder::continuesAutomatically() const
{
    return m_continueAutomatically;
}

/*!
 * \brief Returns the assigned proxy.
 */
inline const QNetworkProxy &DownloadFinder::proxy() const
{
    return m_proxy;
}

/*!
 * \brief Sets a proxy to be used when retrieving downloads.
 */
inline void DownloadFinder::setProxy(const QNetworkProxy &proxy)
{
    m_proxy = proxy;
}

/*!
 * \brief Sets whether the search should continue automatically.
 */
inline void DownloadFinder::setContinueAutomatically(bool continueAutomatically)
{
    m_continueAutomatically = continueAutomatically;
}

/*!
 * \brief Returns whether the search has been finished.
 */
inline bool DownloadFinder::hasFinished() const
{
    return m_finished;
}

/*!
 * \brief Finalizes the request.
 */
inline bool DownloadFinder::finalizeRequest(Download *, QString &)
{
    return true;
}

/*!
 * \brief Reports the collection title.
 *
 * Can be called when subclassing to provide such information.
 */
inline void DownloadFinder::reportCollectionTitle(const QString &title)
{
    m_title = title;
}

/*!
 * \brief Emits the finished signal.
 */
inline void DownloadFinder::emitFinishedSignal(bool success, const QString &reason)
{
    m_finished = true;
    emit finished(success, reason);
}
} // namespace Network

#endif // VIDEOFINDER_H
