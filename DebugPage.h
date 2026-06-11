#pragma once

#include <QWidget>
#include "ui_DebugPage.h"

#pragma execution_character_set("utf-8")
class DebugPage : public QWidget
{
	Q_OBJECT

public:
	DebugPage(QWidget *parent = nullptr);
	~DebugPage();

	void initFilter();
private:
	Ui::DebugPageClass ui;

protected:
	void closeEvent(QCloseEvent *event) override;

public:
	void updateVer(QString,QString);	//版本号
	void appendFloat(QByteArray& frame, float value);

signals:
	void sigSendCmd(quint8 cmd, const QByteArray &params);
	void sigCloseDebugPage(QWidget* pWidget);

public slots:
	
	void textChangeMaxCount(const QString &);
	void textChangeXHOME(const QString &);
	void textChangeYHOME(const QString &);
	void textChangeXSpeed300(const QString &);
	void textChangeYSpeed300(const QString &);
	void textChangeMFreq50000(const QString &);
	void textChangeBFreq50000(const QString &);
	void textChangePhase360(const QString &);
	
	void textChangeTFilter1000(const QString &);
	void textChangeDFilter1000(const QString &);
	void textChangeFilterA1000(const QString &);
	void textChangeFilterB1000(const QString &);
	 
	void textChangeSFreq1000(const QString &);
	void textChangeSFreq_Delay1000(const QString &);
	void textChangeSFreq_Start5000(const QString &);
	void textChangeSFreq_Cutoff5000(const QString &);
	
	void textChangePIDP100(const QString &);
	void textChangePIDI100(const QString &);
	void textChangePIDD100(const QString &);
	void textChangeTPID1000(const QString &);
	
	void textChangeScanSpeed300(const QString &);
	void textChangeScanDelay10000(const QString &);

	bool parseFun(QLineEdit* pEdit, QByteArray& params);
	bool parseSignedHome(QLineEdit* pEdit, QByteArray& params);
	bool parseFreqTenths(QLineEdit* pEdit, QByteArray& params);

	void returnPressedMaxCount();
	void returnPressedXHOME();
	void returnPressedYHOME();

	void returnPressedXSpeed300();
	void returnPressedYSpeed300();
	void returnPressedMFreq50000();
	void returnPressedBFreq50000();
	void returnPressedPhase360();
	
	void returnPressedTFilter1000();
	void returnPressedDFilter1000();
	void returnPressedFilterA1000();
	void returnPressedFilterB1000();
	 
	void returnPressedSFreq1000();
	void returnPressedSFreq_Delay1000();
	void returnPressedSFreq_Start5000();
	void returnPressedSFreq_Cutoff5000();
	
	void returnPressedPID100();
	void returnPressedTPID1000();
	
	void returnPressedScanSpeed300();
	void returnPressedScanDelay10000();
	
	void onClickFBAScan();

	//下位机同步返回
	void returnXHOME1(QString text);
	void returnXHOME2(QString text);
	void returnXHOME3(QString text);
	void returnYHOME1(QString text);
	void returnYHOME2(QString text);
	void returnYHOME3(QString text);
	void returnXSpeed(QString text);
	void returnYSpeed(QString text);
	void returnMFreq(QString text);
	void returnBFreq(QString text);
	void returnPhase(QString text);
	
	void returnTFilter(QString text);
	void returnDFilter(QString text);
	void returnFilterA(QString text);
	void returnFilterB(QString text);
	 
	void returnSFreq(QString text);
	void returnSFreq_Delay(QString text);
	void returnSFreq_Start(QString text);
	void returnSFreq_Cutoff(QString text);
	
	void returnPIDP(QString text);
	void returnPIDI(QString text);
	void returnPIDD(QString text);
	void returnTPID(QString text);
	
	void returnScanSpeed(QString text);
	void returnScanDelay(QString text);
	void returnScanModel(quint8 model);

	void setConfig(QString key, QString value);
	QString getConfigValue(QString key);
	void updateStatus(QString tip);
};

