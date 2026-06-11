#ifndef HEATMAP_H
#define HEATMAP_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QImage>
#include <QColor>

class HeatmapWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit HeatmapWidget(QWidget *parent = nullptr);
    ~HeatmapWidget();

    // 设置网格中指定格子的颜色 (x: 0-99, y: 0-99)
    void setGridColor(int x, int y, const QColor &color);

    // 获取指定格子的颜色
    QColor gridColor(int x, int y) const;

    int GRID_COLS = 100;
    int GRID_ROWS = 100;
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void updateTextureRegion(int x, int y, int width, int height); // 部分更新纹理

private:
//    static constexpr int GRID_COLS = 100;   // 网格列数 (X方向)
//    static constexpr int GRID_ROWS = 100;   // 网格行数 (Y方向)

    QImage m_colorImage;            // 存储格子颜色的图像 (RGBA格式)
    GLuint m_textureId;             // OpenGL纹理ID

    QRectF m_gridRect;              // 网格区域在widget中的矩形位置 (像素坐标)
    int m_leftMargin = 70;          // 左边距 (为Y轴标签留空间)
    int m_rightMargin = 20;         // 右边距
    int m_topMargin = 20;           // 上边距
    int m_bottomMargin = 40;        // 下边距 (为X轴标签留空间)
};

#endif 