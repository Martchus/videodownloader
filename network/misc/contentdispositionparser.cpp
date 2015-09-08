#include "./contentdispositionparser.h"

namespace Network {

/*!
 * \class ContentDispositionParser
 * \brief The ContentDispositionParser class parses a HTTP content disposition.
 */

/*!
 * \brief Constructs a new ContentDispositionParser for the specified \a contentDisposition.
 */
ContentDispositionParser::ContentDispositionParser(const QString &contentDisposition)
{
    this->m_contentDisposition = contentDisposition;
}

/*!
 * \brief Adds data (internally used only).
 */
void ContentDispositionParser::addData(QString fieldName, QString value)
{
    while(fieldName.length() > 0 && fieldName.at(0) == ' ') {
        fieldName = fieldName.mid(1);
    }
    while(value.length() > 0 && value.at(0) == ' ') {
        value = value.mid(1);
    }
    while(fieldName.endsWith(' ')) {
        fieldName = fieldName.mid(0, fieldName.length() - 1);
    }
    while(value.endsWith(' ')) {
        value = value.mid(0, value.length() - 1);
    }

    if(fieldName.compare("attachment", Qt::CaseInsensitive)) {
        m_attachment = true;
    } else if(fieldName.compare("filename", Qt::CaseInsensitive)) {
        m_fileName = value;
    }

    m_data.insert(fieldName, value);
}

/*!
 * \brief Parses the content disposition.
 */
void ContentDispositionParser::pharse()
{
    bool inQuotationMarks = false;
    PharsingPosition pos = FieldName;
    QString fieldName;
    QString value;
    foreach(QChar c, m_contentDisposition) {
        if(c == '\"') {
            inQuotationMarks = !inQuotationMarks;
        } else if(c == ';') {
            if(inQuotationMarks) {
                switch(pos) {
                case FieldName:
                    fieldName.append(c);
                    break;
                case Value:
                    value.append(c);
                    break;
                }
            } else {
                addData(fieldName, value);
                fieldName.clear();
                value.clear();
                pos = FieldName;
            }
        } else if(c == '=') {
            if(inQuotationMarks) {
                switch(pos) {
                case FieldName:
                    fieldName.append(c);
                    break;
                case Value:
                    value.append(c);
                    break;
                }
            } else {
                switch(pos) {
                case FieldName:
                    pos = Value;
                    break;
                case Value:
                    value.append(c);
                    break;
                }
            }
        } else {
            switch(pos) {
            case FieldName:
                fieldName.append(c);
                break;
            case Value:
                value.append(c);
                break;
            }
        }
    }
    if(!fieldName.isEmpty()) {
        addData(fieldName, value);
    }
}

/*!
 * \brief Returns the file name.
 */
const QString &ContentDispositionParser::fileName()
{
    return m_fileName;
}

/*!
 * \brief Returns whether the content is an attachment.
 */
bool ContentDispositionParser::isAttachment()
{
    return m_attachment;
}

/*!
 * \brief Returns the data.
 */
const QMap<QString, QString> &ContentDispositionParser::data()
{
    return m_data;
}

}
