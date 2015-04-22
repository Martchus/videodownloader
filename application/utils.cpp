#include "utils.h"

#include <QString>
#include <QTextDocument>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>

namespace Application {

int substring(const QString &source, QString &target, int startIndex, const QString &startStr, const QString &endStr)
{
    int beg = source.indexOf(startStr, startIndex);
    if(beg > -1) {
        beg += startStr.length();
        int end = source.indexOf(endStr, beg);
        if(end > -1) {
            int len = end - beg;
            if(len > 0) {
                target = source.mid(beg, len);
                return beg;
            } else if(len == 0) {
                target.clear();
                return beg;
            }
        }
    }
    return -1;
}

void replaceHtmlEntities(QString &text)
{
    //text = text.replace("&amp;", "&").replace("&gt;", ">").replace("&lt;" ,"<").replace("&#39;", "'");
    QTextDocument textDocument;
    textDocument.setHtml(text);
    text = textDocument.toPlainText();
    foreach(const QChar &c, text) {
        if(!c.isSpace()) {
            return;
        }
    }
    text.clear(); // clear text if it contains white spaces only
}

QJsonObject loadJsonObjectFromResource(const QString &resource, QString *error)
{
    QFile file(resource);
    if(file.open(QFile::ReadOnly)) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if(parseError.error == QJsonParseError::NoError) {
            if(doc.isObject())
                return doc.object();
            else {
                if(error) {
                    *error = QStringLiteral("Json doesn't contain a main object.");
                }
            }
        } else {
            if(error) {
                *error = parseError.errorString();
            }
        }
    } else {
        if(error) {
            *error = QStringLiteral("Unable to open file.");
        }
    }

    return QJsonObject();
}

}
