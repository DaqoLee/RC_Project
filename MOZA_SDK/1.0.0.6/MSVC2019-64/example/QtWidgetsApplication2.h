#ifndef WIDGET_H
#define WIDGET_H

#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <QtMath>
#include <Qpushbutton.h>
#include <QUdpSocket>
#include <QWidget>
#include <QHostAddress>
#include <QString>
#include <QStringList>
#include<QSerialPort>
#include<QSerialPortInfo>

#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>


#include <QMessageBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = 0);
    ~Widget();

private slots:

    void processPendingDatagrams();
    void processPendingDatagramsUWB();
  //  void connectToPort();

private:
    Ui::Widget* ui;
    int startAngle;//起始角度
    double angle;
    int currentValue;//当前值
    int currentValue2;
    QTimer* timer;
    bool flag;//指针转动标志位


    //QComboBox* portSelector;
    //QComboBox* baudRateSelector;
    //QComboBox* dataBitsSelector;
    //QComboBox* stopBitsSelector;
    //QComboBox* paritySelector;
    //QPushButton* connectButton;
    //QPushButton* sendButton;
    //QTextEdit* sendTextEdit;
    //QTextEdit* receiveTextEdit;
    //QSerialPort serialPort;

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

    void parseJsonData(const QString& jsonString);
    void loadImage();

    //void refreshPorts();
    //void setConnectButtonStyle(bool connected);
 protected:

    QPixmap background;
    QPixmap movingImage[6];

    QPushButton* Button1;
    QPushButton* Button2;
    QPushButton* Button3;
    QPushButton* Button4;
    QPushButton* Button5;
    QPushButton* Button6;

    QUdpSocket* udpSocket;

    QUdpSocket* udpSocketUWB;

    QSerialPort* serialPort;

    int imageX[6];           // 图片的 X 坐标
    int imageY[6];           // 图片的 Y 坐标
    int radius;

    void paintEvent(QPaintEvent* event) override;
};

#endif // WIDGET_H
