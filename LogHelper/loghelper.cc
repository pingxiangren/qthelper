#include "loghelper.h"

#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QTextStream>

const QString LOG_PATH = "log";
const QString LOG_FILE_BASE_NAME = "application.log";
const qint64 MAX_LOG_SIZE_BYTES = 5 * 1024 * 1024;

Q_GLOBAL_STATIC(QMutex, logMutex)
Q_GLOBAL_STATIC(QFile, globalLogFile)

void checkAndRotateLogFile(QFile &file) {
    if (file.isOpen() && file.size() > MAX_LOG_SIZE_BYTES) {
        file.close();

        QString currentFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString backupFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME + "." + timestamp + ".bak");

        if (QFile::rename(currentFileName, backupFileName)) {
            fprintf(stderr, "Log system: Rotated file to %s\n", qPrintable(backupFileName));
        } else {
            fprintf(stderr, "Log system: WARNING! Failed to rename log file for rotation.\n");
        }
    }

    if (!file.isOpen()) {
        QString currentFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME);
        file.setFileName(currentFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            fprintf(stderr, "Log system: FATAL! Could not open log file %s for writing.\n",
                    qPrintable(currentFileName));
        }
    }
}
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker lock(logMutex());

    QString typeChar;
    switch (type) {
        case QtDebugMsg:
            typeChar = "D";
            break;
        case QtInfoMsg:
            typeChar = "I";
            break;
        case QtWarningMsg:
            typeChar = "W";
            break;
        case QtCriticalMsg:
            typeChar = "C";
            break;
        case QtFatalMsg:
            typeChar = "F";
            break;
    }

    QString messageFormat = QString("[%1][%2] -> [%3:%4 - %5]: %6\n")
                                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))
                                .arg(typeChar)
                                .arg(context.file)
                                .arg(context.line)
                                .arg(context.function)
                                .arg(msg);

    QFile &logFile = *globalLogFile();

    checkAndRotateLogFile(logFile);

    if (logFile.isOpen()) {
        QTextStream stream(&logFile);
        stream.setCodec("UTF-8");
        stream << messageFormat;
        stream.flush();
        logFile.flush();
    }

    fprintf(stderr, "%s", messageFormat.toLocal8Bit().constData());

    if (type == QtFatalMsg) {
        abort();
    }
}
