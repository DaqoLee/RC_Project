#ifndef WIDGET_H
#define WIDGET_H

#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <QtMath>

namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = 0);
    ~Widget();

private:
    Ui::Widget* ui;
    int startAngle;//起始角度
    double angle;
    int currentValue;//当前值
    int currentValue2;
    QTimer* timer;
    bool flag;//指针转动标志位


    QPoint windowPos;
    QPoint mousePos;
    QPoint dPos;

    void initCanvas(QPainter& painter);
    void drawMiddleCircle(QPainter& painter, int radius);
    // void drawScaleLine(QPainter& painter, int radius);
    void drawScaleLine(QPainter& painter, int radius, int maxValue, int scale);
    // void drawScaleValue(QPainter& painter, int radius);
    void drawScaleValue(QPainter& painter, int radius, int maxValue, int scale);
    //void drawPoint(QPainter& painter, int radius);
    void drawPoint(QPainter& painter, int radius, int maxValue, float value);
    //void drawSpeedSector(QPainter& painter, int radius);
    void drawSpeedSector(QPainter& painter, int radius, int maxValue, int value);
    void drawInnerEllipse(QPainter& painter, int radius);
    void drawInnerEllipseBlack(QPainter& painter, int radius);
    void drawCurrentSpeed(QPainter& painter);
    void drawEllipseOutSkirts(QPainter& painter, int radius);
    void drawLogo(QPainter& painter, int radius);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // WIDGET_H
