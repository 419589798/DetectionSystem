#include "DetectionSystem.h"
#include <QtWidgets/QApplication>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QFileInfo>

QString g_logName = "";
QString g_version = "1.01";		//	版本号
QString g_maxCount = "10000";	//  采集最大值

static const qint64 LOG_FILE_MAX_SIZE = 10 * 1024 * 1024;
static const int LOG_FILE_MAX_COUNT = 10;


QString createTimestampLogFileName(const QDir& logDir)
{
	QString strLog = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
	QString fileName = logDir.filePath(QString("%1.txt").arg(strLog));
	int index = 1;
	while (QFileInfo::exists(fileName))
	{
		fileName = logDir.filePath(QString("%1_%2.txt").arg(strLog).arg(index++));
	}
	return fileName;
}

void cleanupLogFiles(const QDir& logDir)
{
	QFileInfoList fileList = logDir.entryInfoList(QStringList() << "*.txt", QDir::Files, QDir::Time | QDir::Reversed);
	while (fileList.count() >= LOG_FILE_MAX_COUNT)
	{
		QFile::remove(fileList.takeFirst().absoluteFilePath());
	}
}

QString nextLogFileName()
{
	QDir logDir(QCoreApplication::applicationDirPath() + "/Log");
	if (!logDir.exists())
		logDir.mkpath(".");

	if (!g_logName.isEmpty())
	{
		QFileInfo currentInfo(g_logName);
		if (!currentInfo.exists() || currentInfo.size() < LOG_FILE_MAX_SIZE)
			return g_logName;
	}

	cleanupLogFiles(logDir);
	return createTimestampLogFileName(logDir);
}

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	static QMutex mutex;
	mutex.lock();

	QString text;
	switch (type)
	{
	case QtDebugMsg:
		text = QString("Debug:");
		break;

	case QtWarningMsg:
	{
		text = QString("Warning:");
		mutex.unlock();
		return;
	}
	break;
	case QtCriticalMsg:
		text = QString("Critical:");
		break;

	default:
	{
		mutex.unlock();
		return;
	}
	}

	QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss sss");
	QString current_date = QString("(%1)").arg(current_date_time);
	QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

	g_logName = nextLogFileName();

	QFile file(g_logName);
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << message << "\r\n";
	file.flush();
	file.close();

	mutex.unlock();
}

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);    // 启用高DPI自动缩放
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);       // 高DPI下使用高清图标
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton); // 可选：减少系统控件干扰

    QApplication app(argc, argv);

	app.setWindowIcon(QIcon(":/DetectionSystem/image/logo.ico"));

	g_logName = nextLogFileName();
	qInstallMessageHandler(outputMessage);

    DetectionSystem window;
    window.show();
    return app.exec();
}
