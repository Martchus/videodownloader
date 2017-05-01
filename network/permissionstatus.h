#ifndef PERMISSIONSTATUS
#define PERMISSIONSTATUS

namespace Network {

/*!
 * \brief Specifies whether an action is allowed.
 */
enum class PermissionStatus {
    Unknown, /**< The download will ask for the permission if required by emitting the corresponding signal. */
    Asking, /**< The download is asking for the permission. The corresponding signal has been emitted. */
    Allowed, /**< The download is allowed to perform the action once but the status will turn to Unknown when doing the action. */
    Refused, /**< The download is not allowed to perform the action and the status will turn to Unknown when refusing the action. */
    AlwaysAllowed, /**< The download is allowed to perform the action always. */
    AlwaysRefused /**< The download is not allowed to perform the action. */
};

/*!
 * \brief Sets the specified \a permission to PermissionStatus::Unknown if the permission is only to be used once an not always.
 */
inline void usePermission(PermissionStatus &permission)
{
    switch (permission) {
    case PermissionStatus::Allowed:
    case PermissionStatus::Refused:
        permission = PermissionStatus::Unknown;
        break;
    default:;
    }
}
}

#endif // PERMISSIONSTATUS
