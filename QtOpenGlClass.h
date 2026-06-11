#pragma once

#include <QWidget>
#include "ui_QtOpenGlClass.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTimer>
#include <QColor>
#include <QPainter>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QMessageBox>
#include <cmath>
#include <random>
#include <algorithm>
#include <climits>

class QtOpenGlClass : public QWidget
{
	Q_OBJECT

public:
	QtOpenGlClass(QWidget *parent = nullptr);
	~QtOpenGlClass();

private:
	Ui::QtOpenGlClassClass ui;
};

