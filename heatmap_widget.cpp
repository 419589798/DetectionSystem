#include "heatmap_widget.h"
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <cmath>

HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_textureId(0)
{
    // 初始化颜色图像: 100x100，初始颜色为深灰色
    m_colorImage = QImage(GRID_COLS, GRID_ROWS, QImage::Format_RGBA8888);
    m_colorImage.fill(QColor(50, 50, 50, 255)); // 初始颜色: 深灰色

    setFocusPolicy(Qt::StrongFocus);
}

HeatmapWidget::~HeatmapWidget()
{
    makeCurrent();
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
    }
    doneCurrent();
}

void HeatmapWidget::setGridColor(int x, int y, const QColor &color)
{
    if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS)
        return;

    // 更新图像数据 (注意: QImage坐标原点在左上角，与网格定义一致)
    m_colorImage.setPixelColor(x, y, color);

    // 更新纹理中的对应区域 (单个像素)
    updateTextureRegion(x, y, 1, 1);

    // 触发重绘
    update();
}

QColor HeatmapWidget::gridColor(int x, int y) const
{
    if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS)
        return QColor();
    return m_colorImage.pixelColor(x, y);
}

void HeatmapWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // 设置清除颜色 (深色背景)
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // 启用纹理
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST); // 2D网格不需要深度测试

    // 生成纹理
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // 设置纹理参数: 最近点采样，保证每个格子颜色锐利
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 上传初始纹理数据
    updateTexture();

    // 设置混合模式 (用于QPainter叠加，这里不必须，但为了安全)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void HeatmapWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    // 设置正交投影矩阵 (像素坐标系统，原点在左上角)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);  // Y轴向下为正，方便QPainter坐标对齐

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 计算网格显示区域 (留出边距用于坐标轴)
    int availableWidth = w - m_leftMargin - m_rightMargin;
    int availableHeight = h - m_topMargin - m_bottomMargin;

    if (availableWidth > 0 && availableHeight > 0) {
        // 网格区域矩形 (像素坐标)
        m_gridRect = QRectF(m_leftMargin, m_topMargin, availableWidth, availableHeight);
    } else {
        m_gridRect = QRectF(m_leftMargin, m_topMargin, w - m_leftMargin - m_rightMargin,
                            h - m_topMargin - m_bottomMargin);
    }
}

void HeatmapWidget::paintGL()
{
    // 清除颜色和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_gridRect.width() <= 0 || m_gridRect.height() <= 0)
        return;

    // ========== 1. 绘制带颜色纹理的网格区域 ==========
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // 设置颜色为白色，纹理颜色将直接显示
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 绘制带纹理的四边形 (使用纹理坐标映射网格)
    glBegin(GL_QUADS);
    // 左下角 (纹理坐标 (0, 1) 对应图像左上角? 需要调整)
    // 我们定义: 纹理坐标 (0,0) 对应图像左上角 (0行0列)
    // 纹理坐标 (1,1) 对应图像右下角 (99行99列)
    // 由于OpenGL纹理坐标原点在左下角，而QImage原点在左上角，但我们在纹理上传时没有翻转，
    // 为了让网格显示与逻辑坐标一致(第0行在最上方)，纹理坐标v=0应对应图像第0行(最上方)。
    // 因此顶点纹理坐标: 左上角(0,0)，右上角(1,0)，右下角(1,1)，左下角(0,1)
    float left = m_gridRect.left();
    float right = m_gridRect.right();
    float top = m_gridRect.top();
    float bottom = m_gridRect.bottom();

    // 左上角顶点
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(left, top);
    // 右上角顶点
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(right, top);
    // 右下角顶点
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(right, bottom);
    // 左下角顶点
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(left, bottom);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // ========== 2. 绘制网格线 (白色细线) ==========
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
    glLineWidth(1.0f);

    float cellWidth = m_gridRect.width() / GRID_COLS;
    float cellHeight = m_gridRect.height() / GRID_ROWS;

    glBegin(GL_LINES);
    // 绘制垂直线 (列边界)
    for (int i = 0; i <= GRID_COLS; ++i) {
        float x = m_gridRect.left() + i * cellWidth;
        glVertex2f(x, m_gridRect.top());
        glVertex2f(x, m_gridRect.bottom());
    }
    // 绘制水平线 (行边界)
    for (int i = 0; i <= GRID_ROWS; ++i) {
        float y = m_gridRect.top() + i * cellHeight;
        glVertex2f(m_gridRect.left(), y);
        glVertex2f(m_gridRect.right(), y);
    }
    glEnd();

    // ========== 3. 使用QPainter绘制坐标轴和标签 (叠加在OpenGL之上) ==========
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制坐标轴线
    QPen axisPen(Qt::white, 1.5);
    painter.setPen(axisPen);

    // X轴 (水平线)
    int xAxisY = m_gridRect.bottom() + 5;
    painter.drawLine(m_gridRect.left(), xAxisY, m_gridRect.right(), xAxisY);
    // Y轴 (垂直线)
    int yAxisX = m_gridRect.left() - 5;
    painter.drawLine(yAxisX, m_gridRect.top(), yAxisX, m_gridRect.bottom());

    // 绘制箭头 (简单三角形)
    painter.drawLine(m_gridRect.right(), xAxisY, m_gridRect.right() - 5, xAxisY - 3);
    painter.drawLine(m_gridRect.right(), xAxisY, m_gridRect.right() - 5, xAxisY + 3);
    painter.drawLine(yAxisX, m_gridRect.top(), yAxisX - 3, m_gridRect.top() + 5);
    painter.drawLine(yAxisX, m_gridRect.top(), yAxisX + 3, m_gridRect.top() + 5);
    // 绘制轴标签文字
    QFont axisFont("Arial", 10);
    painter.setFont(axisFont);
    painter.setPen(Qt::white);
    painter.drawText(m_gridRect.right() - 15, xAxisY - 5, "X");
    painter.drawText(yAxisX + 5, m_gridRect.top() + 10, "Y");

    // 绘制刻度标签 (每隔10个格子显示一个标签)
    painter.setPen(QPen(Qt::lightGray, 1));
    int step = 10;  // 每10个格子显示一个标签

    // X轴刻度标签
    for (int i = 0; i <= GRID_COLS; i += step) {
        float x = m_gridRect.left() + i * cellWidth;
        int value = i;
        QString label = QString::number(value);
        QFontMetrics fm(axisFont);
        int textWidth = fm.horizontalAdvance(label);
        painter.drawLine(x, xAxisY - 3, x, xAxisY + 3);
        painter.drawText(x - textWidth/2, xAxisY + 15, label);
    }

    // Y轴刻度标签 (注意Y轴方向: 网格第0行在顶部，所以坐标从上到下增加)
    for (int i = 0; i <= GRID_ROWS; i += step) {
        float y = m_gridRect.top() + i * cellHeight;
        int value = i;
        QString label = QString::number(value);
        painter.drawLine(yAxisX - 3, y, yAxisX + 3, y);
        painter.drawText(yAxisX - 25, y + 4, label);
    }

    // 额外显示一个标题和颜色值说明 (可选)
    painter.setPen(Qt::white);
    painter.drawText(10, 20, "Grid Heatmap (100x100)");
    painter.drawText(10, 40, "Click buttons to change cell color");

    painter.end();
}

void HeatmapWidget::updateTexture()
{
    if (!m_textureId) return;

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    // 上传整个图像数据 (RGBA格式)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRID_COLS, GRID_ROWS, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_colorImage.constBits());
}

void HeatmapWidget::updateTextureRegion(int x, int y, int width, int height)
{
    if (!m_textureId) return;
    if (x < 0 || y < 0 || x + width > GRID_COLS || y + height > GRID_ROWS)
        return;

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    // 更新指定矩形区域的数据
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    m_colorImage.constBits() + (y * GRID_COLS + x) * 4);
}