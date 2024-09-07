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
    //ȥ���߿�
    setWindowFlags(Qt::FramelessWindowHint);
  //  this->setContentsMargins(0, 0, 0, 0);

    background.load("../../images/rc7.jpg");//"C:\Users\xh\Desktop\Snipaste_2024-08-28_09-57-11.jpg"
    movingImage[0].load("../../images/peng.jpg");
    movingImage[1].load("../../images/fan.jpg");
    movingImage[2].load("../../images/ding.jpg");
    movingImage[4].load("../../images/jia.jpg");

   // QString imagePath = QDir::current().filePath("images/background.png");
  //  loadImage();

   // setAttribute(Qt::WA_StyledBackground);      //������ʽ��������
   // setAttribute(Qt::WA_TranslucentBackground); //����͸��

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

    // ��������ѡ���
    portSelector = new QComboBox();
    refreshPorts(); // ��ʼ��ʱˢ�¶˿��б�

    // ����������ѡ���
    baudRateSelector = new QComboBox();
    baudRateSelector->addItems({ "9600", "115200", "19200", "38400", "57600", "74880", "230400" });

    // ��������λѡ���
    dataBitsSelector = new QComboBox();
    dataBitsSelector->addItems({ "5", "6", "7", "8" });

    // ����ֹͣλѡ���
    stopBitsSelector = new QComboBox();
    stopBitsSelector->addItems({ "1", "1.5", "2" });

    // ����У��λѡ���
    paritySelector = new QComboBox();
    paritySelector->addItems({ "None", "Even", "Odd" });

    // �������Ӱ�ť
    connectButton = new QPushButton("Connect");
    connectButton->setFixedSize(200, 40);
    setConnectButtonStyle(false);

    // �������Ϳ�
    sendTextEdit = new QTextEdit();
    sendTextEdit->setFixedHeight(100);

    // �������Ͱ�ť
    sendButton = new QPushButton("Send");
    sendButton->setFixedSize(200, 40);
    sendButton->setEnabled(false); // ��ʼΪ����״̬

    // �������տ�
    receiveTextEdit = new QTextEdit();
    receiveTextEdit->setReadOnly(true);
    receiveTextEdit->setFixedHeight(100);

    // ��������
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

    // ���Ӱ�ť����¼�
    connect(connectButton, &QPushButton::clicked, this, &Widget::connectToPort);
    // ���Ͱ�ť����¼�
   // connect(sendButton, &QPushButton::clicked, this, &Widget::sendData);

#endif



    // ��ʼ������
    serialPort = new QSerialPort(this);
    serialPort->setPortName("COM16"); // �滻Ϊʵ�ʴ������ƣ����� "/dev/ttyUSB0" �� "COM3"
    serialPort->setBaudRate(QSerialPort::Baud9600); // ���ò�����
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
    //udpSocket->bind(QHostAddress("192.168.57.12"), 12345); // �󶨵������ַ�Ͷ˿�12345
    //connect(udpSocket, &QUdpSocket::readyRead, this, &Widget::processPendingDatagrams);
    //Sleep(100);
    udpSocketUWB = new QUdpSocket(this);
    udpSocketUWB->bind(QHostAddress("192.168.50.30"), 12345); // �󶨵������ַ�Ͷ˿�12345
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
        //ˢ�¿ؼ�
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

    // ���ô��ڲ���
    serialPort.setBaudRate(baudRateSelector->currentText().toInt());
    serialPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBitsSelector->currentText().toInt()));
    serialPort.setStopBits(static_cast<QSerialPort::StopBits>(stopBitsSelector->currentText().toInt()));
    serialPort.setParity(static_cast<QSerialPort::Parity>(paritySelector->currentIndex()));

    if (serialPort.open(QIODevice::ReadWrite)) {
        setConnectButtonStyle(true); // ���ӳɹ���
        connectButton->setText("���ӳɹ�"); // ���İ�ť�ı�
        sendButton->setEnabled(true); // ���÷��Ͱ�ť
        QMessageBox::information(this, "Connection", "Connected to " + portName);
        //connect(&serialPort, &QSerialPort::readyRead, this, &Widget::readData); // ���Ӷ������ź�
    }
    else {
        setConnectButtonStyle(false); // ����ʧ��
        connectButton->setText("Connect"); // �ָ���ť�ı�
        sendButton->setEnabled(false); // ���÷��Ͱ�ť
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
        connectButton->setStyleSheet("background-color: green; color: white;"); // ���ӳɹ�����ɫ����
    }
    else {
        connectButton->setStyleSheet("background-color: gray; color: white;"); // δ���ӣ���ɫ����
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

        // ��ȡ����

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
       

        // �������յ��� JSON ����

    }
}


void Widget::parseJsonData(const QString& jsonString) {
    // �� JSON �ַ���ת��Ϊ QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());

    // ����Ƿ�ɹ�����
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create JSON doc.";
        return;
    }
    // ȷ����һ������
    if (!jsonDoc.isObject()) {
        qDebug() << "JSON is not an object.";
        return;
    }
    // ��ȡ JSON ����
    QJsonObject jsonObj = jsonDoc.object();

    // ���������ֶ�
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


    // ����������
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
        update();  // ���´��ڣ������ػ�
    }
}

void Widget::mousePressEvent(QMouseEvent* event)
{
    this->windowPos = this->pos();       // ��ò�����ǰλ��
    this->mousePos = event->globalPos(); // ������λ��
    this->dPos = mousePos - windowPos;   // �ƶ��󲿼����ڵ�λ��
}

void Widget::mouseMoveEvent(QMouseEvent* event)
{
    this->move(event->globalPos() - this->dPos);
}
/*
 * ���ƻ���
*/
void Widget::initCanvas(QPainter& painter)
{
    //�������
    painter.setRenderHint(QPainter::Antialiasing, true);
    //���õ�ɫ
   // painter.setBrush(QColor(166, 183, 199));//rgb(166,183,199)
    painter.setBrush(QColor(0, 0, 0));//rgb(166,183,199)
    painter.drawRect(rect());
    //ƽ������ϵԭ��λ��
    //QPoint center(rect().width() / 4, rect().height() * 0.6);
    //painter.translate(center);

}

/*
 * ��СԲ
*/
void Widget::drawMiddleCircle(QPainter& painter, int radius)
{
    //���û�����ɫ�Ϳ��
    painter.setPen(QPen(QColor(255, 255, 255), 3));
    //ԭ�����꣨0��0�����ư뾶Ϊradius��Բ
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * ���̶�
*/
void Widget::drawScaleLine(QPainter& painter, int radius, int maxValue, int scale)
{
    //�ܼ�60��С�̶ȣ�ÿһ��С�̶ȵĽǶ�ֵ
    angle = 240 * 1.0 / scale;

    //float ang = 240 * 1.0 / scale;
    //���浱ǰ����
    painter.save();
    painter.setPen(QPen(Qt::white, 3));
    //������ʼ�̶�λ��
    painter.rotate(startAngle);
    //painter.
    for (int i = 0; i <= scale; i++)
    {
        if (i >= (3 * scale / 4))
        {
            //��40���̶Ⱥ󣬻��ƻ����޸ĳɺ�ɫ
            painter.setPen(QPen(Qt::blue, 3));
        }
        if (i % 5 == 0)
        {
            //���Ƴ��̶�
            painter.drawLine(radius + 10, 0, radius - 5, 0);

        }
        else
        {
            //���ƶ̶̿�
            painter.drawLine(radius + 10, 0, radius , 0);
        }
        //������һ���̶���תһ������
        painter.rotate(angle);
    }
    //�ָ�����
    painter.restore();
    painter.setPen(QPen(Qt::white, 5));


}

/*
 * ���̶�ֵ
*/
void Widget::drawScaleValue(QPainter& painter, int radius, int maxValue, int scale)
{
    //�����������ͺʹ�С


    QFont textFont("Arial", 12);
    //���ô���

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
            //���浱ǰ����ϵ
            painter.save();
            int delX = qCos((210 - angle * i) * M_PI / 180) * text_r;
            int delY = qSin(qDegreesToRadians(210 - angle * i)) * text_r;
            //ƽ������ϵ
            painter.translate(QPoint(delX, -delY));
            //��ת����ϵ
            painter.rotate(-120 + angle * i);
            //д�Ͽ̶�ֵ�����־���
            painter.drawText(-25, -50, 50, 30, Qt::AlignCenter, QString::number(i * (maxValue / scale)));
            //�ָ�����ϵ
            painter.restore();
        }
    }
    painter.setPen(QPen(Qt::white, 5));
}

/*
 * ��ָ��
*/
void Widget::drawPoint(QPainter& painter, int radius, int maxValue, float value)
{
    //���浱ǰ����
    painter.save();
    //���û�ˢ��ɫ
    painter.setBrush(Qt::white);
    //���û���Ϊ������
    painter.setPen(Qt::NoPen);
    static const QPointF points[4] = {
        QPointF(0, 0.0),
        QPointF(radius * 2.0 / 3, -1.1),
        QPointF(radius * 2.0 / 3, 1.1),
        QPointF(0, 15.0)
    };
    //����ѡ��ת
    painter.rotate(startAngle + value * (240.0 / maxValue));////value * (500.0/240))
    //���ƶ����
    painter.drawPolygon(points, 4);
    //�ָ�����
    painter.restore();
}

/*
 * ������
*/
void Widget::drawSpeedSector(QPainter& painter, int radius, int maxValue, int value)
{
    //�����������
    QRect rentangle(-radius, -radius, radius * 2, radius * 2);
    //���û���������
    painter.setPen(Qt::NoPen);
    //���û�ˢ��ɫ
    painter.setBrush(QColor(0, 0, 255, 80));
    //��������
    painter.drawPie(rentangle, (360 - startAngle) * 16, -value * (240.0 / maxValue) * 16);
}

/*
 * ��������Բ
*/
void Widget::drawInnerEllipse(QPainter& painter, int radius)
{
    QRadialGradient radial(0, 0, radius);
    //������ɫ
    radial.setColorAt(0.0, QColor(0, 0, 255, 200));
    //��Χ��ɫ
    radial.setColorAt(1.0, QColor(0, 0, 0, 100));
    //���û�ˢ����ɫ
    painter.setBrush(radial);
    //��Բ��
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * ����ɫ��Ȧ
*/
void Widget::drawInnerEllipseBlack(QPainter& painter, int radius)
{
    //���û�ˢ
    painter.setBrush(Qt::black);
    //����Բ��
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

/*
 * ���Ƶ�ǰ��ֵ
*/
void Widget::drawCurrentSpeed(QPainter& painter)
{
    painter.setPen(Qt::white);
    //������ֵ
    QFont font("Arial", 24);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-60, -60, 120, 100), Qt::AlignCenter, QString::number(currentValue * 4));
    //���Ƶ�λ
    QFont font_u("Arial", 13);
    painter.setFont(font_u);
    painter.drawText(QRect(-60, -60, 120, 160), Qt::AlignCenter, "km/h");

    //QFont font_T("Arial", 40);
    //font_T.setBold(true);
    //painter.setFont(font_T);
    //painter.drawText(QRect(-60, -60, 1000, 100), Qt::AlignCenter, QString::number(5));
}

/*
 * ����ǣ�������Ȧ
*/
void Widget::drawEllipseOutSkirts(QPainter& painter, int radius)
{
    //�������λ�������
    QRect outAngle(-radius, -radius, 2 * radius, 2 * radius);
    painter.setPen(Qt::NoPen);
    //���ý���ɫ
    QRadialGradient radia(0, 0, radius);
    radia.setColorAt(1, QColor(0, 0, 255, 200));
    radia.setColorAt(0.97, QColor(0, 0, 255, 120));
    radia.setColorAt(0.9, QColor(0, 0, 0, 0));
    radia.setColorAt(0, QColor(0, 0, 0, 0));
    painter.setBrush(radia);
    //����Բ��
    painter.drawPie(outAngle, (360 - 150) * 16, -240 * 16);
}

/*
 * ��LOGO
*/
void Widget::drawLogo(QPainter& painter, int radius)
{
    //����Logo��������
    QRect rectLogo(-65, radius * 0.38, 130, 50);
    painter.drawPixmap(rectLogo, QPixmap("./logo2.png"));
}



/*
 * ��д�����¼�
*/
void Widget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);//rgb(166,183,199)



#if 1
    int dashBoad_r = 160;//�Ǳ��̰뾶
    //1.���ƻ���
    initCanvas(painter);


    QPoint center(250,300);
    painter.translate(center);

    //2.��СԲ
    drawMiddleCircle(painter, 30);
    //3.���̶�
    drawScaleLine(painter, dashBoad_r, 240, 60);
    //4.���̶�ֵ
    drawScaleValue(painter, dashBoad_r, 240, 60);
    //5.��ָ��
    drawPoint(painter, dashBoad_r, 240, currentValue);
    //6.������
    drawSpeedSector(painter, dashBoad_r + 25, 240, currentValue);
    //7.��������Բ
    drawInnerEllipse(painter, 110 / 2);
    //8.����ɫ��Ȧ
    drawInnerEllipseBlack(painter, 80 / 2);
    //9.����ǰ��ֵ
   // drawCurrentSpeed(painter);
    painter.setPen(Qt::white);
    //������ֵ
    QFont font("Arial", 20);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(-60, -60, 120, 120), Qt::AlignCenter, QString::number(currentValue));
    //���Ƶ�λ
    QFont font_u("Arial", 16);
    font_u.setBold(true);
    painter.setFont(font_u);
    painter.drawText(QRect(-60, -60, 120, 320), Qt::AlignCenter, "km/h");
    //10.����ǣ�������Ȧ
    drawEllipseOutSkirts(painter, dashBoad_r + 25);



    QPoint center2(0, 450);
    painter.translate(center2);
    drawMiddleCircle(painter, 30);
    //3.���̶�
    drawScaleLine(painter, dashBoad_r, 240, 50);
    //4.���̶�ֵ
    drawScaleValue(painter, dashBoad_r, 500, 50);
    //5.��ָ��
    drawPoint(painter, dashBoad_r, 500, currentValue2);
    //6.������
    drawSpeedSector(painter, dashBoad_r + 25, 500, currentValue2);
    //7.��������Բ
    drawInnerEllipse(painter, 110 / 2);
    //8.����ɫ��Ȧ
    drawInnerEllipseBlack(painter, 80 / 2);

    painter.setPen(Qt::white);
    // ������ֵ
    QFont font_C("Arial", 20);
    font_C.setBold(true);
    painter.setFont(font_C);
    painter.drawText(QRect(-60, -60, 120, 120), Qt::AlignCenter, QString::number(currentValue2));//* angle * (500.0/240))

    QFont font_T("Arial", 20);
    font_T.setBold(true);
    painter.setFont(font_T);
    painter.drawText(QRect(-60, -60, 120, 320), Qt::AlignCenter, QString::number(SGP_Gear+1));
    //9.����ǰ��ֵ
   // drawCurrentSpeed(painter);
    //10.����ǣ�������Ȧ
  
    //11.��LOGO
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

    int backgroundX = 0; // ��ͼ�ڴ����е� X ����
    int backgroundY = 0; // ��ͼ�ڴ����е� Y ����
    //// ���Ʊ���ͼ
    if (!background.isNull()) {
        painter.drawPixmap(backgroundX, backgroundY, background.scaled(size(), Qt::KeepAspectRatio));
    }

    QLinearGradient gradient(50, 0, 0, 0);
    gradient.setColorAt(0.1, QColor(0, 0, 0, 0)); // �ұ�Ե��ȫ͸��
    gradient.setColorAt(0.6, QColor(0, 0, 0, 255)); // 80% λ����ȫ��͸��
    //gradient.setColorAt(0.8, QColor(0, 0, 0, 255)); // ���Ե��ȫ��͸��

    // ���ƽ�������
    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
    painter.setRenderHint(QPainter::Antialiasing);



    // ����Բ��ͼƬ�����ĺͰ뾶
    // Բ�ΰ뾶
    QRectF circleRect(imageX[0] - radius, imageY[0] - radius, radius * 2, radius * 2);

    // ����һ��Բ�βü�����
    QPainterPath path;
    path.addEllipse(circleRect);
    painter.setClipPath(path); // ���òü�����ΪԲ��
    // ȷ���ƶ�ͼ������Ч�� QPixmap
    if (!movingImage[0].isNull()) {
        // �����ƶ���ͼƬ������������
        QPixmap scaledImage = movingImage[0].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect.topLeft(), scaledImage); // ����Բ��ͼƬ
    }
    // ����Բ�α߿�
    QColor borderColor(255, 255, 0); // ���磬��ɫ�߿�
    painter.setPen(QPen(borderColor, 8)); // ���ñ߿���ɫ�Ϳ��
    painter.setBrush(Qt::NoBrush); // ȷ���߿�û�������ɫ
    painter.drawEllipse(circleRect); // ����Բ�α߿�


    // ����Բ��ͼƬ�����ĺͰ뾶
// Բ�ΰ뾶
    QRectF circleRect1(imageX[1] - radius, imageY[1] - radius, radius * 2, radius * 2);

    // ����һ��Բ�βü�����
    QPainterPath path1;
    path1.addEllipse(circleRect1);
    painter.setClipPath(path1); // ���òü�����ΪԲ��
    // ȷ���ƶ�ͼ������Ч�� QPixmap
    if (!movingImage[1].isNull()) {
        // �����ƶ���ͼƬ������������
        QPixmap scaledImage1 = movingImage[1].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect1.topLeft(), scaledImage1); // ����Բ��ͼƬ
    }
    // ����Բ�α߿�
    QColor borderColor1(0, 255, 255); // ���磬��ɫ�߿�
    painter.setPen(QPen(borderColor1, 8)); // ���ñ߿���ɫ�Ϳ��
    painter.setBrush(Qt::NoBrush); // ȷ���߿�û�������ɫ
    painter.drawEllipse(circleRect1); // ����Բ�α߿�



    // ����Բ��ͼƬ�����ĺͰ뾶
// Բ�ΰ뾶
    QRectF circleRect2(imageX[2] - radius, imageY[2] - radius, radius * 2, radius * 2);

    // ����һ��Բ�βü�����
    QPainterPath path2;
    path2.addEllipse(circleRect2);
    painter.setClipPath(path2); // ���òü�����ΪԲ��
    // ȷ���ƶ�ͼ������Ч�� QPixmap
    if (!movingImage[2].isNull()) {
        // �����ƶ���ͼƬ������������
        QPixmap scaledImage2 = movingImage[2].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect2.topLeft(), scaledImage2); // ����Բ��ͼƬ
    }
    // ����Բ�α߿�
    QColor borderColor2(0, 0, 0); // ���磬��ɫ�߿�
    painter.setPen(QPen(borderColor2, 8)); // ���ñ߿���ɫ�Ϳ��
    painter.setBrush(Qt::NoBrush); // ȷ���߿�û�������ɫ
    painter.drawEllipse(circleRect2); // ����Բ�α߿�


    // ����Բ��ͼƬ�����ĺͰ뾶
// Բ�ΰ뾶
    QRectF circleRect3(imageX[4] - radius, imageY[4] - radius, radius * 2, radius * 2);

    // ����һ��Բ�βü�����
    QPainterPath path3;
    path3.addEllipse(circleRect3);
    painter.setClipPath(path3); // ���òü�����ΪԲ��
    // ȷ���ƶ�ͼ������Ч�� QPixmap
    if (!movingImage[4].isNull()) {
        // �����ƶ���ͼƬ������������
        QPixmap scaledImage3 = movingImage[4].scaled(radius * 2, radius * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(circleRect3.topLeft(), scaledImage3); // ����Բ��ͼƬ
    }

    // ����Բ�α߿�
    QColor borderColor3(255, 255, 255); // ���磬��ɫ�߿�
    painter.setPen(QPen(borderColor3, 8)); // ���ñ߿���ɫ�Ϳ��
    painter.setBrush(Qt::NoBrush); // ȷ���߿�û�������ɫ
    painter.drawEllipse(circleRect3); // ����Բ�α߿�


    for (int i = 0; i < 4; i++)
    {

    }
    // ���òü�����
    painter.setClipRect(QRectF(0, 0, 600,1960), Qt::ReplaceClip);



}
