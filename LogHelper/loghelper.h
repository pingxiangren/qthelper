#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QFile>

/**
 * @brief check and rotate log file
 * @param file log file object
 */
void checkAndRotateLogFile(QFile &file);

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif