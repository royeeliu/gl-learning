#pragma once

#include <QOpenGLFunctions>
#include <QtOpenGLWidgets/QOpenGLWidget>

class DisplayWidget final : public QOpenGLWidget, private QOpenGLFunctions
{
    Q_OBJECT

public:
    DisplayWidget(QWidget* parent = nullptr);
    ~DisplayWidget();

private:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLFunctions* gl_ = nullptr;
};