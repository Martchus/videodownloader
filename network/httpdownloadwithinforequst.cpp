#include "./httpdownloadwithinforequst.h"
#include "./permissionstatus.h"

namespace Network {

/*!
 * \class HttpDownloadWithInfoRequst
 * \brief The HttpDownloadWithInfoRequst class a base class for HTTP downloads which need to perform
 *        additional requests before the actual download.
 *
 * A YoutubeDownload needs to retrieve the download URL for example. These requests are handled by
 * the HttpDownloadWithInfoRequst class during initialization.
 */

/*!
 * \brief Constructs a new YoutubeDownload for the specified \a url.
 */
HttpDownloadWithInfoRequst::HttpDownloadWithInfoRequst(const QUrl &url, QObject *parent) :
    HttpDownload(url, parent)
{}

HttpDownloadWithInfoRequst::~HttpDownloadWithInfoRequst()
{}

void HttpDownloadWithInfoRequst::doInit()
{
    setTitleFromFilename(initialUrl().host());
    bool success;
    QString reasonForFail;
    // get the info request
    m_infoDownload.reset(infoRequestDownload(success, reasonForFail));
    if(success) {
        // the request could be constructed successfully
        if(!m_infoDownload) {
            // no request needed (at this time), just call evalVideoInformation()
            evalVideoInformation(nullptr, nullptr);
        } else {
            // setup and initiate the request
            m_infoDownload->setParent(parent());
            m_infoDownload->setDefaultUserAgentUsed(isDefaultUserAgentUsed());
            m_infoDownload->setCustomUserAgent(userAgent());
            m_infoDownload->setProxy(proxy());
            connect(m_infoDownload.get(), &Download::statusChanged, this, &HttpDownloadWithInfoRequst::infoRequestChangedStatus);
            m_infoDownload->init();
        }
    } else {
        reportInitiated(false, reasonForFail);
    }
}

void HttpDownloadWithInfoRequst::abortDownload()
{
    if(m_infoDownload) {
        m_infoDownload->stop();
    }
    HttpDownload::abortDownload();
}

/*!
 * \brief Handles the status of the info request.
 */
void HttpDownloadWithInfoRequst::infoRequestChangedStatus(Download *download)
{
    switch(download->status()) {
    case DownloadStatus::Failed:
        reportInitiated(false, tr("Couldn't retieve the video information. %1").arg(statusInfo()));
        break;
    case DownloadStatus::Ready:
        if(m_infoDownload->isValidOptionChosen()) {
            m_infoDownload->setRedirectPermission(m_infoDownload->chosenOption(), PermissionStatus::AlwaysAllowed);
            m_infoBuffer.reset(new QBuffer());
            if(m_infoBuffer->open(QIODevice::ReadWrite)) {
                download->start(m_infoBuffer.get());
            } else {
                reportInitiated(false, tr("Couldn't initialize buffer to store initialization data."));
            }
        } else {
            reportInitiated(false, tr("The initialization request has no options."));
        }
        break;
    case DownloadStatus::Finished:
        if(!m_infoBuffer) {
            reportInitiated(false, tr("The initialization data buffer hasn't been initialized."));
        } else {
            m_infoBuffer->seek(0);
            evalVideoInformation(download, m_infoBuffer.get());
            m_infoDownload.reset();
        }
        break;
    default:
        ;
    }
}

}
