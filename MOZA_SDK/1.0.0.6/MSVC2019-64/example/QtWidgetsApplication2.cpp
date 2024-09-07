#include "ui_widget.h"
#include "QtWidgetsApplication2.h"
#include <mozaAPI.h>

#include "WzSerialPort.h"
#include "CMakeProjectMoza.h"


#define MAX_ANGLE 180
#define MAX_BRAKE 32768

uint8_t gear[5] = { 2, 4, 6 , 8, 10 };

uint8_t SGP_Gear = 4;

uint8_t Last_SGP_F = 0;
uint8_t Last_SGP_B = 0;

uint8_t butNum[29] = { 19,20,8,7,6,5,22,23,21,34,33,32,24,37,25,38,36,3,1,2,4,35,13,14,113, 114, 115, 116, 117 };

WzSerialPort w;


typedef struct
{
    union
    {
        uint8_t imuData[20];
        struct
        {

            // float AccelX;
            // float AccelY;
            float AccelZ;

            // float GyroX;
            // float GyroY;
            float GyroZ;

            float roll;
            float pitch;
            //  float yaw;

            float speed;
        };

    };
}IMU_t;

IMU_t IMU = {
    .imuData = {0},
};

typedef struct
{
    double x;
    double y;
    double z;
}UWB_t;

UWB_t UWB[8] = {
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0}
};





Widget::Widget(QWidget* parent) :
    QWidget(parent), 
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setFixedSize(1080, 1960);
    //去掉边框
    setWindowFlags(Qt::FramelessWindowHint);
  //  this->setContentsMargins(0, 0, 0, 0);

    background.load("../../images/rc7.jpg");//"C:\Users\xh\Desktop\Snipaste_2024-08-28_09-57-11.jpg"
    movingImage[0].load("../../images/peng.jpg");
    movingImage[1].load("../../images/fan.jpg");
    movingImage[2].load("../../images/ding.jpg");
    movingImage[4].load("../../images/jia.jpg");

   // QString imagePath = QDir::current().filePath("images/background.png");
  //  loadImage();

   // setAttribute(Qt::WA_StyledBackground);      //启用样式背景绘制
   // setAttribute(Qt::WA_TranslucentBackground); //背景透明

    startAngle = 150;
    currentValue = 0;
    currentValue2 = 0;
    radius = 20;
    flag = true;

    imageX[0] = 100;
    imageY[0] = 400;

    imageX[1] = 200;
    imageY[1] = 400;


    imageX[2] = 300;
    imageY[2] = 400;


    imageX[4] = 400;
    imageY[4] = 400;


    timer = new QTimer(this);
   
    moza::installMozaSDK();
    ERRORCODE err = NORMAL;

#if 0

    // 创建串口选择框
    portSelector = new QComboBox();
    refreshPorts(); // 初始化时刷新端口列表

    // 创建波特率选择框
    baudRateSelector = new QComboBox();
    baudRateSelector->addItems({ "9600", "115200", "19200", "38400", "57600", "74880", "230400" });

    // 创建数据位选择框
    dataBitsSelector = new QComboBox();
    dataBitsSelector->addItems({ "5", "6", "7", "8" });

    // 创建停止位选择框
    stopBitsSelector = new QComboBox();
    stopBitsSelector->addItems({ "1", "1.5", "2" });

    // 创建校验位选择框
    paritySelector = new QComboBox();
    paritySelector->addItems({ "None", "Even", "Odd" });

    // 创建连接按钮
    connectButton = new QPushButton("Connect");
    connectButton->setFixedSize(200, 40);
    setConnectButtonStyle(false);

    // 创建发送框
    sendTextEdit = new QTextEdit();
    sendTextEdit->setFixedHeight(100);

    // 创建发送按钮
    sendButton = new QPushButton("Send");
    sendButton->setFixedSize(200, 40);
    sendButton->setEnabled(false); // 初始为禁用状态

    // 创建接收框
    receiveTextEdit = new QTextEdit();
    receiveTextEdit->setReadOnly(true);
    receiveTextEdit->setFixedHeight(100);

    // 创建布局
    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Select Serial Port:", portSelector);
    formLayout->addRow("Baud Rate:", baudRateSelector);
    formLayout->addRow("Data Bits:", dataBitsSelector);
    formLayout->addRow("Stop Bits:", stopBitsSelector);
    formLayout->addRow("Parity:", paritySelector);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(connectButton);
    layout->addWidget(new QLabel("Send:"));
    layout->addWidget(sendTextEdit);
    layout->addWidget(sendButton);
    layout->addWidget(new QLabel("Received:"));
    layout->addWidget(receiveTextEdit);

    setLayout(layout);

    // 连接按钮点击事件
    connect(connectButton, &QPushButton::clicked, this, &Widget::connectToPort);
    // 发送按钮点击事件
   // connect(sendButton, &QPushButton::clicked, this, &Widget::sendData);

#endif



    // 初始化串口
    serialPort = new QSerialPort(this);
    serialPort->setPortName("COM16"); // 替换为实际串口名称，例如 "/dev/ttyUSB0" 或 "COM3"
    serialPort->setBaudRate(QSerialPort::Baud9600); // 设置波特率
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port:" << serialPort->errorString();
    }


    Sleep(5000);
    err = moza::setMotorLimitAngle(MAX_ANGLE, MAX_ANGLE);

    //udpSocket = new QUdpSocket(this);
    //udpSocket->bind(QHostAddress("192.168.57.12"), 12345); // 绑定到任意地址和端口12345
    //connect(udpSocket, &QUdpSocket::readyRead, this, &Widget::processPendingDatagrams);
    //Sleep(100);
    udpSocketUWB = new QUdpSocket(this);
    udpSocketUWB->bind(QHostAddress("192.168.50.30"), 12345); // 绑定到任意地址和端口12345
    connect(udpSocketUWB, &QUdpSocket::readyRead, this, &Widget::processPendingDatagramsUWB);

   // CreateThread(NULL, 0, test_Func, NULL, 0, NULL);
    std::cout << err << std::endl;



    Button1 = new QPushButton("1", this);
    Button1->setGeometry(20, 1350, 200, 100);

    Button2 = new QPushButton("2", this);
    Button2->setGeometry(280, 1350, 200, 100);


    Button3 = new QPushButton("3", this);
    Button3->setGeometry(20, 1500, 200, 100);


    Button4 = new QPushButton("4", this);
    Button4->setGeometry(280, 1500, 200, 100);

    Button5 = new QPushButton("5", this);
    Button5->setGeometry(20, 1650, 200, 100);


    Button6 = new QPushButton("6", this);
    Button6->setGeometry(280, 1650, 200, 100);




    connect(Button1, &QPushButton::clicked, [=]() {

        mazaData.A = ~mazaData.A;
        if (mazaData.A) {
            Button1->setText("Switch is ON");
            Button1->setStyleSheet("background-color: green; color: white;");
        }
        else {
            Button1->setText("Switch is OFF");
            Button1->setStyleSheet("background-color: red; color: white;");
        }
        qDebug() << "Clicked1"<< mazaData.A;
        });


    connect(Button2, &QPushButton::clicked, [=]() {

        mazaData.B = 1;
        qDebug() << "Clicked2";
        });


    connect(Button3, &QPushButton::clicked, [=]() {

        mazaData.A = 1;
        qDebug() << "Clicked3";
        });


    connect(Button4, &QPushButton::clicked, [=]() {

        mazaData.A = 1;
        qDebug() << "Clicked4";
        });


    connect(Button5, &QPushButton::clicked, [=]() {

        mazaData.A = 1;
        qDebug() << "Clicked5";
        });


    connect(Button6, &QPushButton::clicked, [=]() {

        mazaData.A = 1;
        qDebug() << "Clicked6";
        });


    connect(timer, &QTimer::timeout, [=]() {

# if 1


        ERRORCODE err = NORMAL;
        const HIDData* d = moza::getHIDData(err);
        if (d) {

           // mazaData.buttons = 0;

            for (int i = 28; i >= 0; i--)
            {

               // mazaData.buttons |= (d->buttons[butNum[i]].isPressed() << (i));
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

            serialPort->write((char*)mazaData.Data, 16);

            if (mazaData.brake > 20)
            {
                currentValue2 = mazaData.brake;
            }
            else
            {
                currentValue2 = (d->throttle + MAX_BRAKE) * 500 / (2 * MAX_BRAKE);
            }
           
        }
        currentValue = abs(IMU.speed / 100);
       
#endif 
      //  currentValue = abs(IMU.speed / 100);

        //currentValue = 10;
        //刷新控件
        update();
        });
    timer->start(20);
}

Widget::~Widget()
{
    delete ui;
}

#if 0

void Widget::connectToPort() {
    QString portName = portSelector->currentText();
    serialPort.setPortName(portName);

    // 设置串口参数
    serialPort.setBaudRate(baudRateSelector->currentText().toInt());
    serialPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBitsSelector->currentText().toInt()));
    serialPort.setStopBits(static_cast<QSerialPort::StopBits>(stopBitsSelector->currentText().toInt()));
    serialPort.setParity(static_cast<QSerialPort::Parity>(paritySelector->currentIndex()));

    if (serialPort.open(QIODevice::ReadWrite)) {
        setConnectButtonStyle(true); // 连接成功后
        connectButton->setText("连接成功"); // 更改按钮文本
        sendButton->setEnabled(true); // 启用发送按钮
        QMessageBox::information(this, "Connection", "Connected to " + portName);
        //connect(&serialPort, &QSerialPort::readyRead, this, &Widget::readData); // 连接读数据信号
    }
    else {
        setConnectButtonStyle(false); // 连接失败
        connectButton->setText("Connect"); // 恢复按钮文本
        sendButton->setEnabled(false); // 禁用发送按钮
        QMessageBox::warning(this, "Connection Error", "Failed to connect to " + portName);
    }
}

void Widget::refreshPorts() {
    portSelector->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports) {
        portSelector->addItem(info.portName());
    }

}


void Widget::setConnectButtonStyle(bool connected) {
    if (connected) {
        connectButton->setStyleSheet("background-color: green; color: white;"); // 连接成功，绿色背景
    }
    else {
        connectButton->setStyleSheet("background-color: gray; color: white;"); // 未连接，灰色背景
    }
}

#endif

void Widget::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        if (datagram.size() == 20)
        {
            udpSocket->readDatagram((char*)IMU.imuData, datagram.size(), &sender, &senderPort);
        }
       
    }
}


void Widget::processPendingDatagramsUWB() {
    while (udpSocketUWB->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocketUWB->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        // 读取数据

      //  qDebug() << "Z:" << datagram.size();

        if (datagram.size() == 22)
        {
            udpSocketUWB->readDatagram((char*)IMU.imuData, 20, &sender, &senderPort);
        }
        else
        {
            udpSocketUWB->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            QString jsonString(datagram);
            parseJsonData(jsonString);
        }
       

        // 解析接收到的 JSON 数据

    }
}


void Widget::parseJsonData(const QString& jsonString) {
    // 将 JSON 字符串转换为 QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

    // 检查是否成功解析
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create JSON doc.";
        return;
    }
    // 确保是一个对象
    if (!jsonDoc.isObject()) {
        qDebug() << "JSON is not an object.";
        return;
    }
    // 获取 JSON 对象
    QJsonObject jsonObj = jsonDoc.object();

    // 解析各个字段
    QString command = jsonObj.value("Command").toString();
    int tagID = jsonObj.value("TagID").toInt();
    UWB[tagID].x = jsonObj.value("X").toDouble();
    UWB[tagID].y = jsonObj.value("Y").toDouble();
    UWB[tagID].z = jsonObj.value("Z").toDouble();
    //double x = jsonObj.value("X").toDouble();
    //double y = jsonObj.value("Y").toDouble();
    //double z = jsonObj.value("Z").toDouble();
    imageX[tagID] = abs(UWB[tagID].x)*60;
    imageY[tagID] = abs(UWB[tagID].y)*40;
    update();


    // 输出解析结果
    qDebug() << "Command:" << command;
    qDebug() << "TagID:" << tagID;
    qDebug() << "X:" << UWB[tagID].x;
    qDebug() << "Y:" << UWB[tagID].y;
    qDebug() << "Z:" << UWB[tagID].z;
}


void Widget::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Load Moving Image", "", "Images (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        movingImage[0].load(fileName);
        update();  // 更新窗口，触发重绘
    }
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
   // painter.setBrush(QColor(166, 183, 199));//rgb(166,183,199)
    painter.setBrush(QColor(0, 0, 0));//rgb(166,183,199)
    painter.drawRect(rect());
    //平移坐标系原点位置
    //QPoint center(rect().width() / 4, rect().height() * 0.6);
    //painter.translate(center);

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
    QPainter painter(this);//rgb(166,183,199)



#if 1
    int dashBoad_r = 160;//仪表盘半径
    //1.绘制画布
    initCanvas(painter);


    QPoint center(250,300);
    painter.translate(center);

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



    QPoint center2(0, 450);
    painter.translate(center2);
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
  
    //11.画LOGO
   // drawLogo(painter, dashBoad_r);


    QFont font_T1("Arial", 30);
    font_T1.setBold(true);
    painter.setFont(font_T1);
    painter.drawText( -160, 350, QString::number(IMU.AccelZ, 'f', 4));

    QFont font_T2("Arial", 30);
    font_T2.setBold(true);
    painter.setFont(font_T2);
    painter.drawText(40, 350, QString::number(IMU.GyroZ, 'f', 4));

    QFont font_T3("Arial", 30);
    font_T3.setBold(true);
    painter.setFont(font_T3);
    painter.drawText(-160, 500 , QString::number(IMU.pitch, 'f', 4));

    QFont font_T4("Arial", 30);
    font_T4.setBold(true);
    painter.setFont(font_T4);
    painter.drawText(40, 500, QString::number(IMU.roll, 'f', 4));


    drawEllipseOutSkirts(painter, dashBoad_r + 25);

#endif

    QPoint center3(230, -750);
    painter.translate(center3);

    int backgroundX = 0; // 底图在窗口中的 X 坐标
    int backgroundY = 0; // 底图在窗口中的 Y 坐标
    //// 绘制背景图
    if (!background.isNull()) {
        painter.drawPixmap(backgroundX, backgroundY, background.scaled(size(), Qt::KeepAspectRatio));
    }

    QLinearGradient gradient(50, 0, 0, 0);
    gradient.setColorAt(0.1, QColor(0, 0, 0, 0)); // 右边缘完全透明
    gradient.setColorAt(0.6, QColor(0, 0, 0, 255)); // 80% 位置完全不透明
    //gradient.setColorAt(0.8, QColor(0, 0, 0, 255)); // 左边缘完全不透明

    // 绘制渐变遮罩
    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
    painter.setRenderHint(QPainter::Antialiasing);



    // 设置圆形图片的中心和半径
    // 圆形半径
    QRectF circleRect(imageX[0] - radius, imageY[0] - radius, radius * 2, radius * 2);

    // 创建一个圆形裁剪区域
    QPainterPath path;
    path.addEllipse(circleRect);
    painter.setClipPath(path); // 设置裁剪区域为圆形
    // 确保移动图像是有效的 QPixmap
    if (!movingImage[0].isNull()) {
        // 绘制移动的图片，按比例缩放
        QPixmap scaledImage = movingImage[0].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect.topLeft(), scaledImage); // 绘制圆形图片
    }
    // 绘制圆形边框
    QColor borderColor(255, 255, 0); // 例如，红色边框
    painter.setPen(QPen(borderColor, 8)); // 设置边框颜色和宽度
    painter.setBrush(Qt::NoBrush); // 确保边框没有填充颜色
    painter.drawEllipse(circleRect); // 绘制圆形边框


    // 设置圆形图片的中心和半径
// 圆形半径
    QRectF circleRect1(imageX[1] - radius, imageY[1] - radius, radius * 2, radius * 2);

    // 创建一个圆形裁剪区域
    QPainterPath path1;
    path1.addEllipse(circleRect1);
    painter.setClipPath(path1); // 设置裁剪区域为圆形
    // 确保移动图像是有效的 QPixmap
    if (!movingImage[1].isNull()) {
        // 绘制移动的图片，按比例缩放
        QPixmap scaledImage1 = movingImage[1].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect1.topLeft(), scaledImage1); // 绘制圆形图片
    }
    // 绘制圆形边框
    QColor borderColor1(0, 255, 255); // 例如，红色边框
    painter.setPen(QPen(borderColor1, 8)); // 设置边框颜色和宽度
    painter.setBrush(Qt::NoBrush); // 确保边框没有填充颜色
    painter.drawEllipse(circleRect1); // 绘制圆形边框



    // 设置圆形图片的中心和半径
// 圆形半径
    QRectF circleRect2(imageX[2] - radius, imageY[2] - radius, radius * 2, radius * 2);

    // 创建一个圆形裁剪区域
    QPainterPath path2;
    path2.addEllipse(circleRect2);
    painter.setClipPath(path2); // 设置裁剪区域为圆形
    // 确保移动图像是有效的 QPixmap
    if (!movingImage[2].isNull()) {
        // 绘制移动的图片，按比例缩放
        QPixmap scaledImage2 = movingImage[2].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect2.topLeft(), scaledImage2); // 绘制圆形图片
    }
    // 绘制圆形边框
    QColor borderColor2(0, 0, 0); // 例如，红色边框
    painter.setPen(QPen(borderColor2, 8)); // 设置边框颜色和宽度
    painter.setBrush(Qt::NoBrush); // 确保边框没有填充颜色
    painter.drawEllipse(circleRect2); // 绘制圆形边框


    // 设置圆形图片的中心和半径
// 圆形半径
    QRectF circleRect3(imageX[4] - radius, imageY[4] - radius, radius * 2, radius * 2);

    // 创建一个圆形裁剪区域
    QPainterPath path3;
    path3.addEllipse(circleRect3);
    painter.setClipPath(path3); // 设置裁剪区域为圆形
    // 确保移动图像是有效的 QPixmap
    if (!movingImage[4].isNull()) {
        // 绘制移动的图片，按比例缩放
        QPixmap scaledImage3 = movingImage[4].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect3.topLeft(), scaledImage3); // 绘制圆形图片
    }

    // 绘制圆形边框
    QColor borderColor3(255, 255, 255); // 例如，红色边框
    painter.setPen(QPen(borderColor3, 8)); // 设置边框颜色和宽度
    painter.setBrush(Qt::NoBrush); // 确保边框没有填充颜色
    painter.drawEllipse(circleRect3); // 绘制圆形边框


    for (int i = 0; i < 4; i++)
    {

    }
    // 重置裁剪区域
    painter.setClipRect(QRectF(0, 0, 600,1960), Qt::ReplaceClip);



}
