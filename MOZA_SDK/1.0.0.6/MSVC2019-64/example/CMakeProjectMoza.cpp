
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "mozaAPI.h"
#include <windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <string>
//#include <QWidget>
#include "effects\EffectException.h"
#include "effects\WinDirectInputApiException.h"
#include <time.h>

#include <ctime>
#include <chrono>
#include <sstream>

#include <iostream>



#include "WzSerialPort.h"

#include "CMakeProjectMoza.h"

#include "QtWidgetsApplication2.h"
#include <QApplication>


#pragma comment(lib, "ws2_32.lib")
#if 0
void sendUDPPacket(const char* serverIP, int serverPort, const char* message) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    int sendResult = sendto(sock, message, strlen(message), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Sendto failed: " << WSAGetLastError() << "\n";
    }
    else {
        std::cout << "Message sent: " << message << "\n";
    }

    closesocket(sock);
    WSACleanup();
}

void receiveUDPPacket(int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return;
    }

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int bytesReceived = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&clientAddr, &clientAddrLen);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Recvfrom failed: " << WSAGetLastError() << "\n";
    }
    else {
        buffer[bytesReceived] = '\0';
        std::cout << "Received message: " << buffer << "\n";
    }

    closesocket(sock);
    WSACleanup();
}

void sendUDPArray(const char* serverIP, int serverPort, uint8_t* array, int arraySize) {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr;

    // 初始化Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    // 创建UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    // 设置目标地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);



    // 发送数组
    if (sendto(sock, (char*)array, arraySize * sizeof(uint8_t), 0,
        (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Sendto failed: " << WSAGetLastError() << "\n";
    }
    else {
        std::cout << "Array sent";

        //std::cout << ss.str();
    }

    // 清理
    closesocket(sock);
    WSACleanup();
}

#endif


//uint8_t butNum[29] = { 19,20,8,7,6,5,22,23,21,34,33,32,24,37,25,38,36,3,1,2,4,35,13,14,113, 114, 115, 116, 117 };

MOZA_t mazaData;

MOZA_t returnData;
#if 0
// 打开串口
bool openSerialPort(HANDLE& hSerial, const char* portName) {
    hSerial = CreateFile(
        (const WCHAR*)portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port.\n";
        return false;
    }
    return true;
}

// 配置串口
bool configureSerialPort(HANDLE hSerial) {
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting serial port state.\n";
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting serial port state.\n";
        return false;
    }

    return true;
}

// 发送数据
bool sendSerialData(HANDLE hSerial, const char* data, size_t size) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, size, &bytesWritten, nullptr) || bytesWritten != size) {
        std::cerr << "Error writing to serial port.\n";
        return false;
    }
    return true;
}

// 关闭串口
void closeSerialPort(HANDLE hSerial) {
    CloseHandle(hSerial);
}

#endif



#if 1
WzSerialPort::WzSerialPort()
{

}

WzSerialPort::~WzSerialPort()
{

}

bool WzSerialPort::open(const char* portname,
    int baudrate,
    char parity,
    char databit,
    char stopbit,
    char synchronizeflag)
{
    this->synchronizeflag = synchronizeflag;
    HANDLE hCom = NULL;
    if (this->synchronizeflag)
    {
        //同步方式
        hCom = CreateFileA(portname, //串口名
            GENERIC_READ | GENERIC_WRITE, //支持读写
            0, //独占方式，串口不支持共享
            NULL,//安全属性指针，默认值为NULL
            OPEN_EXISTING, //打开现有的串口文件
            0, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
            NULL);//用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
    }
    else
    {
        //异步方式
        hCom = CreateFileA(portname, //串口名
            GENERIC_READ | GENERIC_WRITE, //支持读写
            0, //独占方式，串口不支持共享
            NULL,//安全属性指针，默认值为NULL
            OPEN_EXISTING, //打开现有的串口文件
            FILE_FLAG_OVERLAPPED, //0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
            NULL);//用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
    }

    if (hCom == (HANDLE)-1)
    {
        return false;
    }

    //配置缓冲区大小 
    if (!SetupComm(hCom, 1024, 1024))
    {
        return false;
    }

    // 配置参数 
    DCB p;
    memset(&p, 0, sizeof(p));
    p.DCBlength = sizeof(p);
    p.BaudRate = baudrate; // 波特率
    p.ByteSize = databit; // 数据位

    switch (parity) //校验位
    {
    case 0:
        p.Parity = NOPARITY; //无校验
        break;
    case 1:
        p.Parity = ODDPARITY; //奇校验
        break;
    case 2:
        p.Parity = EVENPARITY; //偶校验
        break;
    case 3:
        p.Parity = MARKPARITY; //标记校验
        break;
    }

    switch (stopbit) //停止位
    {
    case 1:
        p.StopBits = ONESTOPBIT; //1位停止位
        break;
    case 2:
        p.StopBits = TWOSTOPBITS; //2位停止位
        break;
    case 3:
        p.StopBits = ONE5STOPBITS; //1.5位停止位
        break;
    }

    if (!SetCommState(hCom, &p))
    {
        // 设置参数失败
        return false;
    }

    //超时处理,单位：毫秒
    //总超时＝时间系数×读或写的字符数＋时间常量
    COMMTIMEOUTS TimeOuts;
    TimeOuts.ReadIntervalTimeout = 1000; //读间隔超时
    TimeOuts.ReadTotalTimeoutMultiplier = 500; //读时间系数
    TimeOuts.ReadTotalTimeoutConstant = 5000; //读时间常量
    TimeOuts.WriteTotalTimeoutMultiplier = 500; // 写时间系数
    TimeOuts.WriteTotalTimeoutConstant = 2000; //写时间常量
    SetCommTimeouts(hCom, &TimeOuts);

    PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);//清空串口缓冲区

    memcpy(pHandle, &hCom, sizeof(hCom));// 保存句柄

    return true;
}

void WzSerialPort::close()
{
    HANDLE hCom = *(HANDLE*)pHandle;
    CloseHandle(hCom);
}

int WzSerialPort::send(const void* buf, int len)
{
    HANDLE hCom = *(HANDLE*)pHandle;

    if (this->synchronizeflag)
    {
        // 同步方式
        DWORD dwBytesWrite = len; //成功写入的数据字节数
        BOOL bWriteStat = WriteFile(hCom, //串口句柄
            buf, //数据首地址
            dwBytesWrite, //要发送的数据字节数
            &dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
            NULL); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteStat)
        {
            return 0;
        }
        return dwBytesWrite;
    }
    else
    {
        //异步方式
        DWORD dwBytesWrite = len; //成功写入的数据字节数
        DWORD dwErrorFlags; //错误标志
        COMSTAT comStat; //通讯状态
        OVERLAPPED m_osWrite; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osWrite, 0, sizeof(m_osWrite));
        m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

        ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        BOOL bWriteStat = WriteFile(hCom, //串口句柄
            buf, //数据首地址
            dwBytesWrite, //要发送的数据字节数
            &dwBytesWrite, //DWORD*，用来接收返回成功发送的数据字节数
            &m_osWrite); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bWriteStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在写入
            {
                WaitForSingleObject(m_osWrite.hEvent, 1000); //等待写入事件1秒钟
            }
            else
            {
                ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osWrite.hEvent); //关闭并释放hEvent内存
                return 0;
            }
        }
        return dwBytesWrite;
    }
}

int WzSerialPort::receive(void* buf, int maxlen)
{
    HANDLE hCom = *(HANDLE*)pHandle;

    if (this->synchronizeflag)
    {
        //同步方式
        DWORD wCount = maxlen; //成功读取的数据字节数
        BOOL bReadStat = ReadFile(hCom, //串口句柄
            buf, //数据首地址
            wCount, //要读取的数据最大字节数
            &wCount, //DWORD*,用来接收返回成功读取的数据字节数
            NULL); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            return 0;
        }
        return wCount;
    }
    else
    {
        //异步方式
        DWORD wCount = maxlen; //成功读取的数据字节数
        DWORD dwErrorFlags; //错误标志
        COMSTAT comStat; //通讯状态
        OVERLAPPED m_osRead; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osRead, 0, sizeof(m_osRead));
        m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, L"ReadEvent");

        ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        if (!comStat.cbInQue)return 0; //如果输入缓冲区字节数为0，则返回false

        BOOL bReadStat = ReadFile(hCom, //串口句柄
            buf, //数据首地址
            wCount, //要读取的数据最大字节数
            &wCount, //DWORD*,用来接收返回成功读取的数据字节数
            &m_osRead); //NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在读取中
            {
                //GetOverlappedResult函数的最后一个参数设为TRUE
                //函数会一直等待，直到读操作完成或由于错误而返回
                GetOverlappedResult(hCom, &m_osRead, &wCount, TRUE);
            }
            else
            {
                ClearCommError(hCom, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osRead.hEvent); //关闭并释放hEvent的内存
                return 0;
            }
        }
        return wCount;
    }
}

#endif
using namespace std;



#if 0

#define MAX_ANGLE 720
#define MAX_BRAKE 32768


uint8_t gear[5] = { 2, 4, 6 , 8, 10 };

uint8_t SGP_Gear = 0;

uint8_t Last_SGP_F = 0;
uint8_t Last_SGP_B = 0;



uint8_t butNum[29] = { 19,20,8,7,6,5,22,23,21,34,33,32,24,37,25,38,36,3,1,2,4,35,13,14,113, 114, 115, 116, 117 };

int main(int argc, char* argv[]) {
    // printf("hi\r\n");
    moza::installMozaSDK();
    ERRORCODE err = NORMAL;
    uint8_t arrayToSend[5] = { 1, 2, 3, 4, 5 };
    int port = 1347;
    WzSerialPort w;

    Sleep(5000);
    err = moza::setMotorLimitAngle(MAX_ANGLE, MAX_ANGLE);

    std::cout << err << std::endl;

    //QApplication a(argc, argv);
    //Widget wi;
    //wi.show();

    if (w.open("\\\\.\\COM16", 115200, 0, 8, 1))
  //  if (w.open("COM9", 115200, 0, 8, 1))
    {

    }

    while (true) {
        Sleep(10);


        const HIDData* d = moza::getHIDData(err);
        if (d) {

# if 1
    
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

           // std::cout << SGP_Gear << std::endl;
            printf("SGP_Gear:%d \r\n", SGP_Gear+1);
            
            mazaData.fSteeringWheelAngle = d->fSteeringWheelAngle * 500 / (MAX_ANGLE / 2);
            mazaData.brake = (d->brake + MAX_BRAKE) * 500 / (2 * MAX_BRAKE);
            mazaData.throttle = (d->throttle + MAX_BRAKE) * 50 *  gear[SGP_Gear] / (2 * MAX_BRAKE);

            Last_SGP_F = mazaData.SGP_F;
            Last_SGP_B = mazaData.SGP_B;

           // printf("SGP_F:%d SGP_B:%d SGP_S1:%d SGP_S2:%d SGP_S3:%d \r\n", mazaData.SGP_F, mazaData.SGP_B, mazaData.SGP_S1, mazaData.SGP_S2, mazaData.SGP_S3);
#endif         
#if 1
           // if (w.open("\\\\.\\COM12", 115200, 0, 8, 1))       
            {
                //  for (int i = 0; i < 10; i++)
                {
                    if (w.send(mazaData.Data, 16))
                    {
                       // std::cout << "Angle:" << mazaData.fSteeringWheelAngle << "\tBrake:" << mazaData.brake << std::endl;
                    }
                    else
                    {
                        w.open("\\\\.\\COM16", 115200, 0, 8, 1);
                       // w.open("COM9", 115200, 0, 8, 1);
                        cout << "send demo warning...";
                    }
                    // memset(returnData.imuData, 0, 16);
                    if (w.receive(returnData.Data, 16) == 16)
                    {
                        // std::cout<<"---" << returnData.fSteeringWheelAngle << std::endl;
                    }

                }
                //  cout << "send demo finished...";
            }

            //           sendUDPArray("192.168.53.70", 1122, mazaData.imuData,16);//192.168.58.204
                       //std::thread sender(sendUDPArray, "127.0.0.1", port, arrayToSend, 5);

#endif  
#if 0
            for (int i = 0; i < 56; i++)
            {

                std::cout << i << ":" << d->buttons[i].isPressed() << " ";
            }
#endif          
            //           std::cout << std::endl;
                   //    std::cout << curr_pos << "," << knob << "," << youmen << std::endl;
        }


    }

#if 1



    // closeSerialPort(hSerial);

#endif

    return 0;
}

#endif
/*
N :     19
WIP:    20
L:      8
D:      7
R:      6
U:      5
CAM:    22
RADIO   23

FL      21
PL      34

BOX     33
P       32

S1      24
S2      37
HOME    25
MENU    38

START   36



X       3
A       1
B       2
Y       4

R       35

LG      13
RG      14

*/
//QTEST_MAIN(APITest)

//#include "CMakeProject1.moc"
