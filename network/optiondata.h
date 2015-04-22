#ifndef NETWORK_OPTIONDATA_H
#define NETWORK_OPTIONDATA_H

#include "authenticationcredentials.h"

#include <QString>
#include <QUrl>

#include <memory>
#include <sstream>
#include <limits>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

namespace Network {

enum class PermissionStatus;

constexpr size_t InvalidOptionIndex = std::numeric_limits<decltype(InvalidOptionIndex)>::max();

class OptionData
{
    friend class Download;

public:
    OptionData(const QString &name, const QUrl &url, size_t redirectsTo, size_t redirectionOf);
    ~OptionData();
    OptionData(OptionData &&other) = default;

    const QString &name() const;
    const QUrl &url() const;
    size_t redirectsTo() const;
    size_t redirectionOf() const;
    qint64 bytesWritten() const;
    AuthenticationCredentials &authenticationCredentials();
    const AuthenticationCredentials &authenticationCredentials() const;
    PermissionStatus overwritePermission() const;
    PermissionStatus appendPermission() const;
    PermissionStatus redirectPermission() const;
    PermissionStatus ignoreSslErrorsPermission() const;

private:
    QString m_name;
    QUrl m_url;
    size_t m_redirectsTo;
    size_t m_redirectionOf;
    AuthenticationCredentials m_authData;
    bool m_requestingNewOutputDevice;
    QIODevice *m_outputDevice;
    bool m_outputDeviceReady;
    qint64 m_bytesWritten;
    std::unique_ptr<std::stringstream> m_buffer;
    bool m_stillWriting;
    bool m_downloadComplete;
    bool m_downloadAbortedInternally;
    bool m_hasOutputDeviceOwnership;
    PermissionStatus m_overwritePermission;
    PermissionStatus m_appendPermission;
    PermissionStatus m_redirectPermission;
    PermissionStatus m_ignoreSslErrorsPermission;
    void chuckOutputDevice();
};

/*!
 * \brief Returns the option name.
 */
inline const QString &OptionData::name() const
{
    return m_name;
}

/*!
 * \brief Returns the URL of the option.
 */
inline const QUrl &OptionData::url() const
{
    return m_url;
}

/*!
 * \brief Returns the index of the option this option redirects to.
 *
 * If this option does not redirect to another option, the index
 * of this option is returned.
 */
inline size_t OptionData::redirectsTo() const
{
    return m_redirectsTo;
}

/*!
 * \brief Returns the index of the original option.
 *
 * If this option is no redirection from another option, the index
 * of this option is returned.
 */
inline size_t OptionData::redirectionOf() const
{
    return m_redirectionOf;
}

/*!
 * \brief Returns the number of bytes written to the output device.
 */
inline qint64 OptionData::bytesWritten() const
{
    return m_bytesWritten;
}

/*!
 * \brief Returns the authentication credentials provided for this option.
 * \sa Download::provideAuthenticationCredentials()
 */
inline AuthenticationCredentials &OptionData::authenticationCredentials()
{
    return m_authData;
}

/*!
 * \brief Returns the authentication credentials provided for this option.
 * \sa Download::provideAuthenticationCredentials()
 */
inline const AuthenticationCredentials &OptionData::authenticationCredentials() const
{
    return m_authData;
}

/*!
 * \brief Returns the "overwrite" permission for this option.
 * \sa Download::setOverwritePermission()
 */
inline PermissionStatus OptionData::overwritePermission() const
{
    return m_overwritePermission;
}

/*!
 * \brief Returns the "append" permission for this option.
 * \sa Download::setAppendPermission()
 */
inline PermissionStatus OptionData::appendPermission() const
{
    return m_appendPermission;
}

/*!
 * \brief Returns the "redirect" permission for this option.
 * \sa Download::setRedirectPermission()
 */
inline PermissionStatus OptionData::redirectPermission() const
{
    return m_redirectPermission;
}

/*!
 * \brief Returns the "ignore SSL errors" permission for this option.
 * \sa Download::setIgnoreSslErrorsPermission()
 */
inline PermissionStatus OptionData::ignoreSslErrorsPermission() const
{
    return m_ignoreSslErrorsPermission;
}

} // namespace Network

#endif // NETWORK_OPTIONDATA_H
