#include "./download.h"
#include "./permissionstatus.h"
// these includes are only needed to provide the Download::fromUrl method
#include "./bitsharedownload.h"
#include "./httpdownload.h"
#include "./socksharedownload.h"
#include "./youtubedownload.h"

#include <c++utilities/application/global.h>
#include <c++utilities/io/path.h>

#include <QFileInfo>
#include <QIODevice>
#include <QMessageBox>
#include <QTranslator>

#include <random>

using namespace std;
using namespace ChronoUtilities;
using namespace IoUtilities;

namespace Network {

/*!
 * \class Download
 * \brief The Download class is the base class for all download implementations used within the downloader application.
 *
 * The Download class does more then just downloading. It also writes the downloaded data to an output device and calculates
 * the current progress percentage, the current speed and the remaining time.
 *
 * Some implementations might feature different download options (e. g. different video
 * qualities) and are able to fetch additional meta data such as title and uploader. A suitable filename for the
 * output file is possibly provided as well.
 *
 * <h3>Methods to be implemented when subclassing:</h3>
 *  - doInit(): Starts the initialisation; called before the actual download. After initiating the reportInitiated() method
 *    must be called.
 *  - doDownload(): Starts the actual download.
 *  - abortDownload(): Aborts the current download.
 *  - checkStatusAndClear(): Checks the status and frees resources after the download has been completed; called
 *    after reportDownloadComplete() has been called. Results should be returned using the reportFinalDownloadStatus() method.
 *  - followRedirection(): Starts the download again using the redirection URL; called when a redirection is available
 *    and the redirection is accepted.
 *  - isInitiatingInstantlyRecommendable(): Returns whether instantly initiating is recommendable.
 *  - supportsRange(): Returns whether a range can be set.
 *  - typeName(): Returns the type of the download as string (e. g. "Youtube Download").
 *
 * All of these methods must return immediately. Lasting operations should be performed asynchronously.
 *
 * <h3>Methods to be called when subclassing:</h3>
 *  - reportInitiated(): Reports that the download has been initiated.
 *  - reportNewDataToBeWritten(): Reports that there is new data to be written available.
 *  - reportRedirectionAvailable(): Reports that there is a redirection available.
 *  - reportAuthenticationRequired(): Reports that authentication credentials are required.
 *  - reportSslErrors(): Reports that one or more SSL errors occured.
 *  - reportDownloadComplete(): Reports that the download is complete.
 *  - reportFinalDownloadStatus(): Reports the final download status.
 *  - addDownloadUrl(): Makes a download URL under the specified option name available. Meant to be called during initialization.
 *  - changeDownloadUrl(): Updates the download URL with the specified option index.
 *
 * <h3>Signals to be handled when using the class:</h3>
 *  - overwriteingPermissionRequired(): Emitted when the permission to overwrite an existing file is required. Overwriting might be allowed or
 *    refused using the setOverwritePermission() method.
 *  - appendingPermissionRequired(): Emitted when the permission to append data to an existing file is required. Appending might be allowed or
 *    refused using the setAppendPermission() method.
 *  - redirectionPermissonRequired(): Emitted when the permission to overwrite a file is required. Overwriting might be allowed or
 *    refused using the setRedirectPermission() method.
 *  - outputDeviceRequired(): Emitted when an output device for writing the downloaded data is required. An output device can be provided or denialed
 *    using the provideOutputDevice() method.
 *  - authenticationRequired(): Emitted when authentication credentials are required. Authentication credentials can be provided or denialed
 *    using the provideAuthenticationCredentials() method.
 */

int Download::m_defaultUserAgent = -1;

/*!
 * \brief Returns a random default user agent string.
 *
 * The default user agent is used and returned by userAgent() if no custom user agent has been assigned
 * using the setUserAgent() method.
 *
 * The default user agent is picked from a set of pre-defined user agent strings. To pick another
 * string use the scrambleDefaultUserAgent() method.
 */
const QString &Download::defaultUserAgent()
{
    static const QString agents[] = { QStringLiteral("Mozilla/5.0 (X11; Linux x86_64; rv:32.0) Gecko/20100101 Firefox/32.0"),
        QStringLiteral("Mozilla/5.0 (Windows NT 5.1; rv:31.0) Gecko/20100101 Firefox/31.0"),
        QStringLiteral("Mozilla/5.0 (X11; OpenBSD amd64; rv:28.0) Gecko/20100101 Firefox/28.0"),
        QStringLiteral("Mozilla/5.0 (Windows NT 6.3; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2049.0 Safari/537.36"),
        QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.120 Safari/537.36"),
        QStringLiteral("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/534.34 (KHTML, like Gecko) konqueror/4.14.0 Safari/534.34"),
        QStringLiteral("Opera/9.80 (Windows NT 6.0) Presto/2.12.388 Version/12.14") };
    if (m_defaultUserAgent < 0 || m_defaultUserAgent > 6) {
        scrambleDefaultUserAgent();
    }
    return agents[m_defaultUserAgent];
}

/*!
 * \brief Scrambles the default user agent.
 * \sa defaultUserAgent()
 */
void Download::scrambleDefaultUserAgent()
{
    random_device rd;
    default_random_engine engine(rd());
    uniform_int_distribution<int> distribution(0, 6);
    m_defaultUserAgent = distribution(engine);
}

/*!
 * \brief Returns a suitable filename for the download.
 *
 * This name might be used when creating the output file.
 */
QString Download::suitableFilename() const
{
    string title(this->title().toStdString());
    removeInvalidChars(title);
    return QString::fromStdString(title);
}

/*!
 * \brief Sets the meta data for the download.
 *
 * This method might be useful to provide meta data from an external source.
 */
void Download::provideMetaData(const QString &title, const QString &uploader, TimeSpan duration, const QString &collectionName,
    int positionInCollection, int views, const QString &rating)
{
    if (!title.isEmpty()) {
        this->m_title = title;
        this->m_dontAcceptNewTitleFromFilenameAnymore = true;
    }
    if (!uploader.isEmpty()) {
        this->m_uploader = uploader;
    }
    if (!duration.isNull()) {
        this->m_duration = duration;
    }
    if (!collectionName.isEmpty()) {
        this->m_collectionName = collectionName;
    }
    if (positionInCollection > 0) {
        this->m_positionInCollection = positionInCollection;
    }
    if (views > 0) {
        this->m_views = views;
    }
    if (!rating.isEmpty()) {
        this->m_rating = rating;
    }
}

/*!
 * \brief Returns an indication whether a instant initiation of the download is recommendable.
 *
 * Some downloads (such as YouTube downloads) should be initiated instantly to fetch information like
 * available qualities, title and uploader. Some downloads can not fetch these information during initialization
 * and might be initiated just before starting the download.
 */
bool Download::isInitiatingInstantlyRecommendable() const
{
    return false;
}

/*!
 * \brief Constructs a new download for the specified \a url.
 * \returns Returns the download or nullptr if no download could be
 *          constructed.
 */
Download *Download::fromUrl(const QUrl &url)
{
    QString scheme = url.scheme();
    QString host = url.host(QUrl::FullyDecoded);
    if (scheme == QLatin1String("http") || scheme == QLatin1String("https") || scheme == QLatin1String("ftp")) {
        if (host.contains(QStringLiteral("youtube"), Qt::CaseInsensitive) || host.contains(QStringLiteral("youtu.be"), Qt::CaseInsensitive)) {
            return new YoutubeDownload(url);
        } else if (host.contains(QStringLiteral("sockshare")) || host.contains(QStringLiteral("putlocker"))) {
            return new SockshareDownload(url);
        } else if (host.contains(QStringLiteral("bitshare"))) {
            return new BitshareDownload(url);
        } else {
            return new HttpDownload(url);
        }
    }
    return nullptr;
}

/*!
 * \brief Returns an indication whether the download can be started.
 * \sa start()
 * \param reasonIfNot Specifies a string which will hold the reason if the download can not be started.
 * \remarks This method is used internally be the start() method to determine whether the download
 *          can be started and to set the statusInfo() appropriately. Hence it is not required to perform
 *          this check before starting a download. You might just want to check isStarted() instead.
 */
bool Download::canStart(QString &reasonIfNot)
{
    if (!isInitiated()) {
        reasonIfNot = tr("Download is not initiated.");
    } else if (isStarted()) {
        reasonIfNot = tr("Download is already started.");
    } else if (!isValidOptionChosen()) {
        reasonIfNot = tr("No valid option chosen.");
    } else {
        return true;
    }
    return false;
}

/*!
 * \brief Starts the download. Does nothing if the download is already started.
 *
 * If a target path is has been set (using the setTargetPath() method) it will be used.
 *
 * If no target file is set, an output device will be requested later (when a suitable filename
 * might be available).
 *
 * \remarks The download needs to be initiated before and a valid option needs to be chosen.
 * \sa init()
 * \sa isValidOptionChosen()
 * \sa availableOptionCount()
 * \sa setChosenOption()
 */
void Download::start()
{
    if (!isStarted()) {
        QString reasonForFail;
        if (canStart(reasonForFail)) {
            for (auto &optionData : m_optionData) {
                optionData.m_downloadComplete = false;
                optionData.m_downloadAbortedInternally = false;
                optionData.m_bytesWritten = 0;
            }
            OptionData &optionData = m_optionData.at(chosenOption());
            if (!optionData.m_outputDevice && !m_targetPath.isEmpty()) {
                unique_ptr<QFile> targetDevice(new QFile(m_targetPath));
                if (prepareOutputDevice(chosenOption(), targetDevice.get(), true)) {
                    targetDevice.release();
                } else {
                    return; // output device couldn't be prepared, error already handled in prepareOutputDevice()
                }
            }
            setStatus(DownloadStatus::Downloading);
            doDownload();
        } else {
            setStatusInfo(reasonForFail);
            setStatus(DownloadStatus::Failed);
        }
    }
}

/*!
 * \brief Starts the download. Does nothing if the download is already started.
 *
 * The specified file will be created/opened by the download and used to store the received data.
 *
 * The current target path is replaced by the specified one.
 *
 * \remarks The download needs to be initiated before and a valid option needs to be chosen.
 * \sa init()
 * \sa isValidOptionChosen()
 * \sa availableOptionCount()
 * \sa setChosenOption()
 */
void Download::start(const QString &targetPath)
{
    if (!isStarted()) {
        finalizeOutputDevice(chosenOption());
        m_targetPath = targetPath;
        start();
    }
}

/*!
 * \brief Starts the download. Does nothing if the download is already started.
 *
 * The specified \a targetDevice will be used by the download to store the received data.
 * The download will ignore a previously set target path.
 *
 * The download take only ownership over the specified \a targetDevice if \a giveOwnership is true.
 *
 * \remarks The download needs to be initiated before and a valid option needs to be chosen.
 *
 * \sa init()
 * \sa isValidOptionChosen()
 * \sa availableOptionCount()
 * \sa setChosenOption()
 */
void Download::start(QIODevice *targetDevice, bool giveOwnership)
{
    if (!isStarted()) {
        QString reasonForFail;
        if (canStart(reasonForFail)) {
            finalizeOutputDevice(chosenOption());
            if (prepareOutputDevice(chosenOption(), targetDevice, giveOwnership)) {
                start();
            }
        } else {
            setStatusInfo(reasonForFail);
            setStatus(DownloadStatus::Failed);
        }
    }
}

/*!
 * \brief Stops the download. Does nothing if the download has not been started or has been ended yet.
 */
void Download::stop()
{
    if (isStarted()) {
        switch (status()) {
        case DownloadStatus::Interrupting:
            setStatus(DownloadStatus::Aborting);
            FALLTHROUGH;
        case DownloadStatus::Aborting:
            break;
        default:
            setStatus(DownloadStatus::Aborting);
            abortDownload();
        }
    } else if (status() == DownloadStatus::Initiating) {
        setStatus(DownloadStatus::Aborting);
        abortDownload();
    }
}

/*!
 * \brief Interrupts the download. Does nothing if the download has not been started or has been ended yet.
 *
 * The only difference to the stop() method is that the download will enter the interrupting/interrupted status
 * and not the failed status.
 */
void Download::interrupt()
{
    if (isStarted()) {
        switch (status()) {
        case DownloadStatus::Aborting:
            setStatus(DownloadStatus::Interrupting);
            break;
        case DownloadStatus::Interrupting:
            break;
        default:
            setStatus(DownloadStatus::Interrupting);
            abortDownload();
        }
    }
}

/*!
 * \brief Constructs a new donwload with the specified \a url.
 */
Download::Download(const QUrl &url, QObject *parent)
    : QObject(parent)
    , m_initialUrl(url)
    , m_dontAcceptNewTitleFromFilenameAnymore(false)
    , m_views(0)
    , m_positionInCollection(0)
    , m_chosenOption(0)
    , m_selectedOptionChanged(true)
    , m_availableOptionsChanged(true)
    , m_status(DownloadStatus::None)
    , m_lastState(DownloadStatus::None)
    , m_bytesReceived(-1)
    , m_bytesToReceive(-1)
    , m_newBytesReceived(0)
    , m_newBytesToReceive(0)
    , m_speed(0.0)
    , m_shiftSpeed(0.0)
    , m_shiftRemainingTime(TimeSpan())
    , m_statusInfo(QString())
    , m_time(QTime())
    , m_networkError(QNetworkReply::NoError)
    , m_initiated(false)
    , m_progressUpdateInterval(300)
    , m_useDefaultUserAgent(true)
    , m_proxy(QNetworkProxy::NoProxy)
{
    m_time.start();
}

/*!
 * \brief Starts the download again using the redirection URL.
 * \returns Returns an indication whether the operation succeeded.
 *
 * Needs to be implemented when subclassing.
 */
bool Download::followRedirection(size_t)
{
    return false;
}

/*!
 * \brief Destroys the download.
 */
Download::~Download()
{
}

/*!
 * \brief Sets the chosen option. This option will be used when starting the download.
 * \returns Returns an indication whether the chosen option could be set.
 */
bool Download::setChosenOption(size_t optionIndex)
{
    if (optionIndex < availableOptionCount()) {
        if (m_chosenOption != optionIndex) {
            m_chosenOption = optionIndex;
            m_selectedOptionChanged = true;
        }
        return true;
    } else {
        return false;
    }
}

/*!
 * \brief Starts initiating the download.
 *
 * The download will enter the initiating status. This method returns immediately.
 */
void Download::init()
{
    if ((!m_initiated) && (status() != DownloadStatus::Initiating)) {
        setStatus(DownloadStatus::Initiating);
        doInit();
    }
}

/*!
 * \brief Reports the initialization status.
 *
 * Needs to be called when subclassing after the initialzation ended.
 *
 * \param success Specifies whether the initialization succeeded.
 * \param reasonIfNot Specifies the reason if the initialization failed; ignored otherwise.
 * \param networkError Specifies if and what kind of network error occured.
 */
void Download::reportInitiated(bool success, const QString &reasonIfNot, const QNetworkReply::NetworkError &networkError)
{
    setNetworkError(networkError);
    if (success) {
        m_initiated = true;
        setStatus(DownloadStatus::Ready);
    } else {
        m_initiated = false;
        setStatusInfo(tr("The initial information for this download couldn't be retireved. Reason: ") + reasonIfNot);
        setStatus(DownloadStatus::Failed);
    }
}

/*!
 * \brief Prepares the specified output \a device for the option with the specified \a optionIndex.
 * \remarks If there is already an output device assigned it must be
 *          finalized before calling this method. This method is used internally to
 *          assign an output device instead of setting m_optionData.at(...).outputDevice directly.
 * \returns Returns whether the \a device could be prepared.
 *
 * If the operation fails the error is handled by the method.
 */
bool Download::prepareOutputDevice(size_t optionIndex, QIODevice *device, bool takeOwnership)
{
    bool ok = true;
    bool ready = true;
    OptionData &optionData = m_optionData.at(optionIndex);
    if (!device->isOpen()) {
        if (QFile *file = qobject_cast<QFile *>(device)) {
            if (file->exists() && file->size() > 0) {
                if (m_range.isUsedForWritingOutput() && m_range.currentOffset() > 0) {
                    // we need to append to an existing file
                    switch (optionData.m_appendPermission) {
                    case PermissionStatus::Unknown:
                    case PermissionStatus::Asking:
                        setStatus(DownloadStatus::Waiting);
                        optionData.m_outputDevice = device;
                        if (optionData.m_appendPermission != PermissionStatus::Asking) {
                            optionData.m_appendPermission = PermissionStatus::Asking;
                            emit appendingPermissionRequired(this, optionIndex, file->fileName(), m_range.currentOffset(), file->size());
                        }
                        ready = false;
                        break;
                    case PermissionStatus::Allowed:
                    case PermissionStatus::AlwaysAllowed:
                        usePermission(optionData.m_appendPermission);
                        break;
                    case PermissionStatus::Refused:
                    case PermissionStatus::AlwaysRefused:
                        usePermission(optionData.m_appendPermission);
                        setStatusInfo(tr("Appending to existing target file not permitted."));
                        ok = false;
                    }
                } else { // we need to overwrite the existing file
                    switch (optionData.m_overwritePermission) {
                    case PermissionStatus::Unknown:
                    case PermissionStatus::Asking:
                        setStatus(DownloadStatus::Waiting);
                        optionData.m_outputDevice = device;
                        if (optionData.m_overwritePermission != PermissionStatus::Asking) {
                            optionData.m_overwritePermission = PermissionStatus::Asking;
                            emit overwriteingPermissionRequired(this, optionIndex, file->fileName());
                        }
                        ready = false;
                        break;
                    case PermissionStatus::Allowed:
                    case PermissionStatus::AlwaysAllowed:
                        usePermission(optionData.m_overwritePermission);
                        // overwriting the file is allowed -> clear the present file
                        if (!file->resize(0)) {
                            setStatusInfo(tr("The already existing output file couldn't be cleared."));
                            ok = false;
                        }
                        break;
                    case PermissionStatus::Refused:
                    case PermissionStatus::AlwaysRefused:
                        usePermission(optionData.m_overwritePermission);
                        setStatusInfo(tr("Overwriting the existing target file not permitted."));
                        ok = false;
                    }
                }
            }
        }
        if (ok && ready && !device->open(QIODevice::WriteOnly | QIODevice::Append)) {
            setStatusInfo(tr("Unable to open the output file/stream."));
            ok = false;
        }
    } else if (!device->isWritable()) {
        setStatusInfo(tr("The output file/stream isn't writable."));
        ok = false;
    }
    if (ok && ready && m_range.isUsedForWritingOutput() && m_range.currentOffset() >= 0) {
        if (device->isSequential()) {
            setStatusInfo(tr("Unable to seek to the range on sequential output streams."));
            ok = false;
        } else {
            if (!device->seek(m_range.currentOffset())) {
                setStatusInfo(tr("Unable to seek to the range in the output file/stream."));
                ok = false;
            }
        }
    }
    optionData.m_outputDeviceReady = ok && ready;
    if (ok) {
        optionData.m_outputDevice = device;
        optionData.m_hasOutputDeviceOwnership = takeOwnership;
    } else { // handle error case
        if (isStarted()) {
            abortDownload(); // ensure download is aborted
        }
        if (takeOwnership && device) {
            delete device;
        }
        optionData.m_outputDevice = nullptr;
        optionData.m_hasOutputDeviceOwnership = false;
        optionData.m_buffer.reset();
        optionData.m_requestingNewOutputDevice = false;
        m_range.increaseCurrentOffset(optionData.m_bytesWritten);
        m_range.setUsedForRequest();
        m_range.setUsedForWritingOutput();
        setStatus(DownloadStatus::Failed);
    }
    return ok;
}

/*!
 * \brief Ensures that the currently assigned output device is prepared.
 *
 * Does nothing if there has been no output device assigned using prepareOutputDevice or
 * the output device is already prepared.
 */
void Download::ensureOutputDeviceIsPrepared(size_t optionIndex)
{
    OptionData &optionData = m_optionData.at(optionIndex);
    if (optionData.m_outputDevice && !optionData.m_outputDeviceReady) {
        if (!prepareOutputDevice(optionIndex, optionData.m_outputDevice, optionData.m_hasOutputDeviceOwnership)) {
            return; // output device can not be prepared
        }
        writeBufferToOutputDevice(optionIndex);
        optionData.m_stillWriting = false; // not writing anymore
        if (optionData.m_downloadComplete) { // download has ended, too
            checkStatusAndClear(optionIndex);
        }
    }
}

/*!
 * \brief Finalizes the output device.
 *
 * This method is meant to be called after the all data has been written to the output device.
 * The output device will be closed and deleted if the downloader has the ownership.
 * Does nothing if the download has not the ownership over the device or there is no output device assigned.
 */
void Download::finalizeOutputDevice(size_t optionIndex)
{
    OptionData &optionData = m_optionData.at(optionIndex);
    if (optionData.m_hasOutputDeviceOwnership && optionData.m_outputDevice) {
        if (optionData.m_outputDevice->isOpen()) {
            if (QFile *targetFile = qobject_cast<QFile *>(optionData.m_outputDevice)) {
                targetFile->flush();
            }
            optionData.m_outputDevice->close();
        }
        optionData.chuckOutputDevice();
    }
    optionData.m_outputDevice = nullptr;
    optionData.m_outputDeviceReady = false;
}

/*!
 * \brief Reports the final download status.
 *
 * Needs to be called when subclassing after the download has ended.
 *
 * \param optionIndex Specifies the concerning option.
 * \param success Specifies whether the download was successful.
 * \param statusDescription Specifies a status description.
 * \param networkError Specifies if or what kind of network error occured.
 */
void Download::reportFinalDownloadStatus(size_t optionIndex, bool success, const QString &statusDescription, QNetworkReply::NetworkError networkError)
{
    finalizeOutputDevice(optionIndex);
    const OptionData &optionData = m_optionData[optionIndex];
    if (!optionData.m_downloadAbortedInternally) {
        m_range.increaseCurrentOffset(optionData.m_bytesWritten);
        m_range.setUsedForRequest();
        m_range.setUsedForWritingOutput();
        if (success) {
            setStatusInfo(statusDescription);
            setStatus(DownloadStatus::Finished);
        } else {
            // set current offset of the range to be able to resume downloading
            setStatusInfo(statusDescription);
            setNetworkError(networkError);
            setStatus(DownloadStatus::Failed);
        }
    }
}

/*!
 * \brief Reports that the download has been interrupted.
 *
 * Needs to be called when subclassing after the download has been interrupted.
 */
void Download::reportDownloadInterrupted(size_t optionIndex)
{
    finalizeOutputDevice(optionIndex);
    // set current offset of the range to be able to resume downloading
    const OptionData &optionData = m_optionData[optionIndex];
    m_range.increaseCurrentOffset(optionData.m_bytesWritten);
    m_range.setUsedForRequest();
    m_range.setUsedForWritingOutput();
    setStatus(DownloadStatus::Interrupted);
}

/*!
 * \brief Reports the download progress.
 * \param bytesReceived Specifies the number of bytes received.
 * \param bytesToReceive Specifies the number of bytes to be received.
 *
 * Needs to be called when subclassing while downloading.
 * If the total number of bytes is unknown, -1 should be specified for \a bytesToRecieve.
 */
void Download::reportDownloadProgressUpdate(size_t optionIndex, qint64 bytesReceived, qint64 bytesToReceive)
{
    const OptionData &optionData = m_optionData[optionIndex];
    if (bytesReceived == bytesToReceive) {
        setProgress(bytesReceived, bytesToReceive);
    } else {
        if (!optionData.m_downloadComplete && lastProgressUpdate() >= m_progressUpdateInterval) {
            switch (status()) {
            case DownloadStatus::Interrupting:
            case DownloadStatus::Aborting:
                break;
            default:
                setStatus(DownloadStatus::Downloading);
            }
            setProgress(bytesReceived, bytesToReceive);
        }
    }
}

/*!
 * \brief Reports that new bytes are available.
 * \param inputDevice Specifies the device the download will read the available data from.
 *
 * Needs to be called when subclassing while downloading.
 *
 * Does nothing when currently not downloading.
 */
void Download::reportNewDataToBeWritten(size_t optionIndex, QIODevice *inputDevice)
{
    OptionData &optionData = m_optionData[optionIndex];
    optionData.m_stillWriting = true;
    char buffer[1024];
    streamsize read;
    qint64 written;
    if (optionData.m_outputDevice && optionData.m_outputDeviceReady) { // there's a ready output device
        if (!writeBufferToOutputDevice(optionIndex)) { // write data which has been buffered earlier first
            return; // error is already handled within writeBufferToOutputDevice(), just return here
        }
        // read the new data from the input device and write it to the output device
        if (inputDevice) {
            while ((read = inputDevice->read(buffer, sizeof(buffer))) > 0) {
                if ((written = optionData.m_outputDevice->write(buffer, read)) == read) {
                    optionData.m_bytesWritten += written;
                } else {
                    abortDownload(); // ensure download is aborted
                    optionData.m_buffer.reset();
                    optionData.m_requestingNewOutputDevice = false;
                    optionData.m_stillWriting = false;
                    optionData.m_downloadAbortedInternally = true;
                    reportFinalDownloadStatus(optionIndex, false, tr("Unable to write to provided output device."));
                    return;
                }
            }
        }
        optionData.m_stillWriting = false; // not writing anymore
        if (optionData.m_downloadComplete) {
            // check status and clear if the download is done; shouldn't happen?
            checkStatusAndClear(optionIndex);
        }
    } else {
        // there's no ready output device -> use a buffer to store the data
        if (!optionData.m_buffer) { // create a new buffer if none exists
            optionData.m_buffer.reset(new stringstream(stringstream::in | stringstream::out));
        }
        // write the data to the buffer
        if (inputDevice) {
            while ((read = inputDevice->read(buffer, sizeof(buffer))) > 0) {
                optionData.m_buffer->write(buffer, read);
            }
        }
        if (!optionData.m_outputDevice && !optionData.m_requestingNewOutputDevice) {
            // request a new output device if not requested yet
            optionData.m_requestingNewOutputDevice = true;
            if (optionData.m_downloadComplete) {
                // set the status to waiting if the actual download is complete but an output device is still required
                setStatus(DownloadStatus::Waiting);
            }
            emit outputDeviceRequired(this, optionIndex);
        } // else: we're still waiting for the output device to be ready but there's nothing to be done about that here
    }
}

/*!
 * \brief Writes buffer data to the output device.
 * \remarks
 *          - Does nothing if there is no buffered data or if there is no output device.
 *          - Wipes the buffer afterwards.
 * \returns Returns an indication whether the operation succeeded.
 *
 * If the operation fails the error will be handled within the method.
 */
bool Download::writeBufferToOutputDevice(size_t optionIndex)
{
    OptionData &optionData = m_optionData[optionIndex];
    streamsize read;
    qint64 written;
    if (optionData.m_buffer && optionData.m_outputDevice && optionData.m_outputDeviceReady) {
        optionData.m_buffer->seekg(0);
        char buffer[1024];
        while (optionData.m_buffer->good()) {
            optionData.m_buffer->read(buffer, sizeof(buffer));
            read = optionData.m_buffer->gcount();
            if ((written = optionData.m_outputDevice->write(buffer, read)) >= 0) {
                optionData.m_bytesWritten += written;
            } else {
                optionData.m_buffer.reset();
                optionData.m_requestingNewOutputDevice = false;
                optionData.m_stillWriting = false;
                optionData.m_downloadAbortedInternally = true;
                abortDownload(); // ensure download is aborted
                reportFinalDownloadStatus(optionIndex, false, tr("Unable to write to provided output device."));
                return false;
            }
        }
        optionData.m_buffer.reset();
    }
    return true;
}

/*!
 * \brief Report that a redirection is available.
 *
 * Needs to be called when subclassing if there's a redirection available. An option for the
 * redirection needs to be added before.
 * It is recommendable to specify the index of the new option because the index might be used
 * when asking the user for if the redirection should accepted.
 */
void Download::reportRedirectionAvailable(size_t originalOptionIndex)
{
    OptionData &optionData = m_optionData.at(originalOptionIndex);
    switch (optionData.m_redirectPermission) {
    case PermissionStatus::Unknown:
        setStatus(DownloadStatus::Waiting);
        optionData.m_redirectPermission = PermissionStatus::Asking;
        emit redirectionPermissonRequired(this, originalOptionIndex, optionData.m_redirectionOf);
        break;
    case PermissionStatus::Allowed:
    case PermissionStatus::AlwaysAllowed:
        usePermission(optionData.m_redirectPermission);
        m_range.resetCurrentOffset(); // reset current offset
        if (followRedirection(optionData.m_redirectsTo)) {
            optionData.m_downloadComplete = false;
            setStatus(DownloadStatus::Downloading);
        } else {
            reportFinalDownloadStatus(originalOptionIndex, false, tr("Follwing redirection failed."));
        }
        break;
    case PermissionStatus::Refused:
    case PermissionStatus::AlwaysRefused:
        usePermission(optionData.m_redirectPermission);
        reportFinalDownloadStatus(originalOptionIndex, true, tr("Download finished, redirection rejected."));
        break;
    default:;
    }
}

/*!
 * \brief Reports that authentication credentials are required.
 *
 * \param optionIndex Specifies the concerning option index. Use Network::InvalidOptionIndex to request
 *                    credentials for initialization.
 * \param realm Specifies the realm.
 *
 * Needs to be called when subclassing to ask for credentials. The download will emit authenticationRequired().
 */
void Download::reportAuthenticationRequired(size_t optionIndex, const QString &realm)
{
    AuthenticationCredentials &credentials = optionIndex == InvalidOptionIndex ? m_initAuthData : m_optionData.at(optionIndex).m_authData;
    if (!credentials.m_requested) {
        credentials.m_requested = true;
        emit authenticationRequired(this, optionIndex, realm);
    }
}

/*!
 * \brief Reports that SSL errors occured.
 *
 * \param optionIndex Specifies the concerning option index. Use a negative value when the
 *                    errors occured during initialization.
 * \param reply Specifies the concerning reply.
 * \param sslErrors Specifies which SSL errors occured.
 *
 * Needs to be called when subclassing if SSL errors occured.
 */
void Download::reportSslErrors(size_t optionIndex, QNetworkReply *reply, const QList<QSslError> &sslErrors)
{
    OptionData &optionData = m_optionData.at(optionIndex);
    switch (optionData.m_ignoreSslErrorsPermission) {
    case PermissionStatus::Unknown:
        optionData.m_ignoreSslErrorsPermission = PermissionStatus::Asking;
        emit this->sslErrors(this, optionIndex, sslErrors);
        break;
    case PermissionStatus::Refused:
        usePermission(optionData.m_ignoreSslErrorsPermission);
        FALLTHROUGH;
    case PermissionStatus::Allowed:
    case PermissionStatus::AlwaysAllowed:
        reply->ignoreSslErrors(sslErrors);
        break;
    default:;
    }
}

/*!
 * \brief Reports that the download with the specified \a optionIndex is complete.
 *
 * Needs to be called when subclassing after the download is complete.
 */
void Download::reportDownloadComplete(size_t optionIndex)
{
    OptionData &optionData = m_optionData[optionIndex];
    optionData.m_downloadComplete = true; // everything downloaded
    if (!optionData.m_stillWriting) {
        // not writing anymore -> check status and clear
        checkStatusAndClear(optionIndex);
    } else {
        if (optionData.m_outputDevice && optionData.m_outputDeviceReady) {
            // there's an output device and the download is still writing buffered data to it
            setStatus(DownloadStatus::FinishOuputFile);
        } else {
            // an output device is needed, the download is currently just waiting
            setStatus(DownloadStatus::Waiting);
            if (!optionData.m_requestingNewOutputDevice) {
                // request an output device if not done yet
                optionData.m_requestingNewOutputDevice = true;
                emit outputDeviceRequired(this, optionIndex);
            }
        }
    }
}

/*!
 * \brief Adds a download URL.
 * \param optionName Specifies a name for the option.
 * \param url Specifies the URL to be added.
 * \param redirectionOf Specifies the index of the original URL if the URL is a redirection; provide Network::InvalidOptionIndex otherwise.
 * \return Returns the option index for the new URL.
 *
 * If the specified \a url already exists, no new option will be appended. The old option will just be updated. The index of the updated option
 * is returned in this case.
 *
 * Needs to be called when subclassing.
 */
size_t Download::addDownloadUrl(const QString &optionName, const QUrl &url, size_t redirectionOf)
{
    size_t optionCount = m_optionData.size();
    if (redirectionOf == InvalidOptionIndex || redirectionOf >= optionCount) {
        redirectionOf = optionCount;
    }
    // check if the URL is already present
    for (size_t index = 0; index < optionCount; ++index) {
        OptionData &data = m_optionData[index];
        if (data.m_url == url) {
            // URL has already been added previously -> just update it
            data.m_name = optionName;
            data.m_redirectionOf = redirectionOf;
            goto end;
        }
    }
    // the URL hasn't been added yet
    m_optionData.emplace_back(optionName, url, optionCount, redirectionOf);
    m_availableOptionsChanged = true;
end: // update "m_redirectsTo" of original option
    if (redirectionOf != optionCount) {
        m_optionData.at(redirectionOf).m_redirectsTo = optionCount;
    }
    return optionCount;
}

/*!
 * \brief Changes the download URL with the specified \a optionIndex.
 */
void Download::changeDownloadUrl(size_t optionIndex, const QUrl &value)
{
    m_optionData.at(optionIndex).m_url = value;
    m_availableOptionsChanged = true;
}

/*!
 * \brief Provides an output device.
 * \param optionIndex Specifies the index of the option the output device is provided for.
 * \param device Specifies the output device.
 * \param giveOwnership Specifies whether the ownership is transfered to download.
 *
 * Use this method to provide an output device after the outputDeviceRequired() signal has
 * been emitted.
 * If \a device is nullptr the download will be aborted.
 *
 * In any case: This method does nothing if there is already a ready output device.
 */
void Download::provideOutputDevice(size_t optionIndex, QIODevice *device, bool giveOwnership)
{
    OptionData &optionData = m_optionData[optionIndex];
    optionData.m_requestingNewOutputDevice = false;
    if (!optionData.m_outputDevice || !optionData.m_outputDeviceReady) { // an output device has been requested
        finalizeOutputDevice(optionIndex); // finalize last output device
        if (device) { // a device has been provided
            if (prepareOutputDevice(optionIndex, device, giveOwnership)) { // prepare the output device
                if (optionData.m_outputDeviceReady) { // only proceed if output device is ready
                    optionData.m_hasOutputDeviceOwnership = giveOwnership;
                    if (!writeBufferToOutputDevice(optionIndex)) { // if there's buffered data write it to the provided device
                        return; // error already handled within writeBufferToOutputDevice(), so just return here
                    }
                    optionData.m_stillWriting = false; // not writing anymore
                    if (optionData.m_downloadComplete) { // download has ended, too
                        checkStatusAndClear(optionIndex);
                    }
                }
            } // else: the provided device couldn't be prepared, handled within prepareOutputDevice()
        } else if (isStarted()) { // no device has been provided -> abort the download
            optionData.m_stillWriting = false; // not writing anymore
            abortDownload(); // ensure download is aborted
            optionData.m_buffer.reset();
            reportFinalDownloadStatus(optionIndex, false, tr("No output device provided."));
        }
    }
}

/*!
 * \brief Provides authentication credentials.
 *
 * Use this method when subclassing to supply authentication credentials after the authenticationRequired() signal has been emitted.
 *
 * The credentials will be used for the option with the specified \a optionIndex. If \a optionIndex equals Network::InvalidOptionIndex
 * the credentials will be used for the initialization.
 *
 * Does nothing if the authentication credentials (for the specified purpose) haven't been requested.
 */
void Download::provideAuthenticationCredentials(size_t optionIndex, const AuthenticationCredentials &credentials)
{
    if (optionIndex == InvalidOptionIndex) {
        // credentials are provided for initialization
        if (m_initAuthData.m_requested) {
            m_initAuthData = credentials;
        }
    } else {
        // credentials are provided for specific option
        OptionData &optionData = m_optionData.at(optionIndex);
        if (optionData.m_authData.m_requested) {
            optionData.m_authData = credentials;
            if (!isStarted()) {
                // restart the download
                //range().resetCurrentOffset();
                start();
            }
        }
    }
}

/*!
 * \brief Sets the current progress.
 *
 * This method is only for internal use. When subclassing use reportDownloadProgressUpdate().
 */
void Download::setProgress(qint64 bytesReceived, qint64 bytesToReceive)
{
    if (m_bytesReceived != bytesReceived || m_bytesToReceive != bytesToReceive) {
        if (bytesReceived > m_bytesReceived && m_time.elapsed()) {
            m_speed = (static_cast<double>(bytesReceived - m_bytesReceived) * 0.008) / (static_cast<double>(m_time.restart()) * 0.001);
        }
        m_bytesReceived = bytesReceived;
        m_bytesToReceive = bytesToReceive;
        emit progressChanged(this);
    }
}

/*!
 * \brief Sets whether the download is allowed to overwrite files.
 *
 * The permission is only set for the option with the specified \a optionIndex.
 *
 * The permission state PermissionStatus::Asking can not be set using this method
 * and will be ignored (this state can are only set internally).
 *
 * \sa OptionData::overwritePermission()
 */
void Download::setOverwritePermission(size_t optionIndex, PermissionStatus permission)
{
    OptionData &data = m_optionData.at(optionIndex);
    switch (permission) {
    case PermissionStatus::Unknown:
        data.m_overwritePermission = PermissionStatus::Unknown;
        FALLTHROUGH;
    case PermissionStatus::Asking:
        return; // can not be set here
    default:
        if (data.m_overwritePermission == PermissionStatus::Asking) {
            // the download previously asked for that permission
            data.m_overwritePermission = permission;
            ensureOutputDeviceIsPrepared(optionIndex);
        } else {
            // just keep the permission
            data.m_overwritePermission = permission;
        }
    }
}

/*!
 * \brief Sets whether the download is allowed to append to a existing file.
 *
 * The permission states PermissionStatus::Unknown and PermissionStatus::Asking
 * can not be set using this method and will be ignored (these states can are only set internally).
 *
 * \sa OptionData::appendPermission()
 */
void Download::setAppendPermission(size_t optionIndex, PermissionStatus permission)
{
    OptionData &data = m_optionData.at(optionIndex);
    switch (permission) {
    case PermissionStatus::Unknown:
    case PermissionStatus::Asking:
        return; // can not be set here
    default:
        if (data.m_appendPermission == PermissionStatus::Asking) {
            // the download previously asked for that permission
            data.m_appendPermission = permission;
            ensureOutputDeviceIsPrepared(optionIndex);
        } else {
            // just keep the permission
            data.m_appendPermission = permission;
        }
    }
}

/*!
 * \brief Sets whether the download is allowed to follow redirections.
 *
 * The permission states PermissionStatus::Unknown and PermissionStatus::Asking
 * can not be set using this method and will be ignored (these states can are only set internally).
 *
 * \sa OptionData::redirectPermission()
 */
void Download::setRedirectPermission(size_t originalOptionIndex, PermissionStatus permission)
{
    OptionData &data = m_optionData.at(originalOptionIndex);
    switch (permission) {
    case PermissionStatus::Unknown:
    case PermissionStatus::Asking:
        return; // can not be set here
    default:
        if (data.m_redirectPermission == PermissionStatus::Asking) {
            data.m_redirectPermission = permission;
            // the download previously asked for that permission
            reportRedirectionAvailable(originalOptionIndex);
        } else {
            // just keep the permission
            data.m_redirectPermission = permission;
        }
    }
}

/*!
 * \brief Sets whether the download is allowed to ignore SSL errors.
 *
 * The permission states PermissionStatus::Unknown and PermissionStatus::Asking
 * can not be set using this method and will be ignored (these states can are only set internally).
 *
 * \sa OptionData::ignoreSslErrorsPermission()
 */
void Download::setIgnoreSslErrorsPermission(size_t optionIndex, PermissionStatus permission)
{
    OptionData &data = m_optionData.at(optionIndex);
    switch (permission) {
    case PermissionStatus::Unknown:
    case PermissionStatus::Asking:
        return; // can not be set here
    default:
        if (data.m_ignoreSslErrorsPermission == PermissionStatus::Asking) {
            data.m_ignoreSslErrorsPermission = permission;
            switch (data.m_ignoreSslErrorsPermission) {
            case PermissionStatus::Allowed:
            case PermissionStatus::AlwaysAllowed:
                if (networkError() == QNetworkReply::SslHandshakeFailedError) {
                    start(); // restart the download if it failed because of the SSL error
                }
                break;
            case PermissionStatus::Refused:
            case PermissionStatus::AlwaysRefused:
                usePermission(data.m_ignoreSslErrorsPermission);
                break;
            default:;
            }

        } else {
            // just keep the permission
            data.m_ignoreSslErrorsPermission = permission;
        }
    }
}

// docs for signals and pure virtual methods

/*!
 * \fn Download::statusChanged()
 * \brief Emitted when the status changed.
 */

/*!
 * \fn Download::progressChanged()
 * \brief Emitted when the progress changed, e. g. new bytes have been received.
 *
 * The frequency of emmitation can be adjusted using the Download::setProgressUpdateInterval().
 */

/*!
 * \fn Download::statusInfo()
 * \brief Emitted when the status info changed.
 */

/*!
 * \fn Download::overwriteingPermissionRequired()
 * \brief Emitted when overwriting an existing file needs to be confirmed.
 *
 * The permission might be given or refused using the Download::setOverwritePermission() method.
 * \sa PermissionStatus
 */

/*!
 * \fn Download::appendingPermissionRequired()
 * \brief Emitted when appending data to an existing file needs to be confirmed.
 *
 * The permission might be given or refused using the Download::setAppendPermission() method.
 * \sa PermissionStatus
 */

/*!
 * \fn Download::redirectionPermissonRequired()
 * \brief Emitted when a redirection needs to be confirmed.
 *
 * The permission might be given or refused using the Download::setRedirectPermission() method.
 * \sa PermissionStatus
 */

/*!
 * \fn Download::outputDeviceRequired()
 * \brief Emitted when an output device needs to be provided.
 *
 * The output device needs to be provided using the Download::provideOutputDevice() method.
 */

/*!
 * \fn Download::authenticationRequired()
 * \brief Emitted when authentication credentials needs to be supplied.
 *
 * The credentials needs to be supplied using the Download::provideAuthenticationCredentials() method.
 */

/*!
 * \fn Download::sslErrors()
 * \brief Emitted when SSL errors occur.
 */

/*!
 * \fn Download::typeName()
 * \brief Returns the type of the download as string (e. g. "Youtube Download").
 */

/*!
 * \fn Download::doDownload()
 * \brief Starts the actual download.
 *
 * To be implemented when subclassing.
 */

/*!
 * \fn Download::abortDownload()
 * \brief Aborts the current download.
 *
 * To be implemented when subclassing.
 */

/*!
 * \fn Download::doInit()
 * \brief Initiates the download; called before the actual download.
 *
 * To be implemented when subclassing.
 */

/*!
 * \fn Download::checkStatusAndClear()
 * \brief Checks the status and frees resources after the download has been completed; called
 *        after reportDownloadComplete() has been called.
 * \remarks Should call reportFinalDownloadStatus().
 *
 * To be implemented when subclassing.
 */
}
