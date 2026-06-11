#pragma once

#include <QWidget>
#include <QDateTime>
#include "ui_TestPage.h"
#include <QApplication>
#include <QVector>
#include <QColor>
#include <QBrush>
#include <QPen>

#include <QCloseEvent>
#include <QTimer>
#include "customintvalidator.h"

struct DataPoint {
    int x;      // 1 ~ 10
    int y;      // 1 ~ 10
    float value; // 用于映射颜色
};

#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QDateTime>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLabel>
#include <QVBoxLayout>

class CoordinateDiagram : public QWidget {
public:
    CoordinateDiagram(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(350, 350);
        setWindowTitle("坐标区域示意图");
        
        // 总面积边界 (100mm x 100mm)
        totalWidth = 100;
        totalHeight = 100;
        
        // 渲染区域边界（示例：从 0,0 到 50,50）
        renderMinX = 0;
        renderMinY = 0;
        renderMaxX = 50;
        renderMaxY = 50;
    }
    
    // 设置渲染区域
    void setRenderArea(int minX, int minY, int maxX, int maxY) {
        renderMinX = minX;
        renderMinY = minY;
        renderMaxX = maxX;
        renderMaxY = maxY;
        update();  // 触发重绘
    }
    
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 计算绘图区域（留出边距用于标注）
        int margin = 60;
        int drawWidth = width() - 2 * margin;
        int drawHeight = height() - 2 * margin;
        
        // 缩放比例（mm 到像素）
        double scaleX = (double)drawWidth / totalWidth;
        double scaleY = (double)drawHeight / totalHeight;
        
        int originX = margin;
        int originY = height() - margin;  // Y轴向上为正
        
        // ========== 1. 绘制总面积区域（紫色） ==========
        int totalRectWidth = (int)(totalWidth * scaleX);
        int totalRectHeight = (int)(totalHeight * scaleY);
        
        QRect totalRect(originX, originY - totalRectHeight, 
                        totalRectWidth, totalRectHeight);
        
        // 填充紫色（半透明）
        painter.fillRect(totalRect, QColor(128, 0, 128, 80));  // 半透明紫色
        painter.setPen(QPen(QColor(128, 0, 128), 2));
        painter.drawRect(totalRect);
        
        // ========== 2. 绘制渲染区域（绿色） ==========
        int renderX = originX + (int)(renderMinX * scaleX);
        int renderY = originY - (int)(renderMaxY * scaleY);
        int renderWidth = (int)((renderMaxX - renderMinX) * scaleX);
        int renderHeight = (int)((renderMaxY - renderMinY) * scaleY);
        
        QRect renderRect(renderX, renderY, renderWidth, renderHeight);
        
        // 填充绿色（半透明）
        painter.fillRect(renderRect, QColor(0, 255, 0, 100));
        painter.setPen(QPen(Qt::green, 2));
        painter.drawRect(renderRect);
        
        // ========== 3. 绘制坐标轴 ==========
        painter.setPen(QPen(Qt::black, 1.5));
        
        // X轴
        painter.drawLine(originX - 10, originY, 
                         originX + totalRectWidth + 20, originY);
        // Y轴
        painter.drawLine(originX, originY + 10, 
                         originX, originY - totalRectHeight - 20);
        
        // 箭头
        painter.drawLine(originX + totalRectWidth + 20, originY,
                         originX + totalRectWidth + 10, originY - 5);
        painter.drawLine(originX + totalRectWidth + 20, originY,
                         originX + totalRectWidth + 10, originY + 5);
        painter.drawLine(originX, originY - totalRectHeight - 20,
                         originX - 5, originY - totalRectHeight - 10);
        painter.drawLine(originX, originY - totalRectHeight - 20,
                         originX + 5, originY - totalRectHeight - 10);
        
        // ========== 4. 绘制刻度标签 ==========
        painter.setFont(QFont("Arial", 9));
        
        // X轴刻度（每10mm一个刻度）
        for (int x = 0; x <= totalWidth; x += 10) {
            int xPos = originX + (int)(x * scaleX);
            painter.drawLine(xPos, originY - 3, xPos, originY + 3);
            painter.drawText(xPos - 10, originY + 15, QString::number(x));
        }
        
        // Y轴刻度（每10mm一个刻度）
        for (int y = 0; y <= totalHeight; y += 10) {
            int yPos = originY - (int)(y * scaleY);
            painter.drawLine(originX - 3, yPos, originX + 3, yPos);
            painter.drawText(originX - 35, yPos + 4, QString::number(y));
        }
        
        // ========== 5. 添加文字标注 ==========
        painter.setFont(QFont("Arial", 11, QFont::Bold));
        
        // 总面积标注
        painter.setPen(QColor(128, 0, 128));
        painter.drawText(totalRect.topLeft().x() - 30, 
                         totalRect.topLeft().y() - 20,
                         "最大示意绘图区 (100mm×100mm)");
        
        // 渲染区域标注
        painter.setPen(Qt::green);
        painter.drawText(renderRect.center().x() - 40,
                         renderRect.center().y(),
                         "示意绘图区");
        
        // 坐标轴标签
        //painter.setPen(Qt::black);
        //painter.drawText(originX + totalRectWidth + 25, originY + 5, "X (mm)");
        //painter.drawText(originX - 25, originY - totalRectHeight - 25, "Y (mm)");
        
        // 原点标注
        //painter.drawText(originX - 15, originY + 15, "0,0");
        
        // 渲染区域坐标标注
        painter.setPen(Qt::darkGreen);
        QString coordText = QString("(%1, %2) 到 (%3, %4)")
                            .arg(renderMinX).arg(renderMinY)
                            .arg(renderMaxX).arg(renderMaxY);
        painter.drawText(renderRect.left(), renderRect.top() - 10, coordText);
    }
    
private:
    int totalWidth, totalHeight;    // 总面积尺寸 (mm)
    int renderMinX, renderMinY;     // 渲染区域最小坐标
    int renderMaxX, renderMaxY;     // 渲染区域最大坐标
};

class TestPage : public QWidget
{
	Q_OBJECT

public:
	TestPage(QWidget *parent = nullptr);
	~TestPage();

	
protected:
	void closeEvent(QCloseEvent *event) override;

private:
	Ui::TestPageClass ui;

	HeatmapWidget *m_heatmapWidget=NULL;
    CoordinateDiagram* m_diagram = NULL;
	QTimer *m_animationTimer;

	QVector<DataPoint> m_data;
	QVector<DataPoint> m_dataNew;

	int m_maxX;
    int m_maxY;
	int m_minX;
    int m_minY;
	int m_cellSize;
	int m_cellSpacing;

	QColor m_darkColor;              // 新增
    QColor m_lightColor;             // 新增

    double	m_kFocusMinValue = 0.0;		//  热力图最大值
    double	m_kFocusMaxValue = 0.0;		//  热力图最小值
    double m_minDeviation = 0.0;       // 热力图最小偏差
    double m_maxDeviation = 0.0;       // 热力图最大偏差
    double m_scanFrequency = 0.0;      // 仪器扫描频率

	bool m_animating=false;			 // 开始


    QDateTime m_scanStartTime;      // 开始扫描时间
    bool m_scanDataSaved = true;    // 当前扫描数据是否已保存

signals:
	void sigSendCmd(quint8 cmd, const QByteArray &params);
    void sigCloseTestPage(QWidget* pWidget);

private slots:
	void onScanStartClicked();
	void on_btnImg_clicked();
	void on_btnCSV_clicked();

	void onDarkColorClicked();       // 新增
    void onLightColorClicked();      // 新增

	void textChangeScanCoord(const QString &);
	void textChangeScanStep(const QString &);

	void returnPressedScanCoord();
    void returnPressedScanCoordNew();
	void returnPressedScanStep();

    void returnPressedRCount();

public slots:
	void returnXYValue(int x, int y, float value);
	void sigScanStep(QString text);
    void updateStatus(QString tip);
    void updateScanStatus(quint8 status);
    void updateScanFrequency(float frequency);

    void slot_heartStatue(QString tip);

public:
	QColor getGrayColor(int value);
	void exportToCSV(const QString& filename);
	void exportToIMG(const QString& filename);

	void setConfig(QString key, QString value);
	QString getConfigValue(QString key);

	QColor stringToColor(const QString& str);
	QString colorToString(const QColor& color);

	bool promptSaveScanData();
	bool exportScanDataWithDialog();
	bool isScanning() const;
    bool confirmStopScanning();
	void updateHeatmapRangeFromDeviation();
};

