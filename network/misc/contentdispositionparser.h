#ifndef CONTENTDISPOSITIONPHARSER_H
#define CONTENTDISPOSITIONPHARSER_H

#include <QMap>
#include <QString>

namespace Network {

class ContentDispositionParser {
public:
    ContentDispositionParser(const QString &contentDisposition);
    void pharse();
    const QString &fileName();
    bool isAttachment();
    const QMap<QString, QString> &data();

private:
    /*!
     * \brief Specifies the parsing position.
     */
    enum PharsingPosition {
        FieldName, /**< file name */
        Value /**< value */
    };

    void addData(QString fieldName, QString value);

    QString m_contentDisposition;
    QString m_fileName;
    bool m_attachment;
    QMap<QString, QString> m_data;
};
} // namespace Network

#endif // CONTENTDISPOSITIONPHARSER_H
