#ifndef STATIC_H
#define STATIC_H

#include <QJsonObject>

namespace Application {

int substring(const QString &source, QString &target, int startIndex, const QString &startStr, const QString &endStr);
void replaceHtmlEntities(QString &text);
QJsonObject loadJsonObjectFromResource(const QString &resource, QString *error = nullptr);
}

#endif // STATIC_H
