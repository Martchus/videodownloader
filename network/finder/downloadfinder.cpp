#include "./downloadfinder.h"

#include "../download.h"

namespace Network {

/*!
 * \class DownloadFinder
 * \brief The DownloadFinder class is used as base class for all look up implementations used within the downloader application.
 */

/*!
 * \brief Constructs a new download finder.
 */
DownloadFinder::DownloadFinder(QObject *parent)
    : QObject(parent)
    , m_resultCount(0)
    , m_continueAutomatically(true)
    , m_finished(false)
{
}

/*!
 * \brief Destroys the download finder.
 */
DownloadFinder::~DownloadFinder()
{
    stop();
}

/*!
 * \brief Returns whether the finder is downloading.
 */
bool DownloadFinder::isDownloading() const
{
    return m_download && (m_download->status() == DownloadStatus::Downloading || m_download->status() == DownloadStatus::Initiating);
}

/*!
 * \brief Returns the download from the results which has the specified initial URL.
 * \returns Returns a pointer to the download (ownership remains by the finder) or nullptr
 *          if no download has been found.
 */
Download *DownloadFinder::downloadByInitialUrl(const QUrl &url) const
{
    for (Download *download : m_results) {
        if (download->initialUrl() == url) {
            return download;
        }
    }
    return nullptr;
}

/*!
 * \brief Returns whether a download with the specified download URL has been found.
 * \remarks Only initialized downloads can be checked. Results will not be initialized
 *          automatically.
 */
bool DownloadFinder::hasDownloadUrl(const QUrl &url) const
{
    for (const Download *download : m_results) {
        for (const OptionData &option : download->options()) {
            if (option.url() == url) {
                return true;
            }
        }
    }
    return false;
}

/*!
 * \brief Starts the search.
 */
bool DownloadFinder::start()
{
    m_finished = false;
    if (!isDownloading()) {
        if (m_download) {
            m_download->stop();
            m_download.release()->deleteLater();
        }
    } else {
        return false;
    }
    QString reasonForFail;
    m_download.reset(createRequest(reasonForFail));
    if (m_download) {
        // prepare download and buffer
        connect(m_download.get(), &Download::statusChanged, this, &DownloadFinder::downloadChangedStatus);
        m_download->setProxy(m_proxy);
        emit requestCreated(m_download.get());
        // ensure the output device is only provided by the the finder
        disconnect(m_download.get(), &Download::outputDeviceRequired, nullptr, nullptr);
        connect(m_download.get(), &Download::outputDeviceRequired, this, &DownloadFinder::downloadRequiresOutputDevice);
        m_download->init();
        return true;
    } else {
        emitFinishedSignal(false, reasonForFail);
        return false;
    }
}

/*!
 * \brief Stops searching.
 */
void DownloadFinder::stop()
{
    if (m_download) {
        m_download->stop();
    }
}

/*!
 * \brief Reports the collection title.
 *
 * Should be called when subclassing to propagate results.
 */
void DownloadFinder::reportResult(Download *result)
{
    result->setParent(this);
    result->setProxy(m_proxy);
    m_results << result;
}

/*!
 * \brief Handles a changed download status.
 */
void DownloadFinder::downloadChangedStatus(Download *download)
{
    switch (download->status()) {
    case DownloadStatus::Ready: {
        QString reasonForFail;
        if (finalizeRequest(download, reasonForFail)) {
            download->start();
        } else {
            emitFinishedSignal(false, reasonForFail);
        }
        break;
    }
    case DownloadStatus::Failed:
        emitFinishedSignal(false, download->statusInfo());
        break;
    case DownloadStatus::Finished:
        if (!m_buffer) {
            emitFinishedSignal(false, tr("The buffer hasn't been initialized correctly."));
        } else {
            m_buffer->seek(0);
            QByteArray data = m_buffer->readAll();
            m_buffer.reset();
            QString reasonForFail;
            switch (parseResults(data, reasonForFail)) {
            case ParsingResult::Error:
                m_finished = true;
                emitFinishedSignal(false, reasonForFail);
                break;
            case ParsingResult::Success:
                m_finished = true;
                emitNewResultsSignal();
                emitFinishedSignal(true, reasonForFail);
                break;
            case ParsingResult::AnotherRequestRequired:
                emitNewResultsSignal();
                if (m_continueAutomatically || m_results.size() <= 0) {
                    start();
                }
                break;
            default:
                emitFinishedSignal(false, tr("Invalid parsing status returned."));
            }
        }
        break;
    default:;
    }
}

void DownloadFinder::downloadRequiresOutputDevice(Download *download, size_t option)
{
    m_buffer.reset(new QBuffer);
    if (m_buffer->open(QIODevice::ReadWrite)) {
        download->provideOutputDevice(option, m_buffer.get(), false);
    } else {
        download->provideOutputDevice(option, nullptr);
    }
}

/*!
 * \brief Emits the new results signal.
 */
void DownloadFinder::emitNewResultsSignal()
{
    if (m_resultCount < static_cast<unsigned int>(m_results.size())) {
        emit aboutToMakeNewResultsAvailable(static_cast<unsigned int>(m_results.size()) - m_resultCount);
        const QList<Download *> newResults = m_results.mid(m_resultCount);
        m_resultCount = m_results.size();
        emit newResultsAvailable(newResults);
    }
}

/*!
 * \fn DownloadFinder::aboutToMakeNewResultsAvailable()
 * \brief Emitted just before new results are available.
 */

/*!
 * \fn DownloadFinder::newResultsAvailable()
 * \brief Indicates that new results are available.
 * \param newResults Holds the new results.
 * \remarks The download finder has the ownership over the returned results but the
 *          caller might take ownership using the setParent() method.
 */

/*!
 * \fn DownloadFinder::aboutToClearResults()
 * \brief Emitted just before the results are cleared.
 */

/*!
 * \fn DownloadFinder::resultsCleared()
 * \brief Indicates that the results have been cleared.
 */

/*!
 * \fn DownloadFinder::finished()
 * \brief Indicates that the search has been finished.
 * \param success Indicates whether the search was successful.
 * \param reasonForFail Might hold an error message in the case the search was not successful.
 */

/*!
 * \fn DownloadFinder::createRequest()
 * \brief Creates the request.
 *
 * Needs to be implemented when subclassing.
 */

/*!
 * \fn DownloadFinder::parseResults()
 * \brief Parses the results.
 *
 * Needs to be implemented when subclassing.
 */
}
