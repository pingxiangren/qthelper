#include "qsshelper.h"

#include <QFile>
#include <QFont>

bool LoadQssFile(QApplication *app, const QString &filePath) {
    app->setFont(QFont("Microsoft Yahei", 9));  // 设置统一字体

    QFile file(filePath);
    // cout << "path_qss:" << path_qss.toStdString() << endl;
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "打开Qss文件失败: " << filePath;
        return false;
    }
    QString styleSheet{QLatin1String(file.readAll())};
    app->setStyleSheet(styleSheet);  // * 设置样式

    return true;
}
