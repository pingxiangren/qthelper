#include "loghelper.h"

#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QTextStream>

// --------------------------- 配置参数 -----------------------------------
// 日志文件路径和名称
const QString LOG_PATH = "log";
const QString LOG_FILE_BASE_NAME = "HmIvDiagnosis.log";
const qint64 MAX_LOG_SIZE_BYTES = 5 * 1024 * 1024;  // 5 MB

// --------------------------- 静态变量和工具 ------------------------------
// 确保在多线程环境下写入文件是安全的
Q_GLOBAL_STATIC(QMutex, logMutex)
// 静态文件对象，用于保持文件句柄和状态
Q_GLOBAL_STATIC(QFile, globalLogFile)

// --------------------------- 轮转逻辑函数 ------------------------------
/**
 * @brief 检查日志文件大小并执行轮转（重命名旧文件，创建新文件）。
 * @param file 要操作的QFile对象。
 */
void checkAndRotateLogFile(QFile &file) {
    // 只有在文件已打开且文件大小超过限制时才执行轮转
    if (file.isOpen() && file.size() > MAX_LOG_SIZE_BYTES) {
        // 1. 关闭当前文件
        file.close();

        // 2. 生成新的备份文件名（带时间戳）
        QString currentFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString backupFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME + "." + timestamp + ".bak");

        // 3. 执行重命名操作
        if (QFile::rename(currentFileName, backupFileName)) {
            // 使用标准错误输出，避免递归调用 qDebug/qWarning
            fprintf(stderr, "Log system: Rotated file to %s\n", qPrintable(backupFileName));
        } else {
            fprintf(stderr, "Log system: WARNING! Failed to rename log file for rotation.\n");
        }
    }

    // 4. 确保文件被打开用于写入（无论是新创建还是重新打开）
    if (!file.isOpen()) {
        QString currentFileName = QDir(LOG_PATH).filePath(LOG_FILE_BASE_NAME);
        file.setFileName(currentFileName);

        // 尝试打开文件，以追加模式写入
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            // 如果打开失败，则必须回退到标准错误输出
            fprintf(stderr, "Log system: FATAL! Could not open log file %s for writing.\n",
                    qPrintable(currentFileName));
        }
    }
}
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    // 线程安全锁定
    QMutexLocker lock(logMutex());

    // ------------------- 1. 格式化消息 -----------------------
    QString typeChar;
    // 使用您要求的格式 [I]、[D] 等
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

    // 构造您要求的格式：
    QString messageFormat = QString("[%1][%2] -> [%3:%4 - %5]: %6\n")
                                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"))
                                .arg(typeChar)
                                .arg(context.file)
                                .arg(context.line)
                                .arg(context.function)
                                .arg(msg);

    // ------------------- 2. 写入文件（包含轮转逻辑） -----------------------
    QFile &logFile = *globalLogFile();

    // 检查是否需要轮转或打开文件
    checkAndRotateLogFile(logFile);

    if (logFile.isOpen()) {
        QTextStream stream(&logFile);
        stream.setCodec("UTF-8");
        stream << messageFormat;
        // 强制将数据写入磁盘，确保日志完整性
        stream.flush();
        logFile.flush();
    }

    // ------------------- 3. 打印到控制台（保持调试功能） --------------------
    // 默认行为是同时输出到 stderr/stdout，使用 fprintf 避免递归
    fprintf(stderr, "%s", messageFormat.toLocal8Bit().constData());

    // 致命错误必须退出应用
    if (type == QtFatalMsg) {
        abort();
    }
}
