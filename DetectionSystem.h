#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DetectionSystem.h"
#include "DebugPage.h"
#include "TestPage.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QByteArray>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTimer>
#include <QColor>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QMessageBox>
#include <cmath>
#include <random>
#include <algorithm>
#include <climits>
#include <QDateTime>

#include "customintvalidator.h"
#include <QSpinBox>

class DetectionSystem : public QMainWindow
{
    Q_OBJECT

public:
    DetectionSystem(QWidget *parent = nullptr);
    ~DetectionSystem();

    
protected:
	void closeEvent(QCloseEvent *event) override;

private:
    Ui::DetectionSystemClass ui;
    QSerialPort *serial=NULL;
    QTimer *heartbeatTimer=NULL;
    QTimer *reconnectTimer=NULL;
    DebugPage* m_debugUI = NULL;
    TestPage* m_testUI = NULL;
    bool m_heartbeatSuc = false;
    QString m_lastPortName;
    bool m_userClosedSerial = false;
    bool m_reconnecting = false;

    // 协议辅助函数
    QByteArray buildCommand(quint8 cmd, const QByteArray &params = QByteArray());
    void parseResponse(const QByteArray &data);
    quint8 calculateChecksum(const QByteArray &data);
    void updateStatus(const QString &status);
    void updateVersion(const QString &ver);         // 更新版本
    void configureSerialPort(QSerialPort *port, const QString &portName);
    void refreshSerialPorts(const QString &preferredPort = QString());
    bool openSerialPort(const QString &portName, bool showError = true);
    void handleSerialDisconnected(const QString &reason);

    // 数据缓存
    QByteArray rxBuffer;

    QDateTime m_heartbeatTime;  //  最后一次心跳时间

public slots:
    void autoConnectSerial();                // 自动扫描并连接串口
    void onAttemptReadyRead();               // 扫描过程中处理接收数据

private:
    bool m_handshakeConfirmed = false;       // 握手是否成功
    QSerialPort* m_attemptSerial = nullptr;  // 当前尝试的串口指针
    QEventLoop* m_loop = nullptr;            // 事件循环指针
    QByteArray m_attemptBuffer;              // 临时接收缓冲区

signals:
    void sigTip(QString tip);
    void sig_heartStatue(QString tip);

public slots:
    void on_btnOpenSerial_clicked();
    void on_btnTest_clicked();
    void on_btnDebug_clicked();

    void onInit();
    void onVersion();

    // 心跳定时器
    void heartbeat();
    void readSerialData();
    void reconnectSerial();
    void handleSerialError(QSerialPort::SerialPortError error);

    bool sendCommand(quint8 cmd, const QByteArray &params = QByteArray());

    void parseTip(quint8 status, quint8 cmd);  //解析状态并发送
    void onClosePage(QWidget* pWidget);
    void slot_heartStatue(QString tip);

signals:
    //调试界面
    void sigXHOME1(QString text);
    void sigXHOME2(QString text);
    void sigXHOME3(QString text);
    void sigYHOME1(QString text);
    void sigYHOME2(QString text);
    void sigYHOME3(QString text);
    void sigXSpeed(QString text);
    void sigYSpeed(QString text);
    void sigMFreq(QString text);
    void sigBFreq(QString text);
    void sigPhase(QString text);
    void sigTFilter(QString text);
    void sigDFilter(QString text);
    void sigFilterA(QString text);
    void sigFilterB(QString text);
    void sigSFreq(QString text);
    void sigSFreq_Delay(QString text);
    void sigSFreq_Start(QString text);
    void sigSFreq_Cutoff(QString text);
    void sigPIDP(QString text);
    void sigPIDI(QString text);
    void sigPIDD(QString text);
    void sigTPID(QString text);
    void sigScanSpeed(QString text);
    void sigScanDelay(QString text);
    void sigScanModel(quint8 model);

    // 测试页面消息
    void sigStartX(QString startX,QString startY,QString endX,QString endY);
    void sigScanStep(QString text);
    void sigStatus(quint8 status);
    void sigXYValue(int x, int y, float value);
    void sigScanFrequency(float frequency);
};

