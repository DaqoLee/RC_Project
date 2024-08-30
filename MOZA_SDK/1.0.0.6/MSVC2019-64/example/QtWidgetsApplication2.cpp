#include "ui_widget.h"
#include "QtWidgetsApplication2.h"
#include <mozaAPI.h>

#include "WzSerialPort.h"
#include "CMakeProjectMoza.h"


#define MAX_ANGLE 180
#define MAX_BRAKE 32768

uint8_t gear[5] = { 2, 4, 6 , 8, 10 };

uint8_t SGP_Gear = 0;

uint8_t Last_SGP_F = 0;
uint8_t Last_SGP_B = 0;

uint8_t butNum[29] = { 19,20,8,7,6,5,22,23,21,34,33,32,24,37,25,38,36,3,1,2,4,35,13,14,113, 114, 115, 116, 117 };

WzSerialPort w;


Widget::Widget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setFixedSize(1080, 600);
    //去掉边框
    setWindowFlags(Qt::FramelessWindowHint);
  //  this->setContentsMargins(0, 0, 0, 0);

   // setAttribute(Qt::WA_StyledBackground);      //启用样式背景绘制
   // setAttribute(Qt::WA_TranslucentBackground); //背景透明

    startAngle = 150;
    currentValue = 0;
    flag = true;

    timer = new QTimer(this);
   
    moza::installMozaSDK();
    ERRORCODE err = NORMAL;

    Sleep(5000);
    err = moza::setMotorLimitAngle(MAX_ANGLE, MAX_ANGLE);

    std::cout << err << std::endl;


    //if (w.open("COM9", 115200, 0, 8, 1))
    if (w.open("\\\\.\\COM16", 115200, 0, 8, 1))// if (w.open("\\\\.\\COM16", 115200, 0, 8, 1))
    {

    }

    connect(timer, &QTimer::timeout, [=]() {
# if 0
        if (flag)
        {
            currentValue++;
            if (currentValue >= 61)
            {
                flag = false;
            }
        }
        else
        {
            currentValue--;
            if (currentValue == 0)
            {
                flag = true;
            }
        }
#endif
# if 1


        ERRORCODE err = NORMAL;
        const HIDData* d = moza::getHIDData(err);
        if (d) {

            mazaData.buttons = 0;

            for (int i = 28; i >= 0; i--)
            {

                mazaData.buttons |= (d->buttons[butNum[i]].isPressed() << (i));
                //std::cout << i << ":" << d->buttons[i].isPressed() << " ";
            }

            if (mazaData.SGP_B != Last_SGP_B)
            {
                if (mazaData.SGP_B == 1)
                {
                    SGP_Gear = SGP_Gear >= 4 ? 4 : SGP_Gear + 1;
                }

            }

            if (mazaData.SGP_F != Last_SGP_F)
            {
                if (mazaData.SGP_F == 1)
                {
                    SGP_Gear = SGP_Gear <= 0 ? 0 : SGP_Gear - 1;
                }

            }

            //std::cout << SGP_Gear << std::endl;
            //printf("SGP_Gear:%d \r\n", SGP_Gear + 1);
            if (d->fSteeringWheelAngle >= -(MAX_ANGLE / 2) && d->fSteeringWheelAngle <= (MAX_ANGLE / 2))
            {
               mazaData.fSteeringWheelAngle = d->fSteeringWheelAngle * 500 / (MAX_ANGLE / 2);
            }
           
            mazaData.brake = (d->brake + MAX_BRAKE) * 500 / (2 * MAX_BRAKE);
            mazaData.throttle = (d->throttle + MAX_BRAKE) * 50 * gear[SGP_Gear] / (2 * MAX_BRAKE);

            Last_SGP_F = mazaData.SGP_F;
            Last_SGP_B = mazaData.SGP_B;

            if (w.send(mazaData.Data, 16))
            {
                 std::cout << "Angle:" << mazaData.fSteeringWheelAngle << "\tBrake:" << mazaData.brake << std::endl;
            }
            else
            {
                w.open("\\\\.\\COM16", 115200, 0, 8, 1);
                //w.open("COM9", 115200, 0, 8, 1);
                std::cout << "send demo warning...";
                std::cout << std::endl;
            }                 //    std::cout << curr_pos << "," << knob << "," << youmen << std::endl;

            currentValue2 = (d->throttle + MAX_BRAKE) * 500 / (2 * MAX_BRAKE);
        }
#endif 

        //currentValue = 10;
        //刷新控件
        update();
        });
    timer->start(25);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::mousePressEvent(QMouseEvent* event)
{
    this->windowPos = this->pos();       // 获得部件当前位置
    this->mousePos = event->globalPos(); // 获得鼠标位置
    this->dPos = mousePos - windowPos;   // 移动后部件所在的位置
}

void Widget::mouseMoveEvent(QMouseEvent* event)
{
    this->move(event->globalPos() - this->dPos);
}
/*
 * 绘制画布
*/
void Widget::initCanvas(QPainter& painter)
{
    //消除锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);
    //设置底色
    painter.setBrush(QColor(0, 0, 0));

    painter.drawRect(rect());
    //平移坐标系原点位置
    QPoint center(rect().width() / 4, rect().height() * 0.6);
    painter.translate(center);

}

/*
 * 画小圆
*/
void Widget::drawMiddleCircle(QPainter& painter, int radius)
{
    //设置画笔颜色和宽度
    painter.setPen(QPen(QColor(255, 255, 255), 3));
    //原点坐标（0，0）绘制半径为radius的圆
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * 画刻度
*/
void Widget::drawScaleLine(QPainter& painter, int radius, int maxValue, int scale)
{
    //总计60个小刻度，每一个小刻度的角度值
    angle = 240 * 1.0 / scale;

    //float ang = 240 * 1.0 / scale;
    //保存当前坐标
    painter.save();
    painter.setPen(QPen(Qt::white, 3));
    //设置起始刻度位置
    painter.rotate(startAngle);
    //painter.
    for (int i = 0; i <= scale; i++)
    {
        if (i >= (3 * scale / 4))
        {
            //第40个刻度后，绘制画笔修改成红色
            painter.setPen(QPen(Qt::blue, 3));
        }
        if (i % 5 == 0)
        {
            //绘制长刻度
            painter.drawLine(radius + 10, 0, radius - 5, 0);

        }
        else
        {
            //绘制短刻度
            painter.drawLine(radius + 10, 0, radius , 0);
        }
        //绘制完一个刻度旋转一次坐标
        painter.rotate(angle);
    }
    //恢复坐标
    painter.restore();
    painter.setPen(QPen(Qt::white, 5));


}

/*
 * 画刻度值
*/
void Widget::drawScaleValue(QPainter& painter, int radius, int maxValue, int scale)
{
    //设置字体类型和大小


    QFont textFont("Arial", 12);
    //设置粗体

    angle = 240 * 1.0 / scale;

    // float ang = 240 * 1.0 / scale;
    textFont.setBold(true);
    painter.setFont(textFont);
    int text_r = radius - 49;
    for (int i = 0; i <= scale; i++)
    {
        if (i % 5 == 0)
        {
            if (i >= 3 * scale / 4)
            {
                painter.setPen(QPen(Qt::blue, 5));
            }
            //保存当前坐标系
            painter.save();
            int delX = qCos((210 - angle * i) * M_PI / 180) * text_r;
            int delY = qSin(qDegreesToRadians(210 - angle * i)) * text_r;
            //平移坐标系
            painter.translate(QPoint(delX, -delY));
            //旋转坐标系
            painter.rotate(-120 + angle * i);
            //写上刻度值，文字居中
            painter.drawText(-25, -50, 50, 30, Qt::AlignCenter, QString::number(i * (maxValue / scale)));
            //恢复坐标系
            painter.restore();
        }
    }
    painter.setPen(QPen(Qt::white, 5));
}

/*
 * 画指针
*/
void Widget::drawPoint(QPainter& painter, int radius, int maxValue, float value)
{
    //保存当前坐标
    painter.save();
    //设置画刷颜色
    painter.setBrush(Qt::white);
    //设置画笔为无线条
    painter.setPen(Qt::NoPen);
    static const QPointF points[4] = {
        QPointF(0, 0.0),
        QPointF(radius * 2.0 / 3, -1.1),
        QPointF(radius * 2.0 / 3, 1.1),
        QPointF(0, 15.0)
    };
    //坐标选旋转
    painter.rotate(startAngle + value * (240.0 / maxValue));////value * (500.0/240))
    //绘制多边形
    painter.drawPolygon(points, 4);
    //恢复坐标
    painter.restore();
}

/*
 * 画扇形
*/
void Widget::drawSpeedSector(QPainter& painter, int radius, int maxValue, int value)
{
    //定义矩形区域
    QRect rentangle(-radius, -radius, radius * 2, radius * 2);
    //设置画笔无线条
    painter.setPen(Qt::NoPen);
    //设置画刷颜色
    painter.setBrush(QColor(0, 0, 255, 80));
    //绘制扇形
    painter.drawPie(rentangle, (360 - startAngle) * 16, -value * (240.0 / maxValue) * 16);
}

/*
 * 画渐变内圆
*/
void Widget::drawInnerEllipse(QPainter& painter, int radius)
{
    QRadialGradient radial(0, 0, radius);
    //中心颜色
    radial.setColorAt(0.0, QColor(0, 0, 255, 200));
    //外围颜色
    radial.setColorAt(1.0, QColor(0, 0, 0, 100));
    //设置画刷渐变色
    painter.setBrush(radial);
    //画圆形
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * 画黑色内圈
*/
void Widget::drawInnerEllipseBlack(QPainter& painter, int radius)
{
    //设置画刷
    painter.setBrush(Qt::black);
    //绘制圆形
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * 绘制当前数值
*/
void Widget::drawCurrentSpeed(QPainter& painter)
{
    painter.setPen(Qt::white);
    //绘制数值
    QFont font("Arial", 24);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-60, -60, 120, 100), Qt::AlignCenter, QString::number(currentValue * 4));
    //绘制单位
    QFont font_u("Arial", 13);
    painter.setFont(font_u);
    painter.drawText(QRect(-60, -60, 120, 160), Qt::AlignCenter, "km/h");

    //QFont font_T("Arial", 40);
    //font_T.setBold(true);
    //painter.setFont(font_T);
    //painter.drawText(QRect(-60, -60, 1000, 100), Qt::AlignCenter, QString::number(5));
}

/*
 * 画外壳，发光外圈
*/
void Widget::drawEllipseOutSkirts(QPainter& painter, int radius)
{
    //设置扇形绘制区域
    QRect outAngle(-radius, -radius, 2 * radius, 2 * radius);
    painter.setPen(Qt::NoPen);
    //设置渐变色
    QRadialGradient radia(0, 0, radius);
    radia.setColorAt(1, QColor(0, 0, 255, 200));
    radia.setColorAt(0.97, QColor(0, 0, 255, 120));
    radia.setColorAt(0.9, QColor(0, 0, 0, 0));
    radia.setColorAt(0, QColor(0, 0, 0, 0));
    painter.setBrush(radia);
    //绘制圆形
    painter.drawPie(outAngle, (360 - 150) * 16, -240 * 16);
}

/*
 * 画LOGO
*/
void Widget::drawLogo(QPainter& painter, int radius)
{
    //定义Logo绘制区域
    QRect rectLogo(-65, radius * 0.38, 130, 50);
    painter.drawPixmap(rectLogo, QPixmap("./logo2.png"));
}



/*
 * 重写绘制事件
*/
void Widget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    int dashBoad_r = height() / 4;//仪表盘半径
    //1.绘制画布
    initCanvas(painter);
    //2.画小圆
    drawMiddleCircle(painter, 30);
    //3.画刻度
    drawScaleLine(painter, dashBoad_r, 240, 60);
    //4.画刻度值
    drawScaleValue(painter, dashBoad_r, 240, 60);
    //5.画指针
    drawPoint(painter, dashBoad_r, 240, currentValue);
    //6.画扇形
    drawSpeedSector(painter, dashBoad_r + 25, 240, currentValue);
    //7.画渐变内圆
    drawInnerEllipse(painter, 110 / 2);
    //8.画黑色内圈
    drawInnerEllipseBlack(painter, 80 / 2);
    //9.画当前数值
   // drawCurrentSpeed(painter);
    painter.setPen(Qt::white);
    //绘制数值
    QFont font("Arial", 20);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-60, -60, 120, 120), Qt::AlignCenter, QString::number(currentValue));
    //绘制单位
    QFont font_u("Arial", 16);
    font_u.setBold(true);
    painter.setFont(font_u);
    painter.drawText(QRect(-60, -60, 120, 320), Qt::AlignCenter, "km/h");
    //10.画外壳，发光外圈
    drawEllipseOutSkirts(painter, dashBoad_r + 25);



    QPoint center(2 * rect().width() / 4, 0);
    painter.translate(center);
    drawMiddleCircle(painter, 30);
    //3.画刻度
    drawScaleLine(painter, dashBoad_r, 240, 50);
    //4.画刻度值
    drawScaleValue(painter, dashBoad_r, 500, 50);
    //5.画指针
    drawPoint(painter, dashBoad_r, 500, currentValue2);
    //6.画扇形
    drawSpeedSector(painter, dashBoad_r + 25, 500, currentValue2);
    //7.画渐变内圆
    drawInnerEllipse(painter, 110 / 2);
    //8.画黑色内圈
    drawInnerEllipseBlack(painter, 80 / 2);

    painter.setPen(Qt::white);
    // 绘制数值
    QFont font_C("Arial", 20);
    font_C.setBold(true);
    painter.setFont(font_C);
    painter.drawText(QRect(-60, -60, 120, 120), Qt::AlignCenter, QString::number(currentValue2));//* angle * (500.0/240))

    QFont font_T("Arial", 20);
    font_T.setBold(true);
    painter.setFont(font_T);
    painter.drawText(QRect(-60, -60, 120, 320), Qt::AlignCenter, QString::number(SGP_Gear+1));
    //9.画当前数值
   // drawCurrentSpeed(painter);
    //10.画外壳，发光外圈
    drawEllipseOutSkirts(painter, dashBoad_r + 25);
    //11.画LOGO
   // drawLogo(painter, dashBoad_r);
}
