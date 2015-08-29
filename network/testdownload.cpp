#include "testdownload.h"

namespace Network {

/*!
 * \class Network::TestDownload
 * \brief The TestDownload class is only used for testing purposes.
 *
 * This class is only available if qmake is called with "CONFIG+=testdownload".
 */

/*!
 * \brief Constructs a new test download.
 */
TestDownload::TestDownload()
{
    setTitle(QStringLiteral("Testdownload %1").arg(rand()));
    setUploader(QStringLiteral("someone"));
    m_timer.setInterval(500);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &TestDownload::tick);
}

/*!
 * \brief Destroys the test download.
 */
TestDownload::~TestDownload()
{}

QString TestDownload::typeName() const
{
    return QStringLiteral("Test");
}

bool TestDownload::isInitiatingInstantlyRecommendable() const
{
    return true;
}

void TestDownload::doDownload()
{
    reportDownloadProgressUpdate(chosenOption(), 0, 50000);
    m_timer.start();
}

void TestDownload::abortDownload()
{
    m_timer.stop();
    reportFinalDownloadStatus(chosenOption(), false, QStringLiteral("aborted"), QNetworkReply::OperationCanceledError);
}

void TestDownload::doInit()
{
    addDownloadUrl(QStringLiteral("Test 1"), QUrl(QStringLiteral("foo://bar")));
    reportInitiated(true);
}

void TestDownload::checkStatusAndClear(size_t optionIndex)
{
    reportFinalDownloadStatus(optionIndex, true, QStringLiteral("test completed"));
}

void TestDownload::tick()
{
    if(bytesReceived() < 5000) {
        reportDownloadProgressUpdate(chosenOption(), bytesReceived() + 300, 5000);
    } else {
        reportDownloadComplete(0);
    }
}

}
