#include "DebugPage.h"
#include <QIntValidator> 
#include <QDoubleValidator> 
#include <QToolTip> 
#include "ToastWidget.h"
#include <QSettings>

#define MAX_COUNT "max_count"

extern QString g_maxCount;
DebugPage::DebugPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    this->setWindowTitle("调试页面");

    g_maxCount = getConfigValue(MAX_COUNT);
    ui.lineEdit_maxCount->setText(g_maxCount);

	initFilter();
}

DebugPage::~DebugPage()
{}


void DebugPage::DebugPage::initFilter()
{
    QRegularExpression reg1("^[0-9]+(?:\\.[0-9]{1,7})?$");
    QRegularExpressionValidator* pValidator1 = new QRegularExpressionValidator(reg1, this);


	QRegularExpression reg("[0-9]*");
    QRegularExpressionValidator* pValidator = new QRegularExpressionValidator(reg, this);

    ui.lineEdit_maxCount->setValidator(pValidator);		//XHOME1
	connect(ui.lineEdit_maxCount, &QLineEdit::textChanged,this,&DebugPage::textChangeMaxCount);
    connect(ui.lineEdit_maxCount, &QLineEdit::returnPressed,this,&DebugPage::returnPressedMaxCount);

    ui.lineEdit_xhome1->setValidator(new QIntValidator(-300000, 300000, this));		//XHOME1
	connect(ui.lineEdit_xhome1, &QLineEdit::textChanged,this,&DebugPage::textChangeXHOME);
    connect(ui.lineEdit_xhome1, &QLineEdit::returnPressed,this,&DebugPage::returnPressedXHOME);

    ui.lineEdit_xhome2->setValidator(new QIntValidator(-300000, 300000, this));		//XHOME2
	connect(ui.lineEdit_xhome2, &QLineEdit::textChanged,this,&DebugPage::textChangeXHOME);
    connect(ui.lineEdit_xhome2, &QLineEdit::returnPressed,this,&DebugPage::returnPressedXHOME);

    ui.lineEdit_xhome3->setValidator(new QIntValidator(-300000, 300000, this));		//XHOME3
	connect(ui.lineEdit_xhome3, &QLineEdit::textChanged,this,&DebugPage::textChangeXHOME);
    connect(ui.lineEdit_xhome3, &QLineEdit::returnPressed,this,&DebugPage::returnPressedXHOME);

    ui.lineEdit_yhome1->setValidator(new QIntValidator(-300000, 300000, this));		//YHOME1
	connect(ui.lineEdit_yhome1, &QLineEdit::textChanged,this,&DebugPage::textChangeYHOME);
    connect(ui.lineEdit_yhome1, &QLineEdit::returnPressed,this,&DebugPage::returnPressedYHOME);

    ui.lineEdit_yhome2->setValidator(new QIntValidator(-300000, 300000, this));		//YHOME2
	connect(ui.lineEdit_yhome2, &QLineEdit::textChanged,this,&DebugPage::textChangeYHOME);
    connect(ui.lineEdit_yhome2, &QLineEdit::returnPressed,this,&DebugPage::returnPressedYHOME);

    ui.lineEdit_yhome3->setValidator(new QIntValidator(-300000, 300000, this));		//YHOME3
	connect(ui.lineEdit_yhome3, &QLineEdit::textChanged,this,&DebugPage::textChangeYHOME);
    connect(ui.lineEdit_yhome3, &QLineEdit::returnPressed,this,&DebugPage::returnPressedYHOME);
    
	ui.lineEdit_xspeed->setValidator(pValidator);		//X回零速度
	connect(ui.lineEdit_xspeed, &QLineEdit::textChanged,this,&DebugPage::textChangeXSpeed300);
    connect(ui.lineEdit_xspeed, &QLineEdit::returnPressed,this,&DebugPage::returnPressedXSpeed300);
    
	ui.lineEdit_yspeed->setValidator(pValidator);		//Y回零速度
	connect(ui.lineEdit_yspeed, &QLineEdit::textChanged,this,&DebugPage::textChangeYSpeed300);
    connect(ui.lineEdit_yspeed, &QLineEdit::returnPressed,this,&DebugPage::returnPressedYSpeed300);
	
	ui.lineEdit_mfreq->setValidator(pValidator);		//调制频率
	connect(ui.lineEdit_mfreq, &QLineEdit::textChanged,this,&DebugPage::textChangeMFreq50000);
    connect(ui.lineEdit_mfreq, &QLineEdit::returnPressed,this,&DebugPage::returnPressedMFreq50000);
	ui.lineEdit_bfreq->setValidator(pValidator);		//带通频率
	connect(ui.lineEdit_bfreq, &QLineEdit::textChanged,this,&DebugPage::textChangeBFreq50000);
    connect(ui.lineEdit_bfreq, &QLineEdit::returnPressed,this,&DebugPage::returnPressedBFreq50000);
	ui.lineEdit_phase->setValidator(pValidator);		//相位补充
	connect(ui.lineEdit_phase, &QLineEdit::textChanged,this,&DebugPage::textChangePhase360);
    connect(ui.lineEdit_phase, &QLineEdit::returnPressed,this,&DebugPage::returnPressedPhase360);
	
	ui.lineEdit_tfilter->setValidator(pValidator);		//滤波周期
	connect(ui.lineEdit_tfilter, &QLineEdit::textChanged,this,&DebugPage::textChangeTFilter1000);
    connect(ui.lineEdit_tfilter, &QLineEdit::returnPressed,this,&DebugPage::returnPressedTFilter1000);
	ui.lineEdit_dfilter->setValidator(pValidator);		//滤波深度
	connect(ui.lineEdit_dfilter, &QLineEdit::textChanged,this,&DebugPage::textChangeDFilter1000);
    connect(ui.lineEdit_dfilter, &QLineEdit::returnPressed,this,&DebugPage::returnPressedDFilter1000);
	ui.lineEdit_filtera->setValidator(pValidator);		//滤波备用
	connect(ui.lineEdit_filtera, &QLineEdit::textChanged,this,&DebugPage::textChangeFilterA1000);
    connect(ui.lineEdit_filtera, &QLineEdit::returnPressed,this,&DebugPage::returnPressedFilterA1000);
	ui.lineEdit_filterb->setValidator(pValidator);		//滤波备用
	connect(ui.lineEdit_filterb, &QLineEdit::textChanged,this,&DebugPage::textChangeFilterB1000);
    connect(ui.lineEdit_filterb, &QLineEdit::returnPressed,this,&DebugPage::returnPressedFilterB1000);

	
    ui.lineEdit_sfreq->setValidator(new QIntValidator(0, 100000, this));	//扫频单位
	connect(ui.lineEdit_sfreq, &QLineEdit::textChanged,this,&DebugPage::textChangeSFreq1000);
    connect(ui.lineEdit_sfreq, &QLineEdit::returnPressed,this,&DebugPage::returnPressedSFreq1000);
	ui.lineEdit_freq_delay->setValidator(pValidator);	//扫频间隔
	connect(ui.lineEdit_freq_delay, &QLineEdit::textChanged,this,&DebugPage::textChangeSFreq_Delay1000);
    connect(ui.lineEdit_freq_delay, &QLineEdit::returnPressed,this,&DebugPage::returnPressedSFreq_Delay1000);
	ui.lineEdit_freq_start->setValidator(new QDoubleValidator(0.0, 40000.0, 1, this));	//起始频率
	connect(ui.lineEdit_freq_start, &QLineEdit::textChanged,this,&DebugPage::textChangeSFreq_Start5000);
    connect(ui.lineEdit_freq_start, &QLineEdit::returnPressed,this,&DebugPage::returnPressedSFreq_Start5000);
	ui.lineEdit_freq_cutoff->setValidator(new QDoubleValidator(0.0, 40000.0, 1, this));	//截止频率
	connect(ui.lineEdit_freq_cutoff, &QLineEdit::textChanged,this,&DebugPage::textChangeSFreq_Cutoff5000);
    connect(ui.lineEdit_freq_cutoff, &QLineEdit::returnPressed,this,&DebugPage::returnPressedSFreq_Cutoff5000);
	
	ui.lineEdit_pidp->setValidator(pValidator1);
	//connect(ui.lineEdit_pidp, &QLineEdit::textChanged,this,&DebugPage::textChangePIDP100);
    connect(ui.lineEdit_pidp, &QLineEdit::returnPressed,this,&DebugPage::returnPressedPID100);
	ui.lineEdit_pidi->setValidator(pValidator1);
	//connect(ui.lineEdit_pidi, &QLineEdit::textChanged,this,&DebugPage::textChangePIDI100);
    connect(ui.lineEdit_pidi, &QLineEdit::returnPressed,this,&DebugPage::returnPressedPID100);
	ui.lineEdit_pidd->setValidator(pValidator1);
	//connect(ui.lineEdit_pidd, &QLineEdit::textChanged,this,&DebugPage::textChangePIDD100);
    connect(ui.lineEdit_pidd, &QLineEdit::returnPressed,this,&DebugPage::returnPressedPID100);
	ui.lineEdit_tpid->setValidator(pValidator);
	connect(ui.lineEdit_tpid, &QLineEdit::textChanged,this,&DebugPage::textChangeTPID1000);
    connect(ui.lineEdit_tpid, &QLineEdit::returnPressed,this,&DebugPage::returnPressedTPID1000);
	
	ui.lineEdit_scanSpeed->setValidator(pValidator);	//扫描速度
	connect(ui.lineEdit_scanSpeed, &QLineEdit::textChanged,this,&DebugPage::textChangeScanSpeed300);
    connect(ui.lineEdit_scanSpeed, &QLineEdit::returnPressed,this,&DebugPage::returnPressedScanSpeed300);
	ui.lineEdit_scanDelay->setValidator(pValidator);	//扫描间隔
	connect(ui.lineEdit_scanDelay, &QLineEdit::textChanged,this,&DebugPage::textChangeScanDelay10000);
    connect(ui.lineEdit_scanDelay, &QLineEdit::returnPressed,this,&DebugPage::returnPressedScanDelay10000);

    connect(ui.radioBtnAgainScan, &QRadioButton::clicked,this,&DebugPage::onClickFBAScan);
    connect(ui.radioBtnFScan, &QRadioButton::clicked,this,&DebugPage::onClickFBAScan);
    connect(ui.radioBtnBScan, &QRadioButton::clicked,this,&DebugPage::onClickFBAScan);
}

void DebugPage::textChangeMaxCount(const QString& text)
{
    return;//暂不启用
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 10000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXSpeed300);
        pEdit->setText("10000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXSpeed300);
    }
}
void DebugPage::textChangeXHOME(const QString& text)
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty() || text == "-") return;

    bool ok;
    int value = text.toInt(&ok);
    if (!ok) return;

    int boundedValue = qBound(-30000, value, 30000);
    if (value != boundedValue) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXHOME);
        pEdit->setText(QString::number(boundedValue));
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXHOME);
    }
}
void DebugPage::textChangeYHOME(const QString& text)
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty() || text == "-") return;

    bool ok;
    int value = text.toInt(&ok);
    if (!ok) return;

    int boundedValue = qBound(-30000, value, 30000);
    if (value != boundedValue) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeYHOME);
        pEdit->setText(QString::number(boundedValue));
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeYHOME);
    }
}
void DebugPage::textChangeXSpeed300(const QString& text)
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 300) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXSpeed300);
        pEdit->setText("300");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeXSpeed300);
    }
}
void DebugPage::textChangeYSpeed300(const QString& text)
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 300) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeYSpeed300);
        pEdit->setText("300");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeYSpeed300);
    }
}
void DebugPage::textChangeMFreq50000(const QString& text) 
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 50000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeMFreq50000);
        pEdit->setText("50000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeMFreq50000);
    }
}
void DebugPage::textChangeBFreq50000(const QString& text)
{
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 50000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeBFreq50000);
        pEdit->setText("50000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeBFreq50000);
    }
}
void DebugPage::textChangePhase360(const QString& text) {
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value > 360) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePhase360);
        pEdit->setText("360");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePhase360);
    }
}
void DebugPage::textChangeTFilter1000(const QString& text){
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeTFilter1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeTFilter1000);
    }
}
void DebugPage::textChangeDFilter1000(const QString& text){
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeDFilter1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeDFilter1000);
    }
}
void DebugPage::textChangeFilterA1000(const QString& text){
	QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeFilterA1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeFilterA1000);
    }
}
void DebugPage::textChangeFilterB1000(const QString& text){
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeFilterB1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeFilterB1000);
    }
}
void DebugPage::textChangeSFreq1000(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);

    int boundedValue = qBound(0, value, 10000);
    if (value != boundedValue){
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq1000);
    }
}
void DebugPage::textChangeSFreq_Delay1000(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Delay1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Delay1000);
    }
}
void DebugPage::textChangeSFreq_Start5000(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    double value = text.toDouble(&ok);
    if (ok && value > 4000.0) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Start5000);
        pEdit->setText("4000.0");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Start5000);
    }
}
void DebugPage::textChangeSFreq_Cutoff5000(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    double value = text.toDouble(&ok);
    if (ok && value > 4000.0) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Cutoff5000);
        pEdit->setText("4000.0");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeSFreq_Cutoff5000);
    }
}	
void DebugPage::textChangePIDP100(const QString& text){
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >100) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDP100);
        pEdit->setText("100");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDP100);
    }
}
void DebugPage::textChangePIDI100(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >100) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDI100);
        pEdit->setText("100");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDI100);
    }
}
void DebugPage::textChangePIDD100(const QString& text){
     QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >100) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDD100);
        pEdit->setText("100");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangePIDD100);
    }
}
void DebugPage::textChangeTPID1000(const QString& text){
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >1000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeTPID1000);
        pEdit->setText("1000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeTPID1000);
    }
}
void DebugPage::textChangeScanSpeed300(const QString& text){
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >300) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeScanSpeed300);
        pEdit->setText("300");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeScanSpeed300);
    }
}
void DebugPage::textChangeScanDelay10000(const QString& text){
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(sender());
	// 避免递归调用
    if (text.isEmpty()) return;

    bool ok;
    int value = text.toInt(&ok);
    if (ok && value >10000) {
        // 临时断开信号，防止修改文本时再次触发
        disconnect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeScanDelay10000);
        pEdit->setText("10000");
        // 重新连接
        connect(pEdit, &QLineEdit::textChanged, this, &DebugPage::textChangeScanDelay10000);
    }
}

/// <summary>
/// 以下是下传指令方法
/// </summary>
/// 
/// 通用方法
bool DebugPage::parseFun(QLineEdit* pEdit,QByteArray &params) {
    if (pEdit->text().isEmpty()) 
    {
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "参数不能为空");
        return false;
    }

    quint16 speed = pEdit->text().toInt();
    params.append(static_cast<char>(speed >> 8));
    params.append(static_cast<char>(speed & 0xFF));

    return true;
}

bool DebugPage::parseSignedHome(QLineEdit* pEdit, QByteArray &params) {
    if (pEdit->text().isEmpty() || pEdit->text() == "-")
    {
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "参数不能为空");
        return false;
    }

    bool ok;
    int value = pEdit->text().toInt(&ok);
    if (!ok || value < -30000 || value > 30000) {
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "参数范围为-30000到30000");
        return false;
    }

    quint16 rawValue = static_cast<quint16>(static_cast<qint16>(value));
    params.append(static_cast<char>(rawValue >> 8));
    params.append(static_cast<char>(rawValue & 0xFF));

    return true;
}

bool DebugPage::parseFreqTenths(QLineEdit* pEdit, QByteArray &params) {
    if (pEdit->text().isEmpty())
    {
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "参数不能为空");
        return false;
    }

    bool ok;
    double value = pEdit->text().toDouble(&ok);
    if (!ok || value < 0.0 || value > 4000.0) {
        QToolTip::showText(pEdit->mapToGlobal(pEdit->pos()), "参数范围为0.0到4000.0");
        return false;
    }

    quint16 rawValue = static_cast<quint16>(qRound(value * 10.0));
    params.append(static_cast<char>(rawValue >> 8));
    params.append(static_cast<char>(rawValue & 0xFF));

    return true;
}
void DebugPage::returnPressedMaxCount(){

    QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
    setConfig(MAX_COUNT,pEdit->text());
}
void DebugPage::returnPressedXHOME(){
    QByteArray params;
    quint8 home;
    QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
    if(pEdit == ui.lineEdit_xhome1)
        home = static_cast<quint8>(1);
    else if(pEdit == ui.lineEdit_xhome2)
        home = static_cast<quint8>(2);
    else
        home = static_cast<quint8>(3);
    params.append(home);

    if(parseSignedHome(pEdit, params))
        emit sigSendCmd(0x30, params);
}

void DebugPage::returnPressedYHOME(){
    QByteArray params;
    quint8 home;
    QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
    if(pEdit == ui.lineEdit_yhome1)
        home = static_cast<quint8>(1);
    else if(pEdit == ui.lineEdit_yhome2)
        home = static_cast<quint8>(2);
    else
        home = static_cast<quint8>(3);
    params.append(home);

    if(parseSignedHome(pEdit, params))
        emit sigSendCmd(0x31, params);
}
void DebugPage::returnPressedXSpeed300(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x32, params);
}
void DebugPage::returnPressedYSpeed300(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x33, params);
}
void DebugPage::returnPressedMFreq50000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x34, params);
}
void DebugPage::returnPressedBFreq50000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x35, params);
}
void DebugPage::returnPressedPhase360(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x36, params);
}
void DebugPage::returnPressedTFilter1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x38, params);
}
void DebugPage::returnPressedDFilter1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x39, params);
}
void DebugPage::returnPressedFilterA1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x3A, params);
}
void DebugPage::returnPressedFilterB1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x3B, params);
}
void DebugPage::returnPressedSFreq1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x40, params);
}
void DebugPage::returnPressedSFreq_Delay1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x41, params);
}
void DebugPage::returnPressedSFreq_Start5000(){
    QByteArray params;
    if(parseFreqTenths(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x42, params);
}
void DebugPage::returnPressedSFreq_Cutoff5000(){
    QByteArray params;
    if(parseFreqTenths(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x43, params);
}
void DebugPage::returnPressedPID100(){
    if (ui.lineEdit_pidp->text().isEmpty() ||
        ui.lineEdit_pidi->text().isEmpty() ||
        ui.lineEdit_pidd->text().isEmpty()) 
    {
        QToolTip::showText(ui.lineEdit_pidp->mapToGlobal(ui.lineEdit_pidp->pos()), "PID参数不能为空");
        return;
    }
    float valuep = ui.lineEdit_pidp->text().toFloat();
    float valuei = ui.lineEdit_pidi->text().toFloat();
    float valued = ui.lineEdit_pidd->text().toFloat();

    QByteArray params;
    appendFloat(params, valuep);
    appendFloat(params, valuei);
    appendFloat(params, valued);

    emit sigSendCmd(0x26, params);
}

void DebugPage::appendFloat(QByteArray& frame, float value)
{
    frame.append(reinterpret_cast<const char*>(&value), 4);
}
void DebugPage::returnPressedTPID1000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x37, params);
}
void DebugPage::returnPressedScanSpeed300(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x23, params);
}
void DebugPage::returnPressedScanDelay10000(){
    QByteArray params;
    if(parseFun(qobject_cast<QLineEdit*>(sender()),params))
        emit sigSendCmd(0x24, params);
}

// 版本号
void DebugPage::updateVer(QString topVer, QString bomVer)
{
    ui.label_topVer->setText(topVer);
    ui.label_bomVer->setText(bomVer);
}

// 扫描模式
void DebugPage::onClickFBAScan()
{
    QByteArray params;
    quint8 home;
    QRadioButton* pRadio = qobject_cast<QRadioButton*>(sender());
    if(pRadio == ui.radioBtnFScan)
        home = static_cast<quint8>(1);
    else if(pRadio == ui.radioBtnBScan)
        home = static_cast<quint8>(2);
    else
        home = static_cast<quint8>(3);
    params.append(home);
    emit sigSendCmd(0x44, params);

}

void DebugPage::returnXHOME1(QString text){
    ui.lineEdit_xhome1->setText(text);
}
void DebugPage::returnXHOME2(QString text){
    ui.lineEdit_xhome2->setText(text);
}
void DebugPage::returnXHOME3(QString text){
    ui.lineEdit_xhome3->setText(text);
}
void DebugPage::returnYHOME1(QString text){
    ui.lineEdit_yhome1->setText(text);
}
void DebugPage::returnYHOME2(QString text){
    ui.lineEdit_yhome2->setText(text);
}
void DebugPage::returnYHOME3(QString text){
    ui.lineEdit_yhome3->setText(text);
}
void DebugPage::returnXSpeed(QString text){
    ui.lineEdit_xspeed->setText(text);
}
void DebugPage::returnYSpeed(QString text){
    ui.lineEdit_yspeed->setText(text);
}
void DebugPage::returnMFreq(QString text){
    ui.lineEdit_mfreq->setText(text);
}
void DebugPage::returnBFreq(QString text){
    ui.lineEdit_bfreq->setText(text);
}
void DebugPage::returnPhase(QString text){
    ui.lineEdit_phase->setText(text);
}
void DebugPage::returnTFilter(QString text){
    ui.lineEdit_tfilter->setText(text);
}
void DebugPage::returnDFilter(QString text){
    ui.lineEdit_dfilter->setText(text);
}
void DebugPage::returnFilterA(QString text){
    ui.lineEdit_filtera->setText(text);
}
void DebugPage::returnFilterB(QString text){
    ui.lineEdit_filterb->setText(text);
}
void DebugPage::returnSFreq(QString text){
    ui.lineEdit_sfreq->setText(text);
}
void DebugPage::returnSFreq_Delay(QString text){
    ui.lineEdit_freq_delay->setText(text);
}
void DebugPage::returnSFreq_Start(QString text){
    ui.lineEdit_freq_start->setText(text);
}
void DebugPage::returnSFreq_Cutoff(QString text){
    ui.lineEdit_freq_cutoff->setText(text);
}
void DebugPage::returnPIDP(QString text){
    ui.lineEdit_pidp->setText(text);
}
void DebugPage::returnPIDI(QString text){
    ui.lineEdit_pidi->setText(text);
}
void DebugPage::returnPIDD(QString text){
    ui.lineEdit_pidd->setText(text);
}
void DebugPage::returnTPID(QString text){
    ui.lineEdit_tpid->setText(text);
}
void DebugPage::returnScanSpeed(QString text){
    ui.lineEdit_scanSpeed->setText(text);
}
void DebugPage::returnScanDelay(QString text){
    ui.lineEdit_scanDelay->setText(text);
}

void DebugPage::returnScanModel(quint8 model){
    if (model == 0x01)  ui.radioBtnFScan->setChecked(true);
    else if(model == 0x02)  ui.radioBtnBScan->setChecked(true);
    else if(model == 0x03)  ui.radioBtnAgainScan->setChecked(true);
}

// 设置配置文件
void DebugPage::setConfig(QString key,QString value)
{
  QSettings settings("Config", "TransSytem");
  settings.setValue(key, value);
}

// 获取配置文件
QString DebugPage::getConfigValue(QString key)
{
  QSettings settings("Config", "TransSytem");
  return settings.value(key).toString();
}

void DebugPage::updateStatus(QString tip)
{
    //ToastWidget::instance(this)->showToast(tip);
    ui.label_tip->setText(tip);
}

void DebugPage::closeEvent(QCloseEvent* event)
{
    emit sigCloseDebugPage(this);
}