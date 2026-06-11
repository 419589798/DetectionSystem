#include "TestPage.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <windows.h>
#include <QColorDialog>
#include <QRandomGenerator>
#include "ToastWidget.h"
#include <QSettings>
#include <QToolTip>
#include <QDateTime>
#include <QMap>
#include <algorithm>
#include <functional>

#define DARK_COLOR "darkClork"
#define LIGHT_COLOR "lightClork"
#define MAX_COUNT "max_count"
#define FocusMinValue "focus_min_value"
#define FocusMaxValue "focus_max_value"

#pragma execution_character_set("utf-8")

extern QString g_maxCount;

static QString appendTimestampToFileName(const QString& filePath, const QString& timestamp)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix();
    QString stampedName = suffix.isEmpty()
        ? QString("%1_%2").arg(fileInfo.completeBaseName(), timestamp)
        : QString("%1_%2.%3").arg(fileInfo.completeBaseName(), timestamp, suffix);

    return fileInfo.dir().filePath(stampedName);
}
TestPage::TestPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    this->setWindowTitle("测试页面");

    if (getConfigValue(DARK_COLOR).isEmpty())
        m_darkColor = QColor(0, 0, 0);        //默认黑色
    else
        m_darkColor = stringToColor(getConfigValue(DARK_COLOR));
    
    if (getConfigValue(LIGHT_COLOR).isEmpty())
        m_lightColor = QColor(255, 255, 255); // 新增：默认白色
    else
        m_lightColor = stringToColor(getConfigValue(LIGHT_COLOR));

    // 颜色选择
    ui.m_darkColorButton->setStyleSheet(QString("background-color: %1").arg(m_darkColor.name()));
    ui.m_lightColorButton->setStyleSheet(QString("background-color: %1").arg(m_lightColor.name()));
    connect(ui.m_darkColorButton, &QPushButton::clicked, this, &TestPage::onDarkColorClicked);
    connect(ui.m_lightColorButton, &QPushButton::clicked, this, &TestPage::onLightColorClicked);

    // 扫描坐标
    QRegularExpression reg("[0-9]*");
    QRegularExpressionValidator* pValidator = new QRegularExpressionValidator(reg, this);
		
    //ui.lineEdit_maxCount->setValidator(pValidator);	
    QRegularExpression reg1("^[0-9]+(?:\\.[0-9]{1,3})?$");
    QRegularExpressionValidator* pValidator1 = new QRegularExpressionValidator(reg1, this);
    QRegularExpression deviationReg("^-?[0-9]+(?:\\.[0-9]{1,3})?$");
    QRegularExpressionValidator* pDeviationValidator = new QRegularExpressionValidator(deviationReg, this);
    ui.lineEdit_scanStep->setValidator(pValidator1);
    ui.lineEdit_start_X->setValidator(pValidator1);	
    ui.lineEdit_start_Y->setValidator(pValidator1);	
    ui.lineEdit_end_X->setValidator(pValidator1);	
    ui.lineEdit_end_Y->setValidator(pValidator1);
    ui.lineEdit_minCount->setValidator(pDeviationValidator);
    ui.lineEdit_maxCount->setValidator(pDeviationValidator);

	connect(ui.lineEdit_start_X, &QLineEdit::textChanged,this,&TestPage::textChangeScanCoord);
    connect(ui.lineEdit_start_Y, &QLineEdit::textChanged,this,&TestPage::textChangeScanCoord);
    connect(ui.lineEdit_end_X, &QLineEdit::textChanged,this,&TestPage::textChangeScanCoord);
    connect(ui.lineEdit_end_Y, &QLineEdit::textChanged,this,&TestPage::textChangeScanCoord);
    connect(ui.lineEdit_scanStep, &QLineEdit::textChanged,this,&TestPage::textChangeScanStep);
    connect(ui.lineEdit_start_X, &QLineEdit::returnPressed,this,&TestPage::returnPressedScanCoordNew);
    connect(ui.lineEdit_start_Y, &QLineEdit::returnPressed,this,&TestPage::returnPressedScanCoordNew);
    connect(ui.lineEdit_end_X, &QLineEdit::returnPressed,this,&TestPage::returnPressedScanCoordNew);
    connect(ui.lineEdit_end_Y, &QLineEdit::returnPressed,this,&TestPage::returnPressedScanCoordNew);
    connect(ui.lineEdit_scanStep, &QLineEdit::returnPressed,this,&TestPage::returnPressedScanStep);

    connect(ui.lineEdit_minCount, &QLineEdit::returnPressed, this, &TestPage::returnPressedRCount);
    connect(ui.lineEdit_maxCount, &QLineEdit::returnPressed, this, &TestPage::returnPressedRCount);

    // 开始扫描
    connect(ui.btnScanStart, &QPushButton::clicked, this, &TestPage::onScanStartClicked);
   
    // 动画定时器
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(1);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        static int gradientIndex = 0;
        if (!m_heatmapWidget)
            return;

        int cols = m_heatmapWidget->getGRID_COLS();
        int rows = m_heatmapWidget->getGRID_ROWS();
        if (cols <= 0 || rows <= 0)
            return;

        int col = gradientIndex % cols;
        int row = (gradientIndex / cols) % rows;
        int x = m_heatmapWidget->getMIN_X() + col;
        int y = m_heatmapWidget->getMIN_Y() + row;
        double tx = cols > 1 ? static_cast<double>(col) / (cols - 1) : 0.0;
        double ty = rows > 1 ? static_cast<double>(row) / (rows - 1) : 0.0;
        int value = static_cast<int>((tx * 0.7 + ty * 0.3) * 1000.0);

        DataPoint pt;
        pt.x = x;
        pt.y = y;
        pt.value = value;
        m_data.append(pt);

        m_heatmapWidget->setGridValue(x, y, value);
        gradientIndex = (gradientIndex + 1) % (cols * rows);
    });

    ui.widget_5->setVisible(false);
    ui.lineEdit_minCount->setText(getConfigValue(FocusMinValue));
    ui.lineEdit_maxCount->setText(getConfigValue(FocusMaxValue));
    m_minDeviation = ui.lineEdit_minCount->text().toDouble();
    m_maxDeviation = ui.lineEdit_maxCount->text().toDouble();
    updateHeatmapRangeFromDeviation();
    ui.btnCSV->setVisible(false);
}

TestPage::~TestPage()
{
    if (m_heatmapWidget)
        m_heatmapWidget->deleteLater();
}

void TestPage::onScanStartClicked()
{
    QByteArray params;

    if (m_animating) {
        confirmStopScanning();
    } else {
        if (!promptSaveScanData())
            return;

        returnPressedScanCoord();
        m_scanStartTime = QDateTime::currentDateTime();

        ui.btnScanStart->setText("停止扫描");
        m_animating = true;
        emit sigSendCmd(0x20, params);
    }
}


void TestPage::returnPressedScanCoordNew()
{
    double iMinXNew = ui.lineEdit_start_X->text().toDouble();
    double iMaxXNew = ui.lineEdit_end_X->text().toDouble();
    double iMinYNew = ui.lineEdit_start_Y->text().toDouble();
    double iMaxYNew = ui.lineEdit_end_Y->text().toDouble();

    if (iMinXNew > iMaxXNew || iMinYNew > iMaxYNew)
    {
        QMessageBox::critical(this, "错误", "起始坐标不能大于终止坐标");
        return;
    }

    if (ui.lineEdit_start_X->text().isEmpty() ||
        ui.lineEdit_end_X->text().isEmpty() ||
        ui.lineEdit_start_Y->text().isEmpty() ||
        ui.lineEdit_end_Y->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "坐标不能为空");
        return;
    }

    quint16 iMinX = iMinXNew * 100;
    quint16 iMaxX = iMaxXNew * 100;
    quint16 iMinY = iMinYNew * 100;
    quint16 iMaxY = iMaxYNew * 100;

    // 避免递归调用
    int valueZoom = 0;
    if (ui.lineEdit_scanStep->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "步进不能为空");
        return;
    }
    else
    {
        double value = ui.lineEdit_scanStep->text().toDouble();
        valueZoom = value * 100;
        int xZoom = iMaxX - iMinX;
        int yZoom = iMaxY - iMinY;


        int maxCount = g_maxCount.toInt();
        int scanCount = (xZoom / valueZoom) * (yZoom / valueZoom);
        if (maxCount < scanCount)
        {
            auto ret = QMessageBox::question(this,
                "采样数确认",
                QString("扫描点数超%1，扫描时间较长，是否继续？").arg(QString::number(maxCount)),
                QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::No) {
                return;
            }
        }

        if ((xZoom % valueZoom) != 0 || (yZoom % valueZoom) != 0)
        {
            QMessageBox::critical(this, "错误", "步进不能被扫描范围整除");
            return;
        }
    }

    if (m_heatmapWidget) {
        m_heatmapWidget->deleteLater();
        m_heatmapWidget = NULL;
    }

    m_heatmapWidget = new HeatmapWidget(this, iMaxX, iMaxY, iMinX, iMinY, valueZoom, m_darkColor, m_lightColor);
    m_heatmapWidget->setMinimumSize(800, 700);
    if (m_kFocusMinValue < m_kFocusMaxValue)
        m_heatmapWidget->setFocueValue(m_kFocusMinValue, m_kFocusMaxValue);
    ui.hLayout_charts->addWidget(m_heatmapWidget);

    //渲染示意图
    if (m_diagram)
    {
        m_diagram->deleteLater();
        m_diagram = NULL;
    }
    m_diagram = new CoordinateDiagram(this);
    // 可以动态修改渲染区域
    m_diagram->setRenderArea(iMinXNew, iMinYNew, iMaxXNew, iMaxYNew);
    ui.hLayout_map->addWidget(m_diagram);

    // 清除原有数据
    m_data.clear();
    m_scanDataSaved = true;
}

// 更新扫描坐标参数
void TestPage::returnPressedScanCoord()
{
    double iMinXNew = ui.lineEdit_start_X->text().toDouble();
    double iMaxXNew = ui.lineEdit_end_X->text().toDouble();
    double iMinYNew = ui.lineEdit_start_Y->text().toDouble();
    double iMaxYNew = ui.lineEdit_end_Y->text().toDouble();

    if (iMinXNew>iMaxXNew || iMinYNew>iMaxYNew)
    {
        QMessageBox::critical(this, "错误", "起始坐标不能大于终止坐标");
        return;
    }
    
    if (ui.lineEdit_start_X->text().isEmpty() || 
        ui.lineEdit_end_X->text().isEmpty()||
        ui.lineEdit_start_Y->text().isEmpty()||
        ui.lineEdit_end_Y->text().isEmpty()) 
    {
        QMessageBox::critical(this, "错误", "坐标不能为空");
        return ;
    }

    quint16 iMinX = iMinXNew * 100;
    quint16 iMaxX = iMaxXNew * 100;
    quint16 iMinY = iMinYNew * 100;
    quint16 iMaxY = iMaxYNew * 100;
    
	// 避免递归调用
    int valueZoom = 0;
    if (ui.lineEdit_scanStep->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "步进不能为空");
        return ;
    }
    else
    {
        double value = ui.lineEdit_scanStep->text().toDouble();
        valueZoom = value * 100;
        int xZoom = iMaxX - iMinX;
        int yZoom = iMaxY - iMinY;

        
        int maxCount = g_maxCount.toInt();
        int scanCount = (xZoom / valueZoom) * (yZoom / valueZoom);
        if (maxCount < scanCount)
        {
            auto ret = QMessageBox::question(this,
                "采样数确认",
                QString("扫描点数超%1，扫描时间较长，是否继续？").arg(QString::number(maxCount)),
                QMessageBox::Yes | QMessageBox::No);

            if (ret == QMessageBox::No) {
                return;
            }
        }

        if ((xZoom % valueZoom) != 0 ||(yZoom % valueZoom) != 0 )
        {
            QMessageBox::critical(this, "错误", "步进不能被扫描范围整除");
            return ;
        }

        // 设置步进
        quint16 iScanStep =static_cast<quint16>(valueZoom);
        QByteArray params;
        params.append(static_cast<char>(iScanStep >> 8));
        params.append(static_cast<char>(iScanStep & 0xFF));
        emit sigSendCmd(0x25, params);
    }

    QByteArray params;
    params.append(static_cast<char>(iMinX >> 8));
    params.append(static_cast<char>(iMinX & 0xFF));
    params.append(static_cast<char>(iMinY >> 8));
    params.append(static_cast<char>(iMinY & 0xFF));
    params.append(static_cast<char>(iMaxX >> 8));
    params.append(static_cast<char>(iMaxX & 0xFF));
    params.append(static_cast<char>(iMaxY >> 8));
    params.append(static_cast<char>(iMaxY & 0xFF));
    emit sigSendCmd(0x22, params);

    if (m_heatmapWidget) {
        m_heatmapWidget->deleteLater();
        m_heatmapWidget=NULL;
    }
        
    m_heatmapWidget = new HeatmapWidget(this,iMaxX,iMaxY,iMinX,iMinY,valueZoom,m_darkColor,m_lightColor);
    m_heatmapWidget->setMinimumSize(800, 700);
    if (m_kFocusMinValue < m_kFocusMaxValue)
        m_heatmapWidget->setFocueValue(m_kFocusMinValue, m_kFocusMaxValue);
    ui.hLayout_charts->addWidget(m_heatmapWidget);

    //渲染示意图
    if (m_diagram)
    {
        m_diagram->deleteLater();
        m_diagram=NULL;
    }
    m_diagram = new CoordinateDiagram(this);
    // 可以动态修改渲染区域
    m_diagram->setRenderArea(iMinXNew, iMinYNew, iMaxXNew, iMaxYNew);
    ui.hLayout_map->addWidget(m_diagram);

    // 清除原有数据
    m_data.clear();
    m_scanDataSaved = true;

    //m_animationTimer->stop();
    //m_animationTimer->start();
    /*for (int i = 0; i < 10; i++)
    {to
        for (int j = 0; j < 10; j++)
        {
            returnXYValue(i, j, i*j);
        }
    }*/
}


// 更新热力图范围
void TestPage::returnPressedRCount()
{
    // 避免递归调用
    if (ui.lineEdit_minCount->text().isEmpty()|| ui.lineEdit_maxCount->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "参数不能为空");
        return;
    }

    bool minOk = false;
    bool maxOk = false;
    double minDeviation = ui.lineEdit_minCount->text().toDouble(&minOk);
    double maxDeviation = ui.lineEdit_maxCount->text().toDouble(&maxOk);
    if (!minOk || !maxOk)
    {
        QMessageBox::critical(this, "错误", "参数格式错误");
        return;
    }

    qint64 minDeviation3 = static_cast<qint64>(minDeviation * 1000.0 + (minDeviation >= 0 ? 0.5 : -0.5));
    qint64 maxDeviation3 = static_cast<qint64>(maxDeviation * 1000.0 + (maxDeviation >= 0 ? 0.5 : -0.5));
    if (minDeviation3 >= maxDeviation3)
    {
        QMessageBox::critical(this, "错误", "最小偏差不能大于等于最大偏差");
        return;
    }

    m_minDeviation = minDeviation;
    m_maxDeviation = maxDeviation;

    setConfig(FocusMinValue, QString::number(m_minDeviation, 'f', 3));
    setConfig(FocusMaxValue, QString::number(m_maxDeviation, 'f', 3));
    ui.lineEdit_minCount->setText(QString::number(m_minDeviation, 'f', 3));
    ui.lineEdit_maxCount->setText(QString::number(m_maxDeviation, 'f', 3));

    updateHeatmapRangeFromDeviation();
}

void TestPage::updateHeatmapRangeFromDeviation()
{
    m_kFocusMinValue = m_scanFrequency + m_minDeviation;
    m_kFocusMaxValue = m_scanFrequency + m_maxDeviation;

    ui.lineEdit_heatmapMinValue->setText(QString::number(m_kFocusMinValue, 'f', 3));
    ui.lineEdit_heatmapMaxValue->setText(QString::number(m_kFocusMaxValue, 'f', 3));

    if (m_heatmapWidget && m_kFocusMinValue < m_kFocusMaxValue)
        m_heatmapWidget->setFocueValue(m_kFocusMinValue, m_kFocusMaxValue);
}

void TestPage::updateScanFrequency(float frequency)
{
    m_scanFrequency = frequency;
    updateHeatmapRangeFromDeviation();
}

// 更新步进参数
void TestPage::returnPressedScanStep()
{
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (pEdit->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "参数不能为空");
        return ;
    }

    double dScanStep = pEdit->text().toDouble();
    quint16 iScanStep =dScanStep*100;
    QByteArray params;
    params.append(static_cast<char>(iScanStep >> 8));
    params.append(static_cast<char>(iScanStep & 0xFF));

    emit sigSendCmd(0x25, params);
}

void TestPage::returnXYValue(int x, int y, float value)
{
    DataPoint pt;
    pt.x = x;
    pt.y = y;
    pt.value = value;
    m_data.append(pt);
    m_scanDataSaved = false;

    if(m_heatmapWidget)
        m_heatmapWidget->setGridValue(x,y,value);
}

void TestPage::sigScanStep(QString text)
{
    ui.lineEdit_scanStep->setText(text);
}

void TestPage::textChangeScanCoord(const QString& text)
{
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 100) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &TestPage::textChangeScanCoord);
        pEdit->setText("100");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &TestPage::textChangeScanCoord);
    }
}


void TestPage::textChangeScanStep(const QString& text)
{
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    double value = text.toDouble(&ok);
    if (ok && value*1000 > 10000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &TestPage::textChangeScanStep);
        pEdit->setText("10");
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "最大数值10");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &TestPage::textChangeScanStep);
    }
}

void TestPage::on_btnImg_clicked() {
    exportScanDataWithDialog();
}

void TestPage::on_btnCSV_clicked() {
    exportScanDataWithDialog();
}

bool TestPage::exportScanDataWithDialog()
{
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,                    // 父窗口
        "导出数据文件",              // 窗口标题
        "dataset.csv",              // 默认文件名
        "CSV文件 (*.csv);;文本文件 (*.txt)"  // 文件格式筛选
    );

    if (filePath.isEmpty())
        return false;

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    filePath = appendTimestampToFileName(filePath, timestamp);
    exportToCSV(filePath);

    QFileInfo fileInfo(filePath);
    QString imagePath = fileInfo.dir().filePath(QString("%1.png").arg(fileInfo.completeBaseName()));
    exportToIMG(imagePath);
    m_scanDataSaved = true;
    return true;
}
// 根据灰度值获取颜色（0黑色 -> 1000白色）
QColor TestPage::getGrayColor(int value) {
    int gray = value * 255 / 65535;  // 将0-1000映射到0-255
    return QColor(gray, gray, gray);
}

QColor TestPage::stringToColor(const QString &str)
{
    QColor color;
    QRegExp rx("rgba\\((\\d+),(\\d+),(\\d+),(\\d+)\\)");

    if (rx.exactMatch(str.trimmed())) {
        int r = rx.cap(1).toInt();
        int g = rx.cap(2).toInt();
        int b = rx.cap(3).toInt();
        int a = rx.cap(4).toInt();
        color = QColor(r, g, b, a);
    }
    return color;
}

// 颜色转 RGBA 字符串
QString TestPage::colorToString(const QColor & color)
{
    if (!color.isValid()) return "rgba(0,0,0,0)";
    return QString("rgba(%1,%2,%3,%4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

void TestPage::onDarkColorClicked()
{
    QColor color = QColorDialog::getColor(m_darkColor, this, "Select Dark Color");
    if (color.isValid()) {
        m_darkColor = color;
        ui.m_darkColorButton->setStyleSheet(QString("background-color: %1").arg(m_darkColor.name()));

        if(m_heatmapWidget)
            m_heatmapWidget->setGrayRange(m_darkColor, m_lightColor);

        setConfig(DARK_COLOR,colorToString(m_darkColor));
    }
}

void TestPage::onLightColorClicked()
{
    QColor color = QColorDialog::getColor(m_lightColor, this, "Select Light Color");
    if (color.isValid()) {
        m_lightColor = color;
        ui.m_lightColorButton->setStyleSheet(QString("background-color: %1").arg(m_lightColor.name()));

        if(m_heatmapWidget)
            m_heatmapWidget->setGrayRange(m_darkColor, m_lightColor);

        setConfig(LIGHT_COLOR,colorToString(m_lightColor));
    }
}

void TestPage::exportToCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建CSV文件");
        return;
    }

    file.write("\xEF\xBB\xBF");

    QTextStream stream(&file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
    //stream.setCodec("GBK");
#else
    stream.setEncoding(QStringConverter::Utf8);
#endif

    // UTF-8 BOM，防止 Excel/WPS 打开中文乱码
    stream << QChar(0xFEFF);

    QString startPos = QString("(%1，%2),")
        .arg(ui.lineEdit_start_X->text())
        .arg(ui.lineEdit_start_Y->text());

    QString endPos = QString("(%1，%2),")
        .arg(ui.lineEdit_end_X->text())
        .arg(ui.lineEdit_end_Y->text());

    QString stepP = QString("%1mm").arg(ui.lineEdit_scanStep->text());
    QString scanTime = (m_scanStartTime.isValid() ? m_scanStartTime : QDateTime::currentDateTime()).toString("yyyy-MM-dd hh:mm:ss");

    stream << QString::fromWCharArray(L"扫描时间,") << scanTime << ",";
    stream << QString::fromWCharArray(L"起始坐标,") << startPos << ",";
    stream << QString::fromWCharArray(L"截止坐标,") << endPos << ",";
    stream << QString::fromWCharArray(L"步进频率,") << stepP << "\n\n";

    QMap<QPair<int, int>, float> valueMap;
    QVector<int> xCoords;
    QVector<int> yCoords;

    for (const DataPoint& point : m_data) {
        QPair<int, int> key(point.x, point.y);
        valueMap[key] = point.value;

        if (!xCoords.contains(point.x))
            xCoords.append(point.x);
        if (!yCoords.contains(point.y))
            yCoords.append(point.y);
    }

    std::sort(xCoords.begin(), xCoords.end());
    std::sort(yCoords.begin(), yCoords.end(), std::greater<int>());

    for (int y : yCoords) {
        bool firstColumn = true;
        for (int x : xCoords) {
            if (!firstColumn)
                stream << ",";

            QPair<int, int> key(x, y);
            if (valueMap.contains(key))
                stream << valueMap.value(key);

            firstColumn = false;
        }
        stream << "\n";
    }

    file.close();

    QMessageBox::information(this, "成功", QString("数据已导出到: %1").arg(filename));
}

void TestPage::exportToIMG(const QString &filename)
{
    if (m_heatmapWidget)
    {
        QPixmap pixmap = m_heatmapWidget->grab();
        pixmap.save(filename, "PNG");
        QMessageBox::information(this, "成功", QString("图片已导出到: %1").arg(filename));   
    }
}

bool TestPage::promptSaveScanData()
{
    if (m_data.isEmpty() || m_scanDataSaved)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::question(
        this,
        "保存提醒",
        "当前扫描数据尚未保存，是否需要保存？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (ret == QMessageBox::Yes)
        return exportScanDataWithDialog();

    return true;
}
bool TestPage::isScanning() const
{
    return m_animating;
}
bool TestPage::confirmStopScanning()
{
    if (!m_animating)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::question(
        this,
        "提示",
        "当前正在扫描中，是否停止扫描？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (ret != QMessageBox::Yes)
        return false;

    QByteArray params;
    ui.btnScanStart->setText("开始扫描");
    m_animating = false;
    emit sigSendCmd(0x21, params);
    return true;
}
void TestPage::closeEvent(QCloseEvent *event)
{
    if (!confirmStopScanning()) {
        event->ignore();
        return;
    }

    if (!promptSaveScanData()) {
        event->ignore();
        return;
    }

    emit sigCloseTestPage(this);
    event->accept();
}

// 设置配置文件
void TestPage::setConfig(QString key,QString value)
{
  QSettings settings("Config", "TransSytem");
  settings.setValue(key, value);
}

// 获取配置文件
QString TestPage::getConfigValue(QString key)
{
  QSettings settings("Config", "TransSytem");
  return settings.value(key).toString();
}

void TestPage::updateStatus(QString tip)
{
    //ToastWidget::instance(this)->showToast(tip);
    ui.label_tip->setText(tip);
}

void TestPage::slot_heartStatue(QString tip)
{
    if (tip == "心跳连接失败！") {
        ui.btnScanStart->setEnabled(false);
        ui.btnImg->setEnabled(false);
    }  
    else {
        ui.btnScanStart->setEnabled(true);
        ui.btnImg->setEnabled(true);
    }

    ui.label_tipHeart->setText(tip);
}

void TestPage::updateScanStatus(quint8 status)
{
    QString tip;
    if (status == 0x00) {
        tip = ("状态：空闲中；");
    }
    else if (status == 0x01) {
        tip = ("状态：采集数据中");
    }
    else if (status == 0x02) {
        tip = ("状态：采集完成");
        if (m_animating) {
            QByteArray params;
            ui.btnScanStart->setText("开始扫描");
            m_animating = false;
            emit sigSendCmd(0x21, params);
        }

    }
    else if (status == 0x03) {
        tip = ("状态：采集异常停止");
        if (m_animating) {
            QByteArray params;
            ui.btnScanStart->setText("开始扫描");
            m_animating = false;
            emit sigSendCmd(0x21, params);
        }


    }
    ui.label_status->setText(tip);
}