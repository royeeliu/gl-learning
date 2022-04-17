#include "display_widget.h"

DisplayWidget::DisplayWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

DisplayWidget::~DisplayWidget() {}

void DisplayWidget::initializeGL() {
    initializeOpenGLFunctions();
}

void DisplayWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void DisplayWidget::paintGL() {
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
}
