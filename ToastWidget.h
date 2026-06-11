#ifndef TOASTWIDGET_H
#define TOASTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

class ToastWidget : public QWidget
{
    Q_OBJECT
public:
    // 单例模式，全局唯一，确保新提示覆盖旧提示
    static ToastWidget* instance(QWidget *parent = nullptr);

    // 显示提示文字，默认显示3秒
    void showToast(const QString &text, int msec = 2000);

private:
    explicit ToastWidget(QWidget *parent = nullptr);
    ~ToastWidget();

    // 初始化UI
    void initUI();

private slots:
    // 自动隐藏槽函数
    void onAutoHide();

private:
    static ToastWidget* m_instance;  // 单例对象
    QLabel* m_textLabel;             // 文字标签
    QTimer* m_hideTimer;             // 自动消失定时器
};

#endif // TOASTWIDGET_H