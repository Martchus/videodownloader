#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "./downloadrange.h"
#include "./optiondata.h"

#include <c++utilities/chrono/timespan.h>

#include <QAuthenticator>
#include <QBuffer>
#include <QFile>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QObject>
#include <QTime>

#include <tuple>

namespace Network {

/*!
 * \brief Specifies the download status.
 */
enum class DownloadStatus {
    None, /**< The download has just been created. Call init() to initialize the download. */
    Initiating, /**< The download is currently initiating after init() has been called. */
    Waiting, /**< The download is currently waiting for a permission or the output device to be set. */
    Ready, /**< The download is initiated. It may be started using the start() method. */
    Downloading, /**< The download is actually downloading. */
    FinishOuputFile, /**< The download is not downloading anymore but still writing data from the buffer to the output file. */
    Interrupting, /**< The is being interrupted after interrupt() has been called. */
    Aborting, /**< The is being aborted after abort() has been called. */
    Interrupted, /**< The download is interrupted after interrupt() has been called. */
    Failed, /**< The download failed. The download might enter this status after the initialization has failed or the actual download has failed or has been aborted. */
    Finished /**< The download has been finished and all received data has been written to the output file. */
};

class Download : public QObject {
    Q_OBJECT
public:
    virtual ~Download();

public slots:
    // public slots to control the download
    void init();
    void start();
    void start(const QString &targetPath);
    void start(QIODevice *targetDevice, bool giveOwnership = false);
    void stop();
    void interrupt();

public:
    // public methods
    //  to get status information
    bool canStart(QString &reasonIfNot);
    DownloadStatus status() const;
    DownloadStatus lastStatus() const;
    int progressPercentage() const;
    double speed() const;
    double shiftSpeed();
    CppUtilities::TimeSpan remainingTime() const;
    CppUtilities::TimeSpan shiftRemainingTime();
    qint64 bytesReceived() const;
    qint64 newBytesReceived();
    qint64 bytesToReceive() const;
    qint64 newBytesToReceive();
    int lastProgressUpdate() const;
    bool hasStatusInfo() const;
    const QString &statusInfo() const;
    QNetworkReply::NetworkError networkError() const;
    bool isStarted() const;
    bool isWorking() const;
    bool isDownloading() const;
    bool isInitiated() const;

    // to get and set meta data and additional information
    const QString &targetPath() const;
    void setTargetPath(const QString &targetPath);
    const QUrl &initialUrl() const;
    const QUrl &downloadUrl() const;
    const QUrl &downloadUrl(size_t optionIndex) const;
    const std::vector<OptionData> &options() const;
    std::vector<OptionData> &options();
    const QString &id() const;
    const QString &title() const;
    const QString &collectionName() const;
    size_t availableOptionCount() const;
    bool areOptionsAvailable() const;
    const QString &optionName(size_t optionIndex) const;
    bool haveAvailableOptionsChanged();
    bool isValidOptionChosen() const;
    size_t chosenOption() const;
    const QString &chosenOptionName() const;
    bool setChosenOption(size_t optionIndex);
    bool hasSelectedOptionChanged();
    const QString &userAgent();
    void setCustomUserAgent(const QString &value);
    bool isDefaultUserAgentUsed() const;
    void setDefaultUserAgentUsed(bool defaultUserAgentUsed);
    static const QString &defaultUserAgent();
    static void scrambleDefaultUserAgent();
    const QNetworkProxy &proxy() const;
    void setProxy(const QNetworkProxy &value);
    const QString &uploader() const;
    int views() const;
    CppUtilities::TimeSpan duration() const;
    const QString &rating() const;
    int positionInCollection() const;
    int progressUpdateInterval() const;
    void setProgressUpdateInterval(int value);
    virtual QString suitableFilename() const;
    DownloadRange &range();
    bool setRange(const DownloadRange &value);
    virtual bool supportsRange() const;
    virtual QString typeName() const = 0;
    void provideMetaData(const QString &title = QString(), const QString &uploader = QString(),
        CppUtilities::TimeSpan duration = CppUtilities::TimeSpan(), const QString &collectionName = QString(), int positionInCollection = 0,
        int views = 0, const QString &rating = QString());
    void setOverwritePermission(size_t optionIndex, PermissionStatus permission);
    void setAppendPermission(size_t optionIndex, PermissionStatus permission);
    void setRedirectPermission(size_t originalOptionIndex, PermissionStatus permission);
    void setIgnoreSslErrorsPermission(size_t optionIndex, PermissionStatus permission);
    void provideOutputDevice(size_t optionIndex, QIODevice *device, bool giveOwnership = false);
    void provideAuthenticationCredentials(size_t optionIndex, const AuthenticationCredentials &credentials);
    const AuthenticationCredentials &initialAuthenticationCredentials() const;
    AuthenticationCredentials &initialAuthenticationCredentials();
    virtual bool isInitiatingInstantlyRecommendable() const;

    // to construct new downloads
    static Download *fromUrl(const QUrl &url);

signals:
    void statusChanged(Download *download);
    void progressChanged(Download *download);
    void statusInfoChanged(Download *download);
    void overwriteingPermissionRequired(Download *download, size_t optionIndex, const QString &file);
    void appendingPermissionRequired(Download *download, size_t optionIndex, const QString &file, quint64 offset, quint64 fileSize);
    void redirectionPermissonRequired(Download *download, size_t optionIndex, int redirectionOptionIndex);
    void outputDeviceRequired(Download *concerningDownload, size_t optionIndex);
    void authenticationRequired(Download *concerningDownload, size_t optionIndex, const QString &statusInfo);
    void sslErrors(Download *concerningDownload, size_t optionIndex, const QList<QSslError> &sslErrors);

protected:
    // c'tor
    explicit Download(const QUrl &url = QUrl(), QObject *parent = nullptr);

    // protected methods
    //  to be implemented by derived classes
    virtual void doDownload() = 0;
    virtual bool followRedirection(size_t originalOptionIndex);
    virtual void abortDownload() = 0;
    virtual void doInit() = 0;
    virtual void checkStatusAndClear(size_t optionIndex) = 0;
    //  meant to be called by derived classes
    size_t addDownloadUrl(const QString &optionName, const QUrl &url, size_t redirectionOf = InvalidOptionIndex);
    void changeDownloadUrl(size_t optionIndex, const QUrl &value);
    void reportInitiated(
        bool success, const QString &reasonIfNot = QString(), const QNetworkReply::NetworkError &networkError = QNetworkReply::NoError);
    void reportFinalDownloadStatus(size_t optionIndex, bool success, const QString &statusDescription = QString(),
        QNetworkReply::NetworkError networkError = QNetworkReply::NoError);
protected slots:
    void reportDownloadInterrupted(size_t optionIndex);
    void reportNewDataToBeWritten(size_t optionIndex, QIODevice *inputDevice);
    void reportRedirectionAvailable(size_t originalOptionIndex);
    void reportAuthenticationRequired(size_t optionIndex, const QString &realm);
    void reportSslErrors(size_t optionIndex, QNetworkReply *reply, const QList<QSslError> &sslErrors);
    void reportDownloadComplete(size_t optionIndex);
    void finalizeOutputDevice(size_t optionIndex);
    void setId(const QString &value);
    void setTitle(const QString &value);
    void setTitleFromFilename(const QString &valueObtainedFromFilename);
    void setUploader(const QString &value);
    void setViews(int value);
    void setDuration(CppUtilities::TimeSpan value);
    void setRating(const QString &value);
    void setPositionInCollection(int value);
    void setCollectionName(const QString &value);
    void reportDownloadProgressUpdate(size_t optionIndex, qint64 bytesReceived, qint64 bytesToReceive);

private:
    // private static methods
    static const QUrl &emptyUrl();
    static const QString &emptyString();

    // private methods
    //  to handle the output device
    bool writeBufferToOutputDevice(size_t optionIndex);
    bool prepareOutputDevice(size_t optionIndex, QIODevice *device, bool takeOwnership);
    void ensureOutputDeviceIsPrepared(size_t optionIndex);
    //  to set status information
    void setBytesWritten(qint64 value);
    void setProgress(qint64 m_bytesReceived = -1, qint64 m_bytesToReceive = -1);
    void setStatusInfo(const QString &value);
    void setStatus(DownloadStatus value);
    void setNetworkError(QNetworkReply::NetworkError value);

    // private fields

    //  concerning urls and meta information
    QUrl m_initialUrl;
    QString m_id;
    QString m_title;
    bool m_dontAcceptNewTitleFromFilenameAnymore;
    QString m_uploader;
    QString m_collectionName;
    int m_views;
    CppUtilities::TimeSpan m_duration;
    QString m_rating;
    int m_positionInCollection;
    AuthenticationCredentials m_initAuthData;

    //  concerning available options
    std::vector<OptionData> m_optionData;
    size_t m_chosenOption;
    bool m_selectedOptionChanged;
    bool m_availableOptionsChanged;

    //  concerning status
    DownloadStatus m_status;
    DownloadStatus m_lastState;
    qint64 m_bytesReceived;
    qint64 m_bytesToReceive;
    qint64 m_newBytesReceived;
    qint64 m_newBytesToReceive;
    double m_speed;
    double m_shiftSpeed;
    CppUtilities::TimeSpan m_shiftRemainingTime;
    QString m_statusInfo;
    QTime m_time;
    QNetworkReply::NetworkError m_networkError;
    bool m_initiated;
    int m_progressUpdateInterval;

    //  concerning download
    bool m_useDefaultUserAgent;
    QString m_userAgent;
    static int m_defaultUserAgent;
    QNetworkProxy m_proxy;
    QString m_targetPath;
    DownloadRange m_range;
};

/*!
 * \brief Returns an empty QUrl reference.
 */
inline const QUrl &Download::emptyUrl()
{
    static const QUrl empty;
    return empty;
}

/*!
 * \brief Returns an empty QString reference.
 */
inline const QString &Download::emptyString()
{
    static const QString empty;
    return empty;
}

/*!
 * \brief Returns the initial URL.
 *
 * (e. g. "https://www.youtube.com/watch?v=eOCzuVrMP28")
 */
inline const QUrl &Download::initialUrl() const
{
    return m_initialUrl;
}

/*!
 * \brief Returns the download URL for the chosen option (which might differ from the initial URL).
 * \remarks
 *          - The download URL is possibly not available for all kinds of downloads.
 *          - Some downloads might use additional information (for instance if the POST method is used).
 *          - This information is possibly not available before the download is initiated.
 */
inline const QUrl &Download::downloadUrl() const
{
    return isValidOptionChosen() ? m_optionData.at(m_chosenOption).m_url : emptyUrl();
}

/*!
 * \brief Returns the download URL for the option with the specified \a index (which might differ from the initial URL).
 * \remarks
 *          - The download URL is possibly not available for all kinds of downloads.
 *          - Some downloads might use additional information (for instance if the POST method is used).
 *          - This information is possibly not available before the download is initiated.
 */
inline const QUrl &Download::downloadUrl(size_t optionIndex) const
{
    return m_optionData.at(optionIndex).m_url;
}

/*!
 * \brief Returns all available download URLs.
 * \remarks For a more detailed description see downloadUrl().
 * \sa downloadUrl()
 */
inline const std::vector<OptionData> &Download::options() const
{
    return m_optionData;
}

/*!
 * \brief Returns all available download URLs.
 * \remarks For a more detailed description see downloadUrl().
 * \sa downloadUrl()
 */
inline std::vector<OptionData> &Download::options()
{
    return m_optionData;
}

/*!
 * \brief Returns the download ID if available.
 * \remarks This information is possibly not available before the download is initiated.
 * (e. g. "eOCzuVrMP28" for the YouTube video "https://www.youtube.com/watch?v=eOCzuVrMP28")
 */
inline const QString &Download::id() const
{
    return m_id;
}

/*!
 * \brief Returns the title of the download if available.
 * \remarks This information is possibly not available before the download is initiated.
 */
inline const QString &Download::title() const
{
    return m_title;
}

/*!
 * \brief Returns the name of the collection the download is part of if available.
 *
 * This is the playlist name for YouTube videos for example.
 *
 * \remarks This information is possibly not available before the download is initiated.
 */
inline const QString &Download::collectionName() const
{
    return m_collectionName;
}

/*!
 * \brief Returns the number of available options.
 * \remarks This information is possibly not available before the download is initiated.
 * \sa availableOption()
 */
inline size_t Download::availableOptionCount() const
{
    return m_optionData.size();
}

/*!
 * \brief Returns an indication whether options are available.
 *
 * If no options are available this means that no way to download the requested data could be determined. The download
 * can not be started in this case.
 *
 * \remarks This information is possibly not available before the download is initiated.
 * \sa availableOption()
 */
inline bool Download::areOptionsAvailable() const
{
    return !m_optionData.empty();
}

/*!
 * \brief Returns the name of the option with the specified \a index.
 * \remarks This information is possibly not available before the download is initiated.
 * \sa availableOptionCount()
 */
inline const QString &Download::optionName(size_t optionIndex) const
{
    if (optionIndex < m_optionData.size()) {
        return m_optionData.at(optionIndex).m_name;
    } else {
        return emptyString();
    }
}

/*!
 * \brief Returns whether the available options have been changed since the last call of
 *        this method.
 */
inline bool Download::haveAvailableOptionsChanged()
{
    bool res = m_availableOptionsChanged;
    m_availableOptionsChanged = false;
    return res;
}

/*!
 * \brief Returns an indication whether a valid option has been chosen.
 */
inline bool Download::isValidOptionChosen() const
{
    return availableOptionCount() > 0 && m_chosenOption < availableOptionCount();
}

/*!
 * \brief Returns the index of the chosen option.
 *
 * The index might be invalid. Check isValidOptionChosen() before.
 */
inline size_t Download::chosenOption() const
{
    return m_chosenOption;
}

/*!
 * \brief Returns the name of the chosen option if a valid option has been chosen.
 */
inline const QString &Download::chosenOptionName() const
{
    return optionName(m_chosenOption);
}

/*!
 * \brief Returns an indication whether the selected option has been changed since the last
 *        call of this method.
 */
inline bool Download::hasSelectedOptionChanged()
{
    bool res = m_selectedOptionChanged;
    m_selectedOptionChanged = false;
    return res;
}

/*!
 * \brief Returns the user agent which is used for HTTP downloads or other downloads which allow sending such information.
 *
 * A custom user agent might be assigned using the setCustomUserAgent() method. If no custom user agent is assigned a
 * randomly picked default user agent will be used. This behaviour can be disabled/enabled using the setDefaultUserAgentUsed()
 * method.
 */
inline const QString &Download::userAgent()
{
    if (m_userAgent.isEmpty() && isDefaultUserAgentUsed()) {
        return defaultUserAgent();
    }
    return m_userAgent;
}

/*!
 * \brief Sets a custom user agent used for HTTP downloads or other downloads which allow sending such information.
 * \sa userAgent()
 */
inline void Download::setCustomUserAgent(const QString &value)
{
    m_userAgent = value;
}

/*!
 * \brief Returns whether a default user agent is used when no custom user agent is assigned.
 * \sa userAgent()
 */
inline bool Download::isDefaultUserAgentUsed() const
{
    return m_useDefaultUserAgent;
}

/*!
 * \brief Sets whether a default user agent is used when no custom user agent is assigned.
 * \sa userAgent()
 */
inline void Download::setDefaultUserAgentUsed(bool defaultUserAgentUsed)
{
    m_useDefaultUserAgent = defaultUserAgentUsed;
}

/*!
 * \brief Returns the proxy.
 */
inline const QNetworkProxy &Download::proxy() const
{
    return m_proxy;
}

/*!
 * \brief Sets the proxy.
 * \remarks Setting the proxy when downloading does not affect the current download. You need to
 *          interrupt and restart the download in this case.
 */
inline void Download::setProxy(const QNetworkProxy &value)
{
    m_proxy = value;
}

/*!
 * \brief Returns the name of the uploader if available.
 * \remarks This information is possibly not available before the download is initiated.
 */
inline const QString &Download::uploader() const
{
    return m_uploader;
}

/*!
 * \brief Returns the number of views for visual content if available.
 * \remarks This information is possibly not available before the download is initiated.
 */
inline int Download::views() const
{
    return m_views;
}

/*!
 * \brief Returns the duration if available.
 * \remarks This information is possibly not available before the download is initiated.
 */
inline CppUtilities::TimeSpan Download::duration() const
{
    return m_duration;
}

/*!
 * \brief Returns the rating if available.
 * \remarks This information is possibly not available before the download is initiated.
 */
inline const QString &Download::rating() const
{
    return m_rating;
}

/*!
 * \brief Returns the position of the download in the collection the download is part of.
 *
 * (e. g. the track number for Grooveshark downloads)
 *
 * \remarks This information is possibly not available before the download is initiated.
 */
inline int Download::positionInCollection() const
{
    return m_positionInCollection;
}

/*!
 * \brief Returns the status update interval in miliseconds.
 */
inline int Download::progressUpdateInterval() const
{
    return m_progressUpdateInterval;
}

/*!
 * \brief Sets the status interval.
 * \param value Specifies the status inverval in miliseconds.
 */
inline void Download::setProgressUpdateInterval(int value)
{
    m_progressUpdateInterval = value;
}

/*!
 * \brief Returns the range.
 * \sa DownloadRange
 */
inline DownloadRange &Download::range()
{
    return m_range;
}

/*!
 * \brief Sets the download range.
 *
 * The range can only be set if this feature is supported.
 * \sa supportsRange()
 *
 * \remarks Setting the range when downloading does not affect the current download. You need to
 *          abort and restart the download in this case.
 *
 * \returns Returns whether the range could be set.
 */
inline bool Download::setRange(const DownloadRange &value)
{
    if (supportsRange()) {
        m_range = value;
        return true;
    }
    return false;
}

/*!
 * \brief Returns whether a range can be set.
 */
inline bool Download::supportsRange() const
{
    return false;
}

/*!
 * \brief Returns whether the download has been started.
 *
 * Returns false if the download has ended because the download might
 * be restarted then.
 */
inline bool Download::isStarted() const
{
    switch (status()) {
    case DownloadStatus::Downloading:
    case DownloadStatus::Waiting:
    case DownloadStatus::FinishOuputFile:
    case DownloadStatus::Interrupting:
    case DownloadStatus::Aborting:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns whether the download is actually doing something.
 */
inline bool Download::isWorking() const
{
    switch (status()) {
    case DownloadStatus::Downloading:
    case DownloadStatus::FinishOuputFile:
    case DownloadStatus::Interrupting:
    case DownloadStatus::Aborting:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Returns whether the download is actually downloading something.
 */
inline bool Download::isDownloading() const
{
    switch (status()) {
    case DownloadStatus::Downloading:
        return true;
    default:
        return false;
    }
}

/*!
 * \brief Sets the ID.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa id()
 */
inline void Download::setId(const QString &value)
{
    m_id = value;
}

/*!
 * \brief Sets the title.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa title()
 */
inline void Download::setTitle(const QString &value)
{
    m_title = value;
    m_dontAcceptNewTitleFromFilenameAnymore = true;
}

/*!
 * \brief Sets the title.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * The difference between this method and setTitle() is that this method
 * does nothing if a title has been set previously using the setTitle() method.
 * This might be useful since title information which is only derived from a
 * filename should generally not overwrite a title which has been obtained previously
 * from a better souce (for example the video meta data returned by YouTube in
 * case of a YouTube download).
 *
 * \sa setTitle()
 * \sa title()
 */
inline void Download::setTitleFromFilename(const QString &valueObtainedFromFilename)
{
    if (!m_dontAcceptNewTitleFromFilenameAnymore) {
        m_title = valueObtainedFromFilename;
    }
}

/*!
 * \brief Sets the uploader.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa uploader().
 */
inline void Download::setUploader(const QString &value)
{
    m_uploader = value;
}

/*!
 * \brief Sets the uploader.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa uploader().
 */
inline void Download::setViews(int value)
{
    m_views = value;
}

/*!
 * \brief Sets the duration.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa duration().
 */
inline void Download::setDuration(CppUtilities::TimeSpan value)
{
    m_duration = value;
}

/*!
 * \brief Sets the rating.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa rating().
 */
inline void Download::setRating(const QString &value)
{
    m_rating = value;
}

/*!
 * \brief Sets the position of the download in the collection the download is part of.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa positionInCollection().
 */
inline void Download::setPositionInCollection(int value)
{
    m_positionInCollection = value;
}

/*!
 * \brief Sets name of the collection the download is part of.
 *
 * Might be called when subclassing. To provide meta data from an external source use provideMetaData().
 *
 * \sa collectionName().
 */
inline void Download::setCollectionName(const QString &value)
{
    m_collectionName = value;
}

/*!
 * \brief Returns an indication whether the download is initiated.
 *
 * \remarks A download which has been failed might be initiated or not - depending
 *          whether the failure occured during initialization or during the acutal download.
 */
inline bool Download::isInitiated() const
{
    return m_initiated;
}

/*!
 * \brief Returns the target path. The download will store downloaded data in that file.
 *
 * The target path might be set when calling the start() method
 * or using the setTargetPath() method.
 *
 * \sa setTargetPath()
 */
inline const QString &Download::targetPath() const
{
    return m_targetPath;
}

/*!
 * \brief Sets the target path.
 * \remarks The target path is ignored if there is already an output device present.
 * \sa targetPath()
 */
inline void Download::setTargetPath(const QString &targetPath)
{
    m_targetPath = targetPath;
}

/*!
 * \brief Returns the current status.
 * \sa DownloadStatus
 */
inline DownloadStatus Download::status() const
{
    return m_status;
}

/*!
 * \brief Sets the current status.
 *
 * This method is only intented for internal use. Use the methods start(), stop(), abort(), ... to
 * control the status of the download. When subclassing use the report methods.
 */
inline void Download::setStatus(DownloadStatus value)
{
    if (value != m_status) {
        m_lastState = m_status;
        m_status = value;
        emit statusChanged(this);
    }
}

/*!
 * \brief Returns the last status.
 * \sa status()
 */
inline DownloadStatus Download::lastStatus() const
{
    return m_lastState;
}

/*!
 * \brief Returns the progress percentage if available; otherwise -1.
 */
inline int Download::progressPercentage() const
{
    return (m_bytesReceived > 0 && m_bytesToReceive > 0) ? (static_cast<double>(m_bytesReceived) / static_cast<double>(m_bytesToReceive) * 100.0)
                                                         : -1;
}

/*!
 * \brief Returns the current speed if available; otherwise 0.
 */
inline double Download::speed() const
{
    return m_status == DownloadStatus::Downloading ? m_speed : 0.0;
}

/*!
 * \brief Returns the alteration of the speed since the last call of this method.
 * \sa speed()
 */
inline double Download::shiftSpeed()
{
    double absolute = speed();
    double shift = absolute - m_shiftSpeed;
    m_shiftSpeed = absolute;
    return shift;
}

/*!
 * \brief Returns the estimated remaining time.
 */
inline CppUtilities::TimeSpan Download::remainingTime() const
{
    return (m_status == DownloadStatus::Downloading && m_bytesToReceive != -1)
        ? CppUtilities::TimeSpan::fromSeconds(static_cast<double>(m_bytesToReceive) / (m_speed * 125.0))
        : CppUtilities::TimeSpan();
}

/*!
 * \brief Returns the alteration of the estimated remaining time since the last call of this method.
 * \sa remainingTime()
 */
inline CppUtilities::TimeSpan Download::shiftRemainingTime()
{
    CppUtilities::TimeSpan absolute = remainingTime();
    CppUtilities::TimeSpan shift = absolute - m_shiftRemainingTime;
    m_shiftRemainingTime = absolute;
    return shift;
}

/*!
 * \brief Returns the number of bytes received since the downlod has been started.
 */
inline qint64 Download::bytesReceived() const
{
    return m_bytesReceived;
}

/*!
 * \brief Returns the number of bytes received since the last call of this method or
 *        if the method hasn't been called yet since the download has been started.
 * \sa bytesReceived()
 */
inline qint64 Download::newBytesReceived()
{
    qint64 shift = m_bytesReceived - m_newBytesReceived;
    m_newBytesReceived = m_bytesReceived;
    return shift;
}

/*!
 * \brief Returns the total number of bytes to receive.
 */
inline qint64 Download::bytesToReceive() const
{
    return m_bytesToReceive;
}

/*!
 * \brief Returns the alteration of the total number of bytes to receive.
 * \sa bytesToReceive()
 */
inline qint64 Download::newBytesToReceive()
{
    qint64 shift = m_bytesToReceive - m_newBytesToReceive;
    m_newBytesToReceive = m_bytesToReceive;
    return shift;
}

/*!
 * \brief Returns the number of miliseconds since the last progress update.
 */
inline int Download::lastProgressUpdate() const
{
    return m_time.elapsed();
}

/*!
 * \brief Returns an indication whether a status information is available.
 *
 * A status information is usually available if the download failed. In this case
 * the status info contains the reason for the fail.
 */
inline bool Download::hasStatusInfo() const
{
    return !m_statusInfo.isEmpty();
}

/*!
 * \brief Returns the status info.
 * \sa hasStatusInfo()
 */
inline const QString &Download::statusInfo() const
{
    return m_statusInfo;
}

/*!
 * \brief Sets the status info.
 *
 * This method is only intented for internal use. Use the report methods when subclassing.
 */
inline void Download::setStatusInfo(const QString &value)
{
    if (m_statusInfo != value) {
        m_statusInfo = value;
        emit statusInfoChanged(this);
    }
}

/*!
 * \brief Returns what kind of network error occured.
 *
 * If no network error occured QNetworkReply::NoError is returned.
 */
inline QNetworkReply::NetworkError Download::networkError() const
{
    return m_networkError;
}

/*!
 * \brief Sets the status info.
 *
 * This method is only intented for internal use. Use the report methods when subclassing.
 */
inline void Download::setNetworkError(QNetworkReply::NetworkError value)
{
    m_networkError = value;
}

/*!
 * \brief Returns the authentication credentials provided for initialization using provideAuthenticationCredentials().
 */
inline const AuthenticationCredentials &Download::initialAuthenticationCredentials() const
{
    return m_initAuthData;
}

/*!
 * \brief Returns the authentication credentials provided for initialization using provideAuthenticationCredentials().
 */
inline AuthenticationCredentials &Download::initialAuthenticationCredentials()
{
    return m_initAuthData;
}
} // namespace Network

#endif // DOWNLOAD_H
