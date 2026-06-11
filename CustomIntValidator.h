#ifndef CUSTOMINTVALIDATOR_H
#define CUSTOMINTVALIDATOR_H

#include <QIntValidator>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QImage>
#include <QColor>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPixmap>

#pragma execution_character_set("utf-8")

class HeatmapWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit HeatmapWidget(QWidget* parent = nullptr, int maxX = 1, int maxY = 1, int minX = 1, int minY = 1, int step = 2, QColor darkColor = QColor{0,0,0}, QColor lightColor=QColor{255,255,255});
    ~HeatmapWidget();

    bool saveToPng(const QString &filePath, int cellPixels = 4);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void updateTexture();           // 更新整个纹理到GPU
    void updateTextureRegion(int x, int y, int width, int height); // 部分更新纹理

    int toImageRow(int userY) const;   // 用户Y坐标 -> 图像行索引（顶部为0）
    int toUserY(int imageRow) const;   // 图像行索引 -> 用户Y坐标
private:
    int GRID_COLS;                  // 网格列数 (X方向)
    int GRID_ROWS;                  // 网格行数 (Y方向)
    int m_minX;                     // 起始X
    int m_minY;                     // 起始Y

    QImage m_colorImage;            // 存储格子颜色的图像 (RGBA格式)
    GLuint m_textureId;             // OpenGL纹理ID

    QRectF m_gridRect;              // 网格区域在widget中的矩形位置 (像素坐标)
    int m_leftMargin = 70;          // 左边距 (为Y轴标签留空间)
    int m_rightMargin = 20;         // 右边距
    int m_topMargin = 20;           // 上边距
    int m_bottomMargin = 40;        // 下边距 (为X轴标签留空间)

    // 新增：悬停相关的成员变量
    int m_hoverX;           // 当前悬停的网格X坐标（用户坐标）
    int m_hoverY;           // 当前悬停的网格Y坐标（用户坐标）
    float m_hoverValue;       // 当前悬停的网格数值
    bool m_hasHover;        // 是否有悬停的网格

// 在 HeatmapWidget 类的wheel
private:
    double m_zoom;              // 缩放因子，初始 1.0
    QPointF m_offset;           // 偏移量（像素），网格矩形左上角相对于默认位置的偏移

    void updateGridRect();      // 根据窗口尺寸、缩放和偏移重新计算网格矩形
    QPixmap convertToPixmap();
    
    void drawTooltip(QPainter& painter);// 新增：绘制提示框
    bool windowPosToGridPos(const QPoint& pos, int& gridX, int& gridY) const;// 新增：将窗口坐标转换为网格坐标
public:
    void setGridValue(int x, int y, float value);     // 设置数值 (0-65535)
    float gridValue(int x, int y) const;              // 获取数值
    void setGrayRange(const QColor &darkColor, const QColor &lightColor);  // 设置颜色范围
    bool getGridValueAtPos(const QPoint& pos, int& x, int& y, int& value) const;// 新增：获取鼠标位置对应的网格值

    void setFocueValue(double iMin, double iMax);
    int getGRID_COLS();
    int getGRID_ROWS();
    int getMIN_X();
    int getMIN_Y();
protected:
    void wheelEvent(QWheelEvent *event) override;   // 处理滚轮事件
    void leaveEvent(QEvent *event) override;

private:
    bool m_dragging;            // 是否正在拖拽
    QPointF m_lastMousePos;     // 上一次鼠标位置

    //int m_gridValues[5000][5000];                    // 存储数值
    std::vector<std::vector<float>> m_gridValues;      // 存储数值
    QColor m_darkColor;                              // 最深色
    QColor m_lightColor;                             // 最浅色
    
    void updateAllColors();                          // 更新所有颜色
    void updateColor(int x, int y);                  // 更新单个颜色
    QColor valueToColor(float value) const;            // 数值转颜色

    // 区间增强热力图：重点区间内颜色变化明显，区间外颜色弱化显示。
    // 根据现场数据分布调整这两个值即可。
    double kFocusMinValue =1000;
    double kFocusMaxValue =3000;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};


#endif // CUSTOMINTVALIDATOR_H