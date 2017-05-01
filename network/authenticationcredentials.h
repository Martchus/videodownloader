#ifndef AUTHENTICATIONCREDENTIALS
#define AUTHENTICATIONCREDENTIALS

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

namespace Network {

class Download;

class AuthenticationCredentials {
    friend class Download;

public:
    AuthenticationCredentials();
    AuthenticationCredentials(const QString &userName, const QString &password);

    const QString &userName() const;
    const QString &password() const;
    const QList<QMap<QString, QVariant> > options() const;
    bool isIncomplete() const;
    void clear();

private:
    bool m_requested;
    QString m_userName;
    QString m_password;
    QList<QMap<QString, QVariant> > m_options;
};

inline AuthenticationCredentials::AuthenticationCredentials()
    : m_requested(false)
{
}

inline AuthenticationCredentials::AuthenticationCredentials(const QString &userName, const QString &password)
    : m_requested(false)
    , m_userName(userName)
    , m_password(password)
{
}

inline const QString &AuthenticationCredentials::userName() const
{
    return m_userName;
}

inline const QString &AuthenticationCredentials::password() const
{
    return m_password;
}

inline const QList<QMap<QString, QVariant> > AuthenticationCredentials::options() const
{
    return m_options;
}

inline bool AuthenticationCredentials::isIncomplete() const
{
    return m_userName.isEmpty() || m_password.isEmpty();
}

inline void AuthenticationCredentials::clear()
{
    m_userName.clear();
    m_password.clear();
    m_options.clear();
}
}

#endif // AUTHENTICATIONCREDENTIALS
