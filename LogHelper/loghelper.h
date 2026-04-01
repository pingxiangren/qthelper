#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QFile>
#include <QString>
#include <QtGlobal>   // QtMsgType, QMessageLogContext

/**
 * @brief 检查日志文件大小并执行轮转（重命名旧文件，创建新文件）。
 * @param file 要操作的QFile对象。
 */
void checkAndRotateLogFile(QFile &file);

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif