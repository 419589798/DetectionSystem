#include "DetectionSystem.h"
#include <QMessageBox>
#include <QDebug>
#include <QRandomGenerator>
#include <QColorDialog>

#include <QFileDialog>
#include <QDir>
#include "ToastWidget.h"

extern QString g_version;	//	版本号
DetectionSystem::DetectionSystem(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowTitle("检测系统");
    this->setWindowIcon(QIcon(":/DetectionSystem/image/logo.ico"));

    m_debugUI = new DebugPage();
    connect(m_debugUI, &DebugPage::sigSendCmd, this, &DetectionSystem::sendCommand);
    connect(m_debugUI, &DebugPage::sigCloseDebugPage, this, &DetectionSystem::onClosePage);
    connect(this, &DetectionSystem::sigXHOME1, m_debugUI, &DebugPage::returnXHOME1);
    connect(this, &DetectionSystem::sigXHOME2, m_debugUI, &DebugPage::returnXHOME2);
    connect(this, &DetectionSystem::sigXHOME3, m_debugUI, &DebugPage::returnXHOME3);
    connect(this, &DetectionSystem::sigYHOME1, m_debugUI, &DebugPage::returnYHOME1);
    connect(this, &DetectionSystem::sigYHOME2, m_debugUI, &DebugPage::returnYHOME2);
    connect(this, &DetectionSystem::sigYHOME3, m_debugUI, &DebugPage::returnYHOME3);
    connect(this, &DetectionSystem::sigXSpeed, m_debugUI, &DebugPage::returnXSpeed);
    connect(this, &DetectionSystem::sigYSpeed, m_debugUI, &DebugPage::returnYSpeed);
    connect(this, &DetectionSystem::sigMFreq, m_debugUI, &DebugPage::returnMFreq);
    connect(this, &DetectionSystem::sigBFreq, m_debugUI, &DebugPage::returnBFreq);
    connect(this, &DetectionSystem::sigPhase, m_debugUI, &DebugPage::returnPhase);
    connect(this, &DetectionSystem::sigTFilter, m_debugUI, &DebugPage::returnTFilter);
    connect(this, &DetectionSystem::sigDFilter, m_debugUI, &DebugPage::returnDFilter);
    connect(this, &DetectionSystem::sigFilterA, m_debugUI, &DebugPage::returnFilterA);
    connect(this, &DetectionSystem::sigFilterB, m_debugUI, &DebugPage::returnFilterB);
    connect(this, &DetectionSystem::sigSFreq, m_debugUI, &DebugPage::returnSFreq);
    connect(this, &DetectionSystem::sigSFreq_Delay, m_debugUI, &DebugPage::returnSFreq_Delay);
    connect(this, &DetectionSystem::sigSFreq_Start, m_debugUI, &DebugPage::returnSFreq_Start);
    connect(this, &DetectionSystem::sigSFreq_Cutoff, m_debugUI, &DebugPage::returnSFreq_Cutoff);
    connect(this, &DetectionSystem::sigPIDP, m_debugUI, &DebugPage::returnPIDP);
    connect(this, &DetectionSystem::sigPIDI, m_debugUI, &DebugPage::returnPIDI);
    connect(this, &DetectionSystem::sigPIDD, m_debugUI, &DebugPage::returnPIDD);
    connect(this, &DetectionSystem::sigTPID, m_debugUI, &DebugPage::returnTPID);
    connect(this, &DetectionSystem::sigScanSpeed, m_debugUI, &DebugPage::returnScanSpeed);
    connect(this, &DetectionSystem::sigScanDelay, m_debugUI, &DebugPage::returnScanDelay);
    connect(this, &DetectionSystem::sigScanModel, m_debugUI, &DebugPage::returnScanModel);
    connect(this, &DetectionSystem::sigTip, m_debugUI, &DebugPage::updateStatus);

    m_testUI = new TestPage();
    connect(m_testUI, &TestPage::sigSendCmd, this, &DetectionSystem::sendCommand);
    connect(m_testUI, &TestPage::sigCloseTestPage, this, &DetectionSystem::onClosePage);
    connect(this, &DetectionSystem::sigXYValue, m_testUI, &TestPage::returnXYValue);
    connect(this, &DetectionSystem::sigScanFrequency, m_testUI, &TestPage::updateScanFrequency);
    connect(this, &DetectionSystem::sigTip, m_testUI, &TestPage::updateStatus);
    connect(this, &DetectionSystem::sigStatus, m_testUI, &TestPage::updateScanStatus);

    connect(this, &DetectionSystem::sig_heartStatue, m_testUI, &TestPage::slot_heartStatue);
    connect(this, &DetectionSystem::sig_heartStatue, this, &DetectionSystem::slot_heartStatue);
    
    // 串口初始化
    serial = new QSerialPort(this);

    // 查找可用串口并添加到下拉框
    refreshSerialPorts();

    // 心跳定时器（每1秒发送一次HELLO）
    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &DetectionSystem::heartbeat);

    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(2000);
    connect(reconnectTimer, &QTimer::timeout, this, &DetectionSystem::reconnectSerial);

    ///////////////////////////////////////////////////////////////////////////////
    
    //ui.widget->setVisible(false);
    ui.label_tip->setVisible(false);
    // 暂时关闭自动扫描串口，使用手动选择串口后点击连接。
    //QTimer::singleShot(0, this, &DetectionSystem::autoConnectSerial);
}

DetectionSystem::~DetectionSystem()
{
     if (reconnectTimer)
         reconnectTimer->stop();

     if (heartbeatTimer)
         heartbeatTimer->stop();

     if (serial && serial->isOpen())
        serial->close();

     if (m_testUI)
         m_testUI->destroyed();
}

void DetectionSystem::slot_heartStatue(QString tip)
{
    if (tip == "心跳连接失败！") {
    }
    else {
        ui.btnTest->setEnabled(true);
        ui.btnDebug->setEnabled(true);
    }

    ui.label_tipHeart->setText(tip);
}

void DetectionSystem::configureSerialPort(QSerialPort *port, const QString &portName)
{
    port->setPortName(portName);
    port->setBaudRate(115200);
    port->setDataBits(QSerialPort::Data8);
    port->setStopBits(QSerialPort::OneStop);
    port->setParity(QSerialPort::NoParity);
    port->setFlowControl(QSerialPort::NoFlowControl);
}

void DetectionSystem::refreshSerialPorts(const QString &preferredPort)
{
    QString currentPort = preferredPort.isEmpty() ? ui.comboPort->currentText() : preferredPort;
    ui.comboPort->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui.comboPort->addItem(info.portName());
    }

    int index = ui.comboPort->findText(currentPort);
    if (index >= 0)
        ui.comboPort->setCurrentIndex(index);
}

bool DetectionSystem::openSerialPort(const QString &portName, bool showError)
{
    if (!serial || portName.isEmpty()) {
        if (showError)
            QMessageBox::critical(this, "错误", "请选择串口");
        return false;
    }

    if (serial->isOpen())
        serial->close();

    configureSerialPort(serial, portName);
    if (!serial->open(QIODevice::ReadWrite)) {
        if (showError)
            QMessageBox::critical(this, "错误", QString("无法打开串口：%1").arg(portName));
        return false;
    }

    m_lastPortName = portName;
    m_reconnecting = false;
    if (reconnectTimer)
        reconnectTimer->stop();

    connect(serial, &QSerialPort::readyRead, this, &DetectionSystem::readSerialData, Qt::UniqueConnection);
    connect(serial, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &DetectionSystem::handleSerialError, Qt::UniqueConnection);

    ui.btnOpenSerial->setText("关闭串口");
    updateStatus(QString("串口已打开：%1").arg(portName));

    if (heartbeatTimer)
        heartbeatTimer->start(5000);

    m_heartbeatTime = QDateTime::currentDateTime();
    onVersion();
    return true;
}

void DetectionSystem::handleSerialDisconnected(const QString &reason)
{
    if (m_userClosedSerial || m_reconnecting)
        return;

    m_reconnecting = true;
    if (heartbeatTimer)
        heartbeatTimer->stop();

    if (serial) {
        disconnect(serial, &QSerialPort::readyRead, this, &DetectionSystem::readSerialData);
        if (serial->isOpen())
            serial->close();
    }

    rxBuffer.clear();
    ui.btnOpenSerial->setText("打开串口");
    updateStatus(reason);
    emit sig_heartStatue("心跳连接失败！");

    refreshSerialPorts(m_lastPortName);
    if (reconnectTimer && !reconnectTimer->isActive())
        reconnectTimer->start();
}

void DetectionSystem::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || m_userClosedSerial)
        return;

    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError) {
        handleSerialDisconnected("串口连接断开，正在尝试重连...");
    }
}

void DetectionSystem::reconnectSerial()
{
    if (m_userClosedSerial || !reconnectTimer)
        return;

    refreshSerialPorts(m_lastPortName);
    if (ui.comboPort->findText(m_lastPortName) < 0) {
        updateStatus("未检测到原串口，等待重新插入...");
        return;
    }

    if (openSerialPort(m_lastPortName, false)) {
        updateStatus(QString("串口已重连：%1").arg(m_lastPortName));
    } else {
        updateStatus(QString("串口重连中：%1").arg(m_lastPortName));
    }
}
// 串口按钮
void DetectionSystem::on_btnOpenSerial_clicked()
{
    if (serial == NULL) return;

    if (serial->isOpen()) {
        m_userClosedSerial = true;
        if (heartbeatTimer)
            heartbeatTimer->stop();
        if (reconnectTimer)
            reconnectTimer->stop();
        disconnect(serial, &QSerialPort::readyRead, this, &DetectionSystem::readSerialData);
        serial->close();
        ui.btnOpenSerial->setText("打开串口");
        updateStatus("串口已关闭");
        return;
    }

    m_userClosedSerial = false;
    m_lastPortName = ui.comboPort->currentText();
    openSerialPort(m_lastPortName, true);
}

void DetectionSystem::on_btnTest_clicked()
{
    m_testUI->show();
    hide();
}
    
void DetectionSystem::on_btnDebug_clicked()
{
    m_debugUI->show();
    hide();
}

quint8 DetectionSystem::calculateChecksum(const QByteArray &data)
{
    quint8 sum = 0;
    for (int i = 0; i < data.size(); i++) {
        sum += static_cast<quint8>(data.at(i));
    }
    return sum;
}

QByteArray DetectionSystem::buildCommand(quint8 cmd, const QByteArray &params)
{
    QByteArray frame;
    frame.append(static_cast<char>(0xFE)); // 帧头
    frame.append(static_cast<char>(cmd));

    // 填充参数，固定帧长20字节，参数3-13共11个字节，这里简单处理：实际应根据协议填充
    QByteArray paramData;
    paramData.append(params);
    while (paramData.size() < 13) {
        paramData.append(static_cast<char>(0x00));
    }
    frame.append(paramData);

    quint8 checksum = calculateChecksum(frame.mid(1)); // 从指令开始计算
    frame.append(static_cast<char>(checksum));
    frame.append(static_cast<char>(0x55));
    frame.append(static_cast<char>(0xAA));
    frame.append(static_cast<char>(0x33));
    frame.append(static_cast<char>(0xCC));

    return frame;
}

bool DetectionSystem::sendCommand(quint8 cmd, const QByteArray &params)
{
    if (!serial || !serial->isOpen()) {
        QMessageBox::warning(this, "错误", "串口未打开");
        return false;
    }
    QByteArray frame = buildCommand(cmd, params);
    if(cmd != 0x10)
        qDebug() << "Send:" << frame.toHex();
    qint64 written = serial->write(frame);
    if (written != frame.size()) {
        handleSerialDisconnected("串口写入失败，正在尝试重连...");
        return false;
    }
    return true;
}

void DetectionSystem::readSerialData()
{
    rxBuffer.append(serial->readAll());

    qDebug() << "Recv cmd:" << hex << rxBuffer;
    QString hexString = rxBuffer.toHex(' ').toUpper(); 
    // 查找帧尾 55 AA 33 CC
    int endIdx;
    while ((endIdx = rxBuffer.indexOf(QByteArray::fromHex("55AA33CC"))) != -1) {
        int startIdx = endIdx - 16; // 帧长20字节，帧尾占4字节
        if (startIdx >= 0) {
            QByteArray frame = rxBuffer.mid(startIdx, 20);
            rxBuffer.remove(0, endIdx + 4);
            parseResponse(frame);
        } else {
            // 不完整的帧，等待更多数据
            break;
        }
    }
}

void DetectionSystem::parseResponse(const QByteArray &data)
{
    if (data.size() != 20) return;
    quint8 header = static_cast<quint8>(data[0]);
    if (header != 0xFD) return; // 只处理上传帧

    quint8 cmd = static_cast<quint8>(data[1]);
    quint8 status = static_cast<quint8>(data[2]);
    quint8 param1 = static_cast<quint8>(data[2]);
    quint16 param2 = static_cast<quint16>(data[3])<<8;
    quint16 param3 = static_cast<quint16>(data[4])& 0xff;

    quint8 paramStatus = static_cast<quint8>(data[14]);//00正常响应，01初始化后返回

    //qDebug() << "Recv cmd:" << hex << cmd << "status:" << status;

    switch (cmd) {
    case 0x10: // DEV_HELLO响应
        if (status == 0x00) {
            qint64 timestampSec = QDateTime::currentSecsSinceEpoch();
            // 转换为 QDateTime 对象
            m_heartbeatTime = QDateTime::fromSecsSinceEpoch(timestampSec);

            emit sig_heartStatue("心跳连接成功！");
        }
        //parseTip(status,cmd);
        break;
    case 0x11: // DEV_INIT响应
        if (status == 0x00) {
            
        }
        parseTip(status,cmd);
        break;
    case 0x12: // DEV_VER_READ响应
        if (status == 0x00) {
            param2 = static_cast<quint8>(data[3]);
            param3 = static_cast<quint8>(data[4]);
            QString ver = QString("%1.%2").arg(param2).arg(param3,2, 10, QChar('0'));
            updateVersion(ver);
            onInit();
        }
        parseTip(status,cmd);
        break;
    case 0x20: //SCAN_RUN 0x20 启动扫描
        parseTip(status,cmd);
        break;
    case 0x21: //SCAN_STOP 0x21 停止扫描
        parseTip(status,cmd);
        break;
    case 0x22: //SCAN_RANGE_SET 0x22 扫描范围设置（含X、Y两个范围值）
        if (paramStatus == 0x01)
        {
            quint8 param4 = static_cast<quint8>(data[5])<<8;
            quint8 param5 = static_cast<quint8>(data[6]);
            quint8 param6 = static_cast<quint8>(data[7])<<8;
            quint8 param7 = static_cast<quint8>(data[8]);
            quint8 param8 = static_cast<quint8>(data[9])<<8;
            quint8 param9 = static_cast<quint8>(data[10]);
            QString startX = QString::number(static_cast<quint16>(param2 | param3));
            QString startY = QString::number(static_cast<quint16>(param4 | param5));
            QString endX = QString::number(static_cast<quint16>(param6 | param7));
            QString endY = QString::number(static_cast<quint16>(param8 | param9));

            emit sigStartX(startX,startY,endX,endY);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x23: //SCAN_SPEED_SET 0x23 扫描速度设置（只扫描时电机运行速度）
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigScanSpeed(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x24: //SCAN_DELAY_SET 0x24 扫描时间间隔设置（单位ms，为0时代表连续扫描）
        if (paramStatus == 0x01)
        {
            QString temp = QString::number(static_cast<quint16>(param2 | param3));
            emit sigScanDelay(temp);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x25: //SCAN_STEP_SET 0x25 扫描步进设置(间隔扫描时每次移动的距离)
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigScanStep(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x26: //SCAN_PID_SET 0x26 扫描PID参数设置（包含P、I、D三个参数）
        if (paramStatus == 0x01)
        {
            quint32 pidp = (static_cast<quint32>(static_cast<quint8>(data[5])) << 24) |
                (static_cast<quint32>(static_cast<quint8>(data[4])) << 16) |
                (static_cast<quint32>(static_cast<quint8>(data[3])) << 8) |
                static_cast<quint32>(static_cast<quint8>(data[2]));

            quint32 pidi = (static_cast<quint32>(static_cast<quint8>(data[9])) << 24) |
                (static_cast<quint32>(static_cast<quint8>(data[8])) << 16) |
                (static_cast<quint32>(static_cast<quint8>(data[7])) << 8) |
                static_cast<quint32>(static_cast<quint8>(data[6]));


            quint32 pidd = (static_cast<quint32>(static_cast<quint8>(data[13])) << 24) |
                (static_cast<quint32>(static_cast<quint8>(data[12])) << 16) |
                (static_cast<quint32>(static_cast<quint8>(data[11])) << 8) |
                static_cast<quint32>(static_cast<quint8>(data[10]));

            float fpidp = 0.0f;
            float fpidi = 0.0f;
            float fpidd = 0.0f;
            memcpy(&fpidp, &pidp, sizeof(float));
            memcpy(&fpidi, &pidi, sizeof(float));
            memcpy(&fpidd, &pidd, sizeof(float));
            
            emit sigPIDP(QString::number(fpidp));
            emit sigPIDI(QString::number(fpidi));
            emit sigPIDD(QString::number(fpidd));
        }
        else {
            quint8 receivedChecksum = static_cast<quint8>(data[15]);
            quint8 calculatedChecksum = calculateChecksum(data.mid(1, 14));
            if (receivedChecksum != calculatedChecksum) {
                qDebug() << "0x26 响应校验和错误:" << receivedChecksum
                         << "calculated:" << calculatedChecksum;

                parseTip(0x01, cmd);
                break;
            }

            parseTip(0x00,cmd);
        }
        break;    
    case 0x27: //SCAN_STATUS_REBACK 0x27 扫描状态返回（空闲、采集中、采集完成）
        {
            emit sigStatus(param1);
        }
        break;
    case 0x28: // SCAN_DATA_REBACK
        {
            quint32 frameId = (static_cast<quint32>(data[2]) << 24) |
                              (static_cast<quint32>(data[3]) << 16) |
                              (static_cast<quint32>(data[4]) << 8) |
                              static_cast<quint32>(data[5]& 0xff);
            quint16 x = (static_cast<quint16>(data[6]) << 8) | static_cast<quint16>(data[7] & 0xff);
            quint16 y = (static_cast<quint16>(data[8]) << 8) | static_cast<quint16>(data[9] & 0xff);
            //quint16 value = (static_cast<quint16>(data[10]) << 8) | static_cast<quint16>(data[11]& 0xff);

            quint32 freqRaw = (static_cast<quint32>(static_cast<quint8>(data[13])) << 24) |
                (static_cast<quint32>(static_cast<quint8>(data[12])) << 16) |
                (static_cast<quint32>(static_cast<quint8>(data[11])) << 8) |
                static_cast<quint32>(static_cast<quint8>(data[10]));

            float frequency = 0.0f;
            memcpy(&frequency, &freqRaw, sizeof(float));
            emit sigXYValue(x,y, frequency);
        }
        break;
    case 0x29: // SCAN_FREQ_REBACK 扫描频率上传
        {
            quint32 freqRaw = (static_cast<quint32>(static_cast<quint8>(data[5])) << 24) |
                              (static_cast<quint32>(static_cast<quint8>(data[4])) << 16) |
                              (static_cast<quint32>(static_cast<quint8>(data[3])) << 8) |
                              static_cast<quint32>(static_cast<quint8>(data[2]));

            float frequency = 0.0f;
            memcpy(&frequency, &freqRaw, sizeof(float));
            emit sigScanFrequency(frequency);
        }
        break;
    case 0x30: //DEV_XHOME_SET 0x30 X轴HOME设置指令（每轴预留3个位置）
        if (paramStatus == 0x01)
        {
            quint8 Home = static_cast<quint8>(data[3]);
            quint16 rawValue = (static_cast<quint16>(static_cast<quint8>(data[4])) << 8) |
                               static_cast<quint16>(static_cast<quint8>(data[5]));
            qint16 paramTemp = static_cast<qint16>(rawValue);
            QString temp = QString::number(paramTemp);
            if (Home == 0x01)   emit sigXHOME1(temp);
            else if (Home == 0x02)   emit sigXHOME2(temp);
            else if (Home == 0x03)   emit sigXHOME3(temp);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x31: //DEV_YHOME_SET 0x31 Y轴HOME设置指令（每轴预留3个位置）
        if (paramStatus == 0x01)
        {
            quint8 Home = static_cast<quint8>(data[3]);
            quint16 rawValue = (static_cast<quint16>(static_cast<quint8>(data[4])) << 8) |
                               static_cast<quint16>(static_cast<quint8>(data[5]));
            qint16 paramTemp = static_cast<qint16>(rawValue);
            QString temp = QString::number(paramTemp);
            if (Home == 0x01)   emit sigYHOME1(temp);
            else if (Home == 0x02)   emit sigYHOME2(temp);
            else if (Home == 0x03)   emit sigYHOME3(temp);
        }
        else {
            parseTip(status,cmd);
        }
        break;    case 0x32: //DEV_XSPEED_SET 0x32 X轴电机回零速度设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigXSpeed(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x33: //DEV_YSPEED_SET 0x33 Y轴电机回零速度设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigYSpeed(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x34: //DEV_MFREQ_SET 0x34 调制频率设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigMFreq(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x35: //DEV_BFREQ_SET 0x35 带通频率设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigBFreq(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x36: //DEV_PHASE_SET 0x36 相位补偿设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigPhase(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x37: //DEV_TPID_SET 0x37 PID调整周期设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigTPID(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x38: //DEV_TFILTER_SET 0x38 滤波周期设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigTFilter(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x39: //DEV_DFILTER_SET 0x39 滤波深度设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigDFilter(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x3A: //DEV_FILTERA_SET 0x3A 滤波备用参数A设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigFilterA(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x3B: //DEV_FILTERB_SET 0x3B 滤波备用参数B设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigFilterB(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    /// <param name="data"></param>
    case 0x40: //INIT_SFREQ_SET 0x40 初始化扫频单位设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigSFreq(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x41: //INIT_FREQ_DELAY_SET 0x41 初始化扫频间隔设置
        if (paramStatus == 0x01)
        {
            QString scanStep = QString::number(static_cast<quint16>(param2 | param3));
            emit sigSFreq_Delay(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x42: //INIT_FREQ_START_SET 0x42 扫频起始频率设置
        if (paramStatus == 0x01)
        {
            double freq = static_cast<double>(static_cast<quint16>(param2 | param3)) / 10.0;
            QString scanStep = QString::number(freq, 'f', 1);
            emit sigSFreq_Start(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x43: //INIT_FREQ_CUTOFF_SET 0x43 扫频截止频率设置
        if (paramStatus == 0x01)
        {
            double freq = static_cast<double>(static_cast<quint16>(param2 | param3)) / 10.0;
            QString scanStep = QString::number(freq, 'f', 1);
            emit sigSFreq_Cutoff(scanStep);
        }
        else {
            parseTip(status,cmd);
        }
        break;
    case 0x44: //INIT_SCAN_MODEL_SET 0x44 扫描模式设置
        if (paramStatus == 0x01)
        {
            emit sigScanModel(static_cast<quint8>(data[3]));
        }
        else {
            parseTip(status,cmd);
        }
        break;
    default:
        if (status == 0x00) updateStatus("指令执行成功");
        else if (status == 0x01) updateStatus("指令执行错误");
        else if (status == 0x02) updateStatus("指令执行中");
        else if (status == 0x03) updateStatus("错误指令或数据");
        break;
    }
}

void DetectionSystem::updateStatus(const QString &status)
{
    //ui->statusBar->showMessage(status);
}

void DetectionSystem::updateVersion(const QString &ver)
{
    QString topVersion = "上位机版本：V" + g_version;
    QString bomVersion = "下位机版本：V" + ver;

    if (m_debugUI)
        m_debugUI->updateVer(topVersion,bomVersion);
}

void DetectionSystem::heartbeat()
{
    if (!serial || !serial->isOpen())
        return;

    QDateTime now = QDateTime::currentDateTime();
    qint64 diffSeconds = m_heartbeatTime.secsTo(now);
    if (diffSeconds > 60)
    {
        handleSerialDisconnected("心跳连接失败，正在尝试重连...");
        return;
    }

    sendCommand(0x10); // DEV_HELLO
}

// 开机初始化
void DetectionSystem::onInit() { sendCommand(0x11); }
void DetectionSystem::onVersion() { sendCommand(0x12); }

void DetectionSystem::parseTip(quint8 status, quint8 cmd)
{
    QString tip;
    if (status == 0x00) tip=("指令执行成功");
    else if (status == 0x01) tip=("指令执行错误");
    else if (status == 0x02) tip=("指令执行中");
    else if (status == 0x03) tip=("错误指令或数据");

    QString tipCmd;
    if (cmd == 0x90) tipCmd = ("指令执行成功");
    else if (cmd == 0x11) tipCmd = ("仪器初始化");
    else if (cmd == 0x12) tipCmd = ("仪器软件版本读取");
    else if (cmd == 0x20) tipCmd = ("启动扫描");
    else if (cmd == 0x21) tipCmd = ("停止扫描");
    else if (cmd == 0x22) tipCmd = ("扫描范围设置");
    else if (cmd == 0x23) tipCmd = ("扫描速度设置");
    else if (cmd == 0x24) tipCmd = ("扫描时间间隔设置");
    else if (cmd == 0x25) tipCmd = ("扫描步进设置");
    else if (cmd == 0x26) tipCmd = ("扫描PID参数设置");
    //else if (cmd == 0x27) tip = ("指令执行错误");
    //else if (cmd == 0x28) tip = ("扫描数据上传(");
    else if (cmd == 0x30) tipCmd = ("X轴HOME设置");
    else if (cmd == 0x31) tipCmd = ("Y轴HOME设置");
    else if (cmd == 0x32) tipCmd = ("X轴电机回零速度");
    else if (cmd == 0x33) tipCmd = ("Y轴电机回零速度");
    else if (cmd == 0x34) tipCmd = ("调制频率设置");
    else if (cmd == 0x35) tipCmd = ("带通频率设置");
    else if (cmd == 0x36) tipCmd = ("相位补偿设置");
    else if (cmd == 0x37) tipCmd = ("PID调整周期设置");
    else if (cmd == 0x38) tipCmd = ("滤波周期设置");
    else if (cmd == 0x39) tipCmd = ("滤波深度设置");
    else if (cmd == 0x3A) tipCmd = ("滤波备用参数A设置");
    else if (cmd == 0x3B) tipCmd = ("滤波备用参数B设置");
    else if (cmd == 0x40) tipCmd = ("初始化扫频单位设置");
    else if (cmd == 0x41) tipCmd = ("初始化扫频间隔设置");
    else if (cmd == 0x42) tipCmd = ("扫频起始频率设置");
    else if (cmd == 0x43) tipCmd = ("扫频截止频率设置");
    else if (cmd == 0x44) tipCmd = ("扫描模式设置");
    
    QString temp = "指令返回：" + tipCmd+tip;
    emit sigTip(temp);
    ui.label_tip->setText(temp);
}

void DetectionSystem::onClosePage(QWidget* pWidget)
{
    pWidget->hide();
    show();
}

void DetectionSystem::autoConnectSerial()
{
    // 获取当前可用串口列表
    QList<QSerialPortInfo> portInfos = QSerialPortInfo::availablePorts();
    if (portInfos.isEmpty()) {
        QMessageBox::warning(this, "警告", "未检测到任何串口！");
        return;
    }

    // 依次尝试每个串口
    for (const QSerialPortInfo &info : portInfos) {
        QString portName = info.portName();
        ToastWidget::instance(NULL)->showToast(QString("正在尝试串口 %1 ...").arg(portName));

        // 创建临时串口对象
        m_attemptSerial = new QSerialPort(this);
        m_attemptSerial->setPortName(portName);
        m_attemptSerial->setBaudRate(115200);
        m_attemptSerial->setDataBits(QSerialPort::Data8);
        m_attemptSerial->setStopBits(QSerialPort::OneStop);
        m_attemptSerial->setParity(QSerialPort::NoParity);
        m_attemptSerial->setFlowControl(QSerialPort::NoFlowControl);

        if (!m_attemptSerial->open(QIODevice::ReadWrite)) {
            delete m_attemptSerial;
            m_attemptSerial = nullptr;
            continue;   // 打开失败，尝试下一个
        }

        // 清空临时缓冲区，重置握手标志
        m_attemptBuffer.clear();
        m_handshakeConfirmed = false;

        // 连接临时数据接收槽
        connect(m_attemptSerial, &QSerialPort::readyRead, this, &DetectionSystem::onAttemptReadyRead);

        // 发送 DEV_HELLO 指令 (0x10)
        QByteArray cmd = buildCommand(0x10, QByteArray());
        m_attemptSerial->write(cmd);

        // 创建事件循环，等待响应或超时
        QEventLoop loop;
        m_loop = &loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timeoutTimer.start(2000);   // 2000ms 超时

        loop.exec();               // 阻塞直到超时或收到有效握手

        // 断开临时信号连接
        disconnect(m_attemptSerial, &QSerialPort::readyRead, this, &DetectionSystem::onAttemptReadyRead);

        if (m_handshakeConfirmed) {
            // 握手成功，采用当前串口
            serial = m_attemptSerial;       // 接管串口对象
            connect(serial, &QSerialPort::readyRead, this, &DetectionSystem::readSerialData);
            serial->moveToThread(this->thread()); // 确保线程正确
            updateStatus(QString("已连接到设备，串口: %1").arg(portName));
            heartbeatTimer->start(1000);    // 启动心跳

            qint64 timestampSec = QDateTime::currentSecsSinceEpoch();
            // 转换为 QDateTime 对象
            m_heartbeatTime = QDateTime::fromSecsSinceEpoch(timestampSec);

            // 可以触发初始化等其他操作
            onVersion();   // 可选，发送初始化指令
            break;
        } else {
            // 失败：关闭并删除临时串口

            //qDebug() << "关闭串口:" << m_attemptSerial->portName();
            m_attemptSerial->close();
            delete m_attemptSerial;
            m_attemptSerial = nullptr;
        }
    }

    // 清理事件循环指针
    m_loop = nullptr;

    if (!serial || !serial->isOpen()) {
        QTimer::singleShot(10, this, &DetectionSystem::autoConnectSerial);
        //QMessageBox::critical(this, "错误", "未找到可响应的设备，请检查连接后重试。");
        //exit(0);
    }
}

void DetectionSystem::onAttemptReadyRead()
{
    if (!m_attemptSerial) return;

    // 累积接收数据
    m_attemptBuffer.append(m_attemptSerial->readAll());

    // 查找完整帧（按照协议：帧头 FD，总长20字节，帧尾 55 AA 33 CC）
    // 简化：只需检测是否收到有效的 HELLO 响应
    // 完整应答帧格式： FD 10 00 ... 55 AA 33 CC
    if (m_attemptBuffer.size() >= 20) {
        // 寻找帧头 0xFD
        int idx = m_attemptBuffer.indexOf('\xFD');
        if (idx != -1 && m_attemptBuffer.size() >= idx + 20) {
            QByteArray frame = m_attemptBuffer.mid(idx, 20);
            // 检查命令是否为 0x10，状态为 0x00
            if (static_cast<quint8>(frame[1]) == 0x10 && static_cast<quint8>(frame[2]) == 0x00) {
                // 可选：校验尾部和校验和
                m_handshakeConfirmed = true;
            }
        }
    }

    if (m_handshakeConfirmed && m_loop) {
        m_loop->quit();   // 退出事件循环，停止等待
    }
}

void DetectionSystem::closeEvent(QCloseEvent* event)
{
    if (m_testUI && !m_testUI->confirmStopScanning()) {
        event->ignore();
        return;
    }

    if (m_testUI && !m_testUI->promptSaveScanData()) {
        event->ignore();
        return;
    }

    event->accept();
    exit(0);
}