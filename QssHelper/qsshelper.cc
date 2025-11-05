#include "qsshelper.h"

#include <QFile>
#include <QFont>
#include <QDebug>

bool LoadQssFile(QApplication *app, const QString &filePath)
{
    app->setFont(QFont("Microsoft Yahei", 9));

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "open qss file failed:" << filePath;
        return false;
    }
    QString styleSheet{QLatin1String(file.readAll())};
    app->setStyleSheet(styleSheet);

    return true;
}
