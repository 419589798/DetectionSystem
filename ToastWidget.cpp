#include "ToastWidget.h"
#include <QGraphicsOpacityEffect>
#include <QDesktopWidget>
#include <QApplication>

// 静态单例初始化
ToastWidget* ToastWidget::m_instance = nullptr;

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

ToastWidget::~ToastWidget()
{
}

// 单例获取函数（全局唯一）
ToastWidget* ToastWidget::instance(QWidget *parent)
{
    if (!m_instance) {
        m_instance = new ToastWidget(parent);
    }
    return m_instance;
}

// 初始化UI：透明、圆角、居中、无边框
void ToastWidget::initUI()
{
    // 窗口设置：无边框、始终置顶、透明背景
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);  // 透明背景
    this->setAttribute(Qt::WA_DeleteOnClose, false);  // 关闭不销毁，复用

    // 布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 12, 20, 12);  // 内边距
    layout->setAlignment(Qt::AlignCenter);

    // 文字标签
    m_textLabel = new QLabel();
    m_textLabel->setMinimumSize(300,50);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 20px;
        }
    )");
    layout->addWidget(m_textLabel);

    // 自动消失定时器
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);  // 只触发一次
    connect(m_hideTimer, &QTimer::timeout, this, &ToastWidget::onAutoHide);

    // 设置透明效果（可选，更柔和）
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.9);
    this->setGraphicsEffect(effect);

    // 背景样式：半透明黑 + 圆角
    this->setStyleSheet(R"(
        QWidget {
            background-color: rgba(0, 0, 0, 0.7);
            border-radius: 5px;
        }
    )");
}

// 显示提示文字（新提示覆盖旧提示，重新计时）
void ToastWidget::showToast(const QString &text, int msec)
{
    if (text.isEmpty()) return;

    // 停止之前的定时器，重新计时
    if (m_hideTimer->isActive()) {
        m_hideTimer->stop();
    }

    // 更新文字
    m_textLabel->setText(text);

    // 自适应文字大小
    this->adjustSize();

    // 居中显示在父窗口/屏幕中央
    if (this->parentWidget()) {
        QPoint centerPos = this->parentWidget()->geometry().center();
        this->move(centerPos.x() - this->width() / 2, centerPos.y() - this->height() / 2);
    } else {
        this->move(QApplication::desktop()->rect().center() - this->rect().center());
    }

    // 显示并启动定时器
    this->show();
    m_hideTimer->start(msec);
}

// 自动隐藏
void ToastWidget::onAutoHide()
{
    this->hide();
}
