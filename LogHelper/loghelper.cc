#include "loghelper.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QTextStream>
#include <QThread>

#include "src/common/app_paths.h"

const QString LOG_PATH = "log";
const QString LOG_FILE_BASE_NAME = "application.log";
const qint64 MAX_LOG_SIZE_BYTES = 5 * 1024 * 1024;

Q_GLOBAL_STATIC(QMutex, logMutex)
Q_GLOBAL_STATIC(QFile, globalLogFile)

namespace {

QString MessageTypeToString(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:
            return QStringLiteral("D");
        case QtInfoMsg:
            return QStringLiteral("I");
        case QtWarningMsg:
            return QStringLiteral("W");
        case QtCriticalMsg:
            return QStringLiteral("C");
        case QtFatalMsg:
            return QStringLiteral("F");
    }
    return QStringLiteral("?");
}

QString FileNameOnly(const char* file) {
    if (file == nullptr || *file == '\0') {
        return QStringLiteral("unknown");
    }
    return QFileInfo(QString::fromUtf8(file)).fileName();
}

QString CategoryName(const QMessageLogContext& context) {
    if (context.category == nullptr || context.category[0] == '\0') {
        return QStringLiteral("default");
    }
    return QString::fromUtf8(context.category);
}

void EnsureLogDirectoryExists() {
    QDir().mkpath(hm::paths::kLogDir);
}

int MessagePriority(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:
            return 0;
        case QtInfoMsg:
            return 1;
        case QtWarningMsg:
            return 2;
        case QtCriticalMsg:
            return 3;
        case QtFatalMsg:
            return 4;
    }
    return 0;
}

int MinimumPriority() {
    static const int cachedPriority = []() {
        const QString rawLevel = qEnvironmentVariable("HM_LOG_LEVEL", QStringLiteral("debug"))
                                     .trimmed()
                                     .toLower();
        if (rawLevel == QStringLiteral("debug")) {
            return 0;
        }
        if (rawLevel == QStringLiteral("info")) {
            return 1;
        }
        if (rawLevel == QStringLiteral("warning") || rawLevel == QStringLiteral("warn")) {
            return 2;
        }
        if (rawLevel == QStringLiteral("critical") || rawLevel == QStringLiteral("error")) {
            return 3;
        }
        if (rawLevel == QStringLiteral("fatal")) {
            return 4;
        }
        return 0;
    }();
    return cachedPriority;
}

}  // namespace

QString appLogFilePath() {
    EnsureLogDirectoryExists();
    return QDir(hm::paths::kLogDir).filePath(LOG_FILE_BASE_NAME);
}

void checkAndRotateLogFile(QFile &file) {
    EnsureLogDirectoryExists();

    if (file.isOpen() && file.size() > MAX_LOG_SIZE_BYTES) {
        file.close();

        QString currentFileName = QDir(hm::paths::kLogDir).filePath(LOG_FILE_BASE_NAME);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString backupFileName = QDir(hm::paths::kLogDir).filePath(
            LOG_FILE_BASE_NAME + "." + timestamp + ".bak");

        if (QFile::rename(currentFileName, backupFileName)) {
            fprintf(stderr, "Log system: Rotated file to %s\n", qPrintable(backupFileName));
        } else {
            fprintf(stderr, "Log system: WARNING! Failed to rename log file for rotation.\n");
        }
    }

    if (!file.isOpen()) {
        QString currentFileName = QDir(hm::paths::kLogDir).filePath(LOG_FILE_BASE_NAME);
        file.setFileName(currentFileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            fprintf(stderr, "Log system: FATAL! Could not open log file %s for writing.\n",
                    qPrintable(currentFileName));
        }
    }
}

void installAppMessageHandler() {
    static bool installed = false;
    if (installed) {
        return;
    }
    installed = true;
    qInstallMessageHandler(customMessageHandler);
}

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (MessagePriority(type) < MinimumPriority()) {
        return;
    }

    QMutexLocker lock(logMutex());

    QString messageFormat =
        QString("[%1][%2][tid:%3][%4][%5:%6 - %7] %8\n")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))
            .arg(MessageTypeToString(type))
            .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()))
            .arg(CategoryName(context))
            .arg(FileNameOnly(context.file))
            .arg(context.line)
            .arg(context.function ? QString::fromUtf8(context.function) : QStringLiteral("unknown"))
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
