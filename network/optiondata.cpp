#include "optiondata.h"
#include "permissionstatus.h"

#include <QIODevice>

namespace Network {

/*!
 * \class OptionData
 * \brief The OptionData class wraps download option specific information.
 */

/*!
 * \brief Constructs a new option data object.
 */
OptionData::OptionData(const QString &name, const QUrl &url, size_t redirectsTo, size_t redirectionOf) :
    m_name(name),
    m_url(url),
    m_redirectsTo(redirectsTo),
    m_redirectionOf(redirectionOf),
    m_requestingNewOutputDevice(false),
    m_outputDevice(nullptr),
    m_outputDeviceReady(false),
    m_bytesWritten(0),
    m_stillWriting(false),
    m_downloadComplete(false),
    m_downloadAbortedInternally(false),
    m_hasOutputDeviceOwnership(false),
    m_overwritePermission(PermissionStatus::Unknown),
    m_appendPermission(PermissionStatus::Unknown),
    m_redirectPermission(PermissionStatus::Unknown),
    m_ignoreSslErrorsPermission(PermissionStatus::Unknown)
{}

/*!
 * \brief Destroys the assigned output device if ownership has been given, detaches the assigned
 *        output device if ownership has not been given.
 */
void OptionData::chuckOutputDevice()
{
    if(m_outputDevice && m_hasOutputDeviceOwnership) {
        delete m_outputDevice;
    }
    m_outputDevice = nullptr;
    m_hasOutputDeviceOwnership = false;
}

/*!
 * \brief Destroys the option data object. Also calls destroyDevice().
 */
OptionData::~OptionData()
{
    chuckOutputDevice();
}

} // namespace Network

