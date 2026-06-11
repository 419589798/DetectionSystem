#include "customintvalidator.h"
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QFontMetrics>
#include <cmath>
#include <QFileInfo>
#include <QDir>

HeatmapWidget::HeatmapWidget(QWidget *parent,int maxX,int maxY,int minX,int minY,int step,QColor darkColor,QColor lightColor)
    : QOpenGLWidget(parent)
    , m_textureId(0)
    , m_zoom(1.0)      // 新增
    , m_offset(0, 0)   // 新增
    , m_dragging(false)          // 新增
    , m_lastMousePos(0, 0)       // 新增
    , m_darkColor(0, 0, 0)        // 新增：默认黑色
    , m_lightColor(255, 255, 255) // 新增：默认白色
    , m_hoverX(0)           // 新增
    , m_hoverY(0)           // 新增
    , m_hoverValue(0)       // 新增
    , m_hasHover(false)     // 新增
{
 
    m_darkColor = darkColor;
    m_lightColor = lightColor;

    GRID_COLS = (maxX-minX)/step;   // 网格列数 (X方向)
    GRID_ROWS = (maxY-minY)/step;
  
    m_minX = minX;
    m_minY = minY;

    // 初始化数值数组 (使用相对坐标 0 到 GRID_COLS-1)
    m_gridValues.resize(GRID_COLS, std::vector<float>(GRID_ROWS, 0));

    m_colorImage = QImage(GRID_COLS, GRID_ROWS, QImage::Format_RGBA8888);
    updateAllColors();  // 新增：更新颜色

    //setGrayRange(m_darkColor, m_lightColor);

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

HeatmapWidget::~HeatmapWidget()
{
    makeCurrent();
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
    }
    doneCurrent();
}

void HeatmapWidget::setFocueValue(double iMin,double iMax)
{
    kFocusMinValue = iMin;
    kFocusMaxValue = iMax;
}

int HeatmapWidget::getGRID_COLS()
{
    return GRID_COLS;
}

int HeatmapWidget::getGRID_ROWS()
{
    return GRID_ROWS;
}

int HeatmapWidget::getMIN_X()
{
    return m_minX;
}

int HeatmapWidget::getMIN_Y()
{
    return m_minY;
}

QColor HeatmapWidget::valueToColor(float value) const
{
    

    if (value < kFocusMinValue) {
        double t = qBound(0.0, static_cast<double>(value) / kFocusMinValue, 1.0);
        int gray = 30 + static_cast<int>(t * 45.0);
        return QColor(gray, gray, gray + 20);
    }

    if (value > kFocusMaxValue) {
        double t = qBound(0.0, (static_cast<double>(value) - kFocusMaxValue) / (65535.0 - kFocusMaxValue), 1.0);
        int gray = 95 + static_cast<int>(t * 45.0);
        return QColor(gray + 20, gray, gray);
    }

    double t = (static_cast<double>(value) - kFocusMinValue) / (kFocusMaxValue - kFocusMinValue);

    struct ColorStop {
        double pos;
        QColor color;
    };

    static const ColorStop stops[] = {
        {0.00, QColor(0, 0, 180)},
        {0.25, QColor(0, 190, 255)},
        {0.50, QColor(0, 220, 0)},
        {0.75, QColor(255, 230, 0)},
        {1.00, QColor(255, 0, 0)}
    };

    for (int i = 0; i < 4; ++i) {
        if (t <= stops[i + 1].pos) {
            double localT = (t - stops[i].pos) / (stops[i + 1].pos - stops[i].pos);
            int r = stops[i].color.red() + static_cast<int>(localT * (stops[i + 1].color.red() - stops[i].color.red()));
            int g = stops[i].color.green() + static_cast<int>(localT * (stops[i + 1].color.green() - stops[i].color.green()));
            int b = stops[i].color.blue() + static_cast<int>(localT * (stops[i + 1].color.blue() - stops[i].color.blue()));
            return QColor(r, g, b);
        }
    }

    return stops[4].color;
}
void HeatmapWidget::setGridValue(int x, int y, float value)
{
    // 转换为相对坐标
    int rx = x - m_minX;
    int ry = y - m_minY;
    
    if (rx < 0 || rx >= GRID_COLS || ry < 0 || ry >= GRID_ROWS)
        return;
    
    //value = qBound(0, (int)value, 65535);
    m_gridValues[rx][ry] = value;
    updateColor(rx, ry);  // 传入相对坐标
    update();
}

float HeatmapWidget::gridValue(int x, int y) const
{
    int rx = x - m_minX;
    int ry = y - m_minY;
    if (rx < 0 || rx >= GRID_COLS || ry < 0 || ry >= GRID_ROWS)
        return 0;
    return m_gridValues[rx][ry];
}

void HeatmapWidget::setGrayRange(const QColor &darkColor, const QColor &lightColor)
{
    m_darkColor = darkColor;
    m_lightColor = lightColor;
    updateAllColors();
    update();
}

void HeatmapWidget::updateAllColors()
{
    for (int rx = 0; rx < GRID_COLS; ++rx) {
        for (int ry = 0; ry < GRID_ROWS; ++ry) {
            int imageRow = GRID_ROWS - 1 - ry;  // 相对坐标转图像行
            QColor color = valueToColor(m_gridValues[rx][ry]);
            m_colorImage.setPixelColor(rx, imageRow, color);
        }
    }
    updateTexture();
}

void HeatmapWidget::updateColor(int rx, int ry)
{
    int imageRow = GRID_ROWS - 1 - ry;
    QColor color = valueToColor(m_gridValues[rx][ry]);
    m_colorImage.setPixelColor(rx, imageRow, color);
    updateTextureRegion(rx, imageRow, 1, 1);
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

    // 设置纹理参数: 线性采样，让相邻格子的颜色过渡更柔和
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateGridRect();   // 替换原有的直接计算代码
}

void HeatmapWidget::paintGL()
{
    if (!isVisible())
        return;

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

    //隐藏网络边框
    /*glBegin(GL_LINES);
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
    glEnd();*/

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

    int step = 10;//每10个格子显示一个标签
    if(GRID_COLS>3000 || GRID_ROWS>3000)
        step = 100;
    else if(GRID_COLS>2000 || GRID_ROWS>2000)
        step = 50;
    else if(GRID_COLS>=1000 || GRID_ROWS>=1000)
        step = 20;

    // X轴刻度标签
    for (int i = 0; i <=  GRID_COLS; i += step) {
        float x = m_gridRect.left() + i * cellWidth;
        int value;
        if (i == 0)
            value = 0;
        else if (i == GRID_COLS-1)
            value = GRID_COLS;
        else
            value = i;   // i=10 显示 10，i=20 显示 20
        QString label = QString::number(value+m_minX);
        QFontMetrics fm(axisFont);
        int textWidth = fm.horizontalAdvance(label);
        painter.drawLine(x, xAxisY - 3, x, xAxisY + 3);
        painter.drawText(x - textWidth/2, xAxisY + 15, label);
    }

    // Y轴标签倒序：底部显示0，顶部显示99
    for (int i = 0; i <= GRID_ROWS; i += step) 
    {
        float y = m_gridRect.top() + i * cellHeight;
        int userY;
        if (i == GRID_ROWS)
            userY = (GRID_ROWS - i);
        else
            userY = (GRID_ROWS - i); // i=0时顶部显示100，i=100时底部显示1
        QString label = QString::number(userY+m_minY);
        painter.drawLine(yAxisX - 3, y, yAxisX + 3, y);
        painter.drawText(yAxisX - 25, y + 4, label);
    }

    // 额外显示一个标题和颜色值说明 
    painter.setPen(Qt::white);
    QString drawT = QString("网格 (%1x%2)").arg(QString::number(GRID_COLS)).arg(QString::number(GRID_ROWS));
    painter.drawText(10, 20, drawT);

    // 新增：绘制悬停提示框
    drawTooltip(painter);

    painter.end();
}

void HeatmapWidget::updateTexture()
{
    if (!m_textureId) return;

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    // 上传整个图像数据 (RGBA格式)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GRID_COLS,  GRID_ROWS, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_colorImage.constBits());
}

void HeatmapWidget::updateTextureRegion(int x, int y, int width, int height)
{
    if (!m_textureId) return;
    if (x < 0 || y < 0 || x + width > GRID_COLS || y + height > GRID_ROWS)
        return;

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    m_colorImage.constBits() + (y * GRID_COLS + x) * 4);
}

int HeatmapWidget::toImageRow(int userY) const
{
    // userY: 0 = 底部, 99 = 顶部
    // 图像行索引: 0 = 顶部, 99 = 底部
    int relativeY = userY - m_minY;
    return GRID_ROWS - 1 - relativeY;
}

int HeatmapWidget::toUserY(int imageRow) const
{
    int relativeY = GRID_ROWS - 1 - imageRow;
    return m_minY + relativeY;
}

void HeatmapWidget::wheelEvent(QWheelEvent *event)
{
    // 滚轮方向：角度增量 > 0 表示向上滚动（放大）
    double delta = event->angleDelta().y();
    double zoomFactor = 1.1;
    double newZoom = m_zoom;
    if (delta > 0) {
        newZoom *= zoomFactor;
    } else if (delta < 0) {
        newZoom /= zoomFactor;
    } else {
        return;
    }

    // 限制缩放范围
    const double MIN_ZOOM = 0.3;
    const double MAX_ZOOM = 100.0;
    if (newZoom < MIN_ZOOM || newZoom > MAX_ZOOM)
        return;

    // 获取鼠标位置（相对于窗口）
    QPointF mousePos = event->pos();
    
    // 获取当前网格矩形（缩放前）
    QRectF oldRect = m_gridRect;
    if (oldRect.isNull())
        return;
    
    // 计算鼠标在网格矩形中的相对位置（归一化，0~1）
    double relX = (mousePos.x() - oldRect.left()) / oldRect.width();
    double relY = (mousePos.y() - oldRect.top()) / oldRect.height();
    
    // 限制相对位置在 [0,1] 范围内
    relX = qBound(0.0, relX, 1.0);
    relY = qBound(0.0, relY, 1.0);
    
    // 更新缩放因子
    m_zoom = newZoom;
    
    // 重新计算网格矩形（基于当前窗口和新的缩放）
    updateGridRect();
    
    // 计算缩放后，为使鼠标点位置不变，需要调整偏移量
    QRectF newRect = m_gridRect;
    double newMouseX = newRect.left() + relX * newRect.width();
    double newMouseY = newRect.top() + relY * newRect.height();
    double dx = mousePos.x() - newMouseX;
    double dy = mousePos.y() - newMouseY;
    
    // 更新偏移量
    m_offset.rx() += dx;
    m_offset.ry() += dy;
    
    // 再次重新计算网格矩形，应用新的偏移
    updateGridRect();
    
    // 触发重绘
    update();
    
    event->accept();
}

void HeatmapWidget::updateGridRect()
{
    int w = width();
    int h = height();

    int availableWidth = w - m_leftMargin - m_rightMargin;
    int availableHeight = h - m_topMargin - m_bottomMargin;

    if (availableWidth <= 0 || availableHeight <= 0 || GRID_COLS <= 0 || GRID_ROWS <= 0) {
        m_gridRect = QRectF();
        return;
    }

    double cellSize = qMin(static_cast<double>(availableWidth) / GRID_COLS,
                           static_cast<double>(availableHeight) / GRID_ROWS);
    double baseWidth = GRID_COLS * cellSize;
    double baseHeight = GRID_ROWS * cellSize;
    double baseLeft = m_leftMargin + (availableWidth - baseWidth) / 2.0;
    double baseTop = m_topMargin + (availableHeight - baseHeight) / 2.0;

    double scaledWidth = baseWidth * m_zoom;
    double scaledHeight = baseHeight * m_zoom;
    double left = baseLeft + m_offset.x();
    double top = baseTop + m_offset.y();

    m_gridRect = QRectF(left, top, scaledWidth, scaledHeight);
}

void HeatmapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastMousePos = event->pos();
        event->accept();
        setCursor(Qt::ClosedHandCursor);
    } else {
        QOpenGLWidget::mousePressEvent(event);
    }
}

//void HeatmapWidget::mouseMoveEvent(QMouseEvent *event)
//{
//    if (m_dragging) {
//        QPointF delta = event->pos() - m_lastMousePos;
//        m_offset += delta;                     // 累加偏移
//        m_lastMousePos = event->pos();
//        updateGridRect();                      // 重新计算网格矩形
//        update();                              // 触发重绘
//        event->accept();
//    } else {
//        QOpenGLWidget::mouseMoveEvent(event);
//    }
//}

void HeatmapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
        setCursor(Qt::ArrowCursor);
    } else {
        QOpenGLWidget::mouseReleaseEvent(event);
    }
}

// ===================== 核心转换函数 =====================
QPixmap HeatmapWidget::convertToPixmap()
{
    // 必须在 OpenGL 上下文有效时调用
    makeCurrent();

    int w = width();
    int h = height();

    // 读取 OpenGL 帧缓冲像素（GL_FRONT 是前台缓冲）
    QImage image(w, h, QImage::Format_RGB888);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image.bits());

    // OpenGL 图像是上下颠倒的，必须翻转
    image = image.mirrored(false, true);

    // 转 QPixmap
    QPixmap pixmap = QPixmap::fromImage(image);

    // 释放 OpenGL 上下文
    doneCurrent();

    return pixmap;
}

bool HeatmapWidget::saveToPng(const QString &filePath, int cellPixels)
{
    QPixmap pix = convertToPixmap();
        
    QString path = filePath;
   
    // 保存到文件
    QDir dir;
    if (!dir.exists(QFileInfo(filePath).absolutePath())) {
        dir.mkpath(QFileInfo(filePath).absolutePath());
    }

    return pix.save(filePath, "PNG");
}

bool HeatmapWidget::windowPosToGridPos(const QPoint& pos, int& gridX, int& gridY) const
{
    // 检查鼠标是否在网格区域内
    if (!m_gridRect.contains(pos))
        return false;
    
    // 计算鼠标在网格中的相对位置 (0 到 1)
    double relX = (pos.x() - m_gridRect.left()) / m_gridRect.width();
    double relY = (pos.y() - m_gridRect.top()) / m_gridRect.height();
    
    // 边界检查
    if (relX < 0 || relX > 1 || relY < 0 || relY > 1)
        return false;
    
    // 计算网格索引
    int col = static_cast<int>(relX * GRID_COLS);
    int row = static_cast<int>(relY * GRID_ROWS);
    
    // 边界裁剪
    col = qBound(0, col, GRID_COLS - 1);
    row = qBound(0, row, GRID_ROWS - 1);
    
    // 转换为用户坐标
    gridX = m_minX + col;
    gridY = m_minY + (GRID_ROWS - 1 - row);  // 注意Y轴方向反转
    
    return true;
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 处理拖拽
    if (m_dragging) {
        QPointF delta = event->pos() - m_lastMousePos;
        m_offset += delta;
        m_lastMousePos = event->pos();
        updateGridRect();
        update();
        event->accept();
        return;
    }
    
    // 处理悬停显示
    int gridX, gridY;
    if (windowPosToGridPos(event->pos(), gridX, gridY)) {
        float value = gridValue(gridX, gridY);
        
        // 只有当网格变化时才更新
        if (!m_hasHover || m_hoverX != gridX || m_hoverY != gridY || m_hoverValue != value) {
            m_hoverX = gridX;
            m_hoverY = gridY;
            m_hoverValue = value;
            m_hasHover = true;
            update();  // 触发重绘以更新提示框
        }
    } else {
        if (m_hasHover) {
            m_hasHover = false;
            update();  // 清除提示框
        }
    }
    
    QOpenGLWidget::mouseMoveEvent(event);
}

void HeatmapWidget::leaveEvent(QEvent *event)
{
    // 鼠标离开窗口时清除悬停状态
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
    QOpenGLWidget::leaveEvent(event);
}

void HeatmapWidget::drawTooltip(QPainter& painter)
{
    if (!m_hasHover)
        return;
    
    // 准备提示文本
    QString tooltipText = QString("坐标: (%1, %2) 数值: %3")
                          .arg(m_hoverX)
                          .arg(m_hoverY)
                          .arg(m_hoverValue);
    
    // 计算提示框大小
    QFont tooltipFont("Arial", 10);
    painter.setFont(tooltipFont);
    QFontMetrics fm(tooltipFont);
    QRect textRect = fm.boundingRect(QRect(), Qt::TextSingleLine, tooltipText);
    textRect.adjust(-10, -8, 10, 8);  // 添加内边距
    
    // 获取鼠标当前位置（QCursor::pos() 是屏幕坐标，需要转换）
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    
    // 计算提示框位置（默认显示在鼠标右下方）
    int tipX = mousePos.x() + 15;
    int tipY = mousePos.y() + 15;
    
    // 边界检查，确保提示框不超出窗口
    if (tipX + textRect.width() > width())
        tipX = mousePos.x() - textRect.width() - 15;
    if (tipY + textRect.height() > height())
        tipY = mousePos.y() - textRect.height() - 15;
    
    // 绘制半透明背景
    painter.setBrush(QColor(30, 30, 30, 220));
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawRect(tipX, tipY, textRect.width(), textRect.height());
    
    // 绘制文本
    painter.setPen(Qt::white);
    painter.drawText(QRect(tipX + 5, tipY + 4, 
                          textRect.width(), 
                          textRect.height() - 8),
                    Qt::AlignLeft | Qt::AlignVCenter,
                    tooltipText);
}