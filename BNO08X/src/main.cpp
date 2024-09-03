/*
  Using the BNO080 IMU
  By: Nathan Seidle
  SparkFun Electronics
  Date: December 21st, 2017
  SparkFun code, firmware, and software is released under the MIT License.
	Please see LICENSE.md for further details.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14586

  This example shows how to output the parts of the calibrated gyro.

  It takes about 1ms at 400kHz I2C to read a record from the sensor, but we are polling the sensor continually
  between updates from the sensor. Use the interrupt pin on the BNO080 breakout to avoid polling.

  Hardware Connections:
  Attach the Qwiic Shield to your Arduino/Photon/ESP32 or other
  Plug the sensor onto the shield
  Serial.print it out at 115200 baud to serial monitor.
*/

#if 1
//#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "Servo16.h"
#include <WIFIUdp.h> //引用以使用异步UDP
#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
#include <AsyncUDP.h> //引用以使用异步UDP
#include "esp_rom_sys.h"
#include <soc/rtc_cntl_struct.h>
#include "esp_timer.h"

#include <stdio.h>
BNO080 myIMU;

//AsyncUDP  udp;//声明UDP对象
WiFiUDP Udp;
const char* wifi_SSID="Work";  //存储AP的名称信息
const char* wifi_Password="xhaxx518";  //存储AP的密码信息

uint16_t udp_port=1122;  //存储需要监听的端口号

char incomingPacket[255];  //存储Udp客户端发过来的数据
IPAddress remote_IP(192,168,50,75);// 自定义远程监听 IP 地址//192,168,57,252//192.158.56.19 //192,168,50,30 192,168,50,70//


typedef struct 
{
    union 
    {
        uint8_t imuData[16];
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
        };    

    };  
}IMU_t;

IMU_t IMU;

typedef struct
{
    union
    {
        uint8_t mozaData[16];
        struct
        {
            float fSteeringWheelAngle;
            int brake;
            int throttle;
            union
            {
                uint32_t buttons;
                struct
                {
                    uint32_t N : 1;
                    uint32_t WIP : 1;
                    uint32_t L : 1;
                    uint32_t D : 1;
                    uint32_t Ri : 1;
                    uint32_t U : 1;
                    uint32_t CAM : 1;
                    uint32_t RADIO : 1;
                    uint32_t FL : 1;
                    uint32_t PL : 1;
                    uint32_t BOX : 1;
                    uint32_t P : 1;
                    uint32_t S1 : 1;
                    uint32_t S2 : 1;
                    uint32_t HOME : 1;
                    uint32_t MENU : 1;
                    uint32_t START : 1;
                    uint32_t X : 1;
                    uint32_t A : 1;
                    uint32_t B : 1;
                    uint32_t Y : 1;
                    uint32_t R : 1;
                    uint32_t LG : 1;
                    uint32_t RG : 1;
                    uint32_t Other : 8;
                    /* data */
                };

            };
        };

    };
}MOZA_t;

MOZA_t mazaData;


void taskUDPread( void * parameter );
void taskUDPsend( void * parameter );
void onPacketCallBack(AsyncUDPPacket packet);

Servo M1;

void setup()
{
  Serial1.begin(115200);
  Serial1.setPins(5,6);

  Serial.begin(115200);
  Serial.println();
  Serial1.println("BNO080 Read Example");

 // pinMode(14,OUTPUT);

  Wire.begin(16, 15, 400000);//16,15

  myIMU.begin();

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  myIMU.enableGyro(50); //Send data update every 50ms
  myIMU.enableAccelerometer(50); //Send data update every 50ms
  myIMU.enableRotationVector(50); //Send data update every 50ms


  // myIMU.enableAccelerometer(50);    //We must enable the accel in order to get MEMS readings even if we don't read the reports.
  // myIMU.enableRawAccelerometer(50); //Send data update every 50ms
  // myIMU.enableGyro(50);
  // myIMU.enableRawGyro(50);
  // myIMU.enableMagnetometer(50);
  // myIMU.enableRawMagnetometer(50);

  Serial.println(F("Gyro enabled"));
  Serial.println(F("Output in form x, y, z, in radians per second"));

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_SSID, wifi_Password);
  while (!WiFi.isConnected())
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());

  Udp.begin(udp_port); //启用UDP监听以接收数据

  //  while (!udp.listen(udp_port)) //等待udp监听设置成功
  // {
  // }
  // udp.onPacket(onPacketCallBack); //注册收到数据包事件

  // M1.setPeriodHertz(50);
  // M1.attach(3, 0, 1000, 2000);
 // M1.writeMicroseconds(1000);

  //xTaskCreatePinnedToCore(taskUDPread, "taskUDPread", 4096, NULL, 1, NULL,1);
 xTaskCreatePinnedToCore(taskUDPsend, "taskUDPsend", 4096, NULL, 1, NULL,0);

}



void taskUDPsend( void * parameter )
{
  while (1)
  {
   // static uint8_t i = 0;
 #if 1
  //Look for reports from the IMU
  if (myIMU.dataAvailable() == true)
  {

    // IMU.AccelX = myIMU.getAccelX();
    // IMU.AccelY = myIMU.getAccelY();
    IMU.AccelZ = myIMU.getAccelZ()-9.8;

    // IMU.GyroX = myIMU.getGyroX();
    // IMU.GyroY = myIMU.getGyroY();
    IMU.GyroZ = myIMU.getGyroZ();

    IMU.pitch =  myIMU.getPitch();
    IMU.roll = myIMU.getRoll(); // Convert pitch to degrees
   // IMU.yaw = myIMU.getYaw()*180.0 / PI; // Convert yaw / heading to degrees

    // float roll = (myIMU.getRoll()) * 180.0 / PI; // Convert roll to degrees
    // float pitch = (myIMU.getPitch()) * 180.0 / PI; // Convert pitch to degrees
    // float yaw = (myIMU.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees

    // Udp.beginPacket(remote_IP, 4124); //准备发送数据
    // Udp.write(IMU.imuData, 16);
    // Udp.println();
    // Udp.endPacket();            //发送数据

    Serial.printf("roll:%.2f   pitch:%.2f  \r\n",IMU.roll, IMU.pitch);
    //Serial.println();
  }
#endif

   delay(10);

  }

  
}


/*
N :     19
WIP:    20
L:      8
D:      7
Ri:      6
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

void loop()
{
  static uint8_t i = 0;
 #if 0
  //Look for reports from the IMU
  if (myIMU.dataAvailable() == true)
  {
    // float x = myIMU.getGyroX();
    // float y = myIMU.getGyroY();

    // IMU.AccelX = myIMU.getAccelX();
    // IMU.AccelY = myIMU.getAccelY();
    IMU.AccelZ = myIMU.getAccelZ()-9.8;

    // IMU.GyroX = myIMU.getGyroX();
    // IMU.GyroY = myIMU.getGyroY();
    IMU.GyroZ = myIMU.getGyroZ();

    // float roll = myIMU.getRoll();
    // float pitch = myIMU.getPitch();
    // float yaw = myIMU.getYaw();

    // float roll = myIMU.getRoll(); // Convert roll to degrees
    // float pitch = myIMU.getPitch(); // Convert pitch to degrees
    // float yaw = myIMU.getYaw(); // Convert yaw / heading to degrees


    IMU.pitch =  myIMU.getPitch();
    IMU.roll = myIMU.getRoll(); // Convert pitch to degrees
   // IMU.yaw = myIMU.getYaw()*180.0 / PI; // Convert yaw / heading to degrees


    // float roll = (myIMU.getRoll()) * 180.0 / PI; // Convert roll to degrees
    // float pitch = (myIMU.getPitch()) * 180.0 / PI; // Convert pitch to degrees
    // float yaw = (myIMU.getYaw()) * 180.0 / PI; // Convert yaw / heading to degrees



    Udp.beginPacket(remote_IP, 4124); //准备发送数据
    Udp.write(IMU.imuData, 16);
    Udp.println();
    Udp.endPacket();            //发送数据

    //Serial.printf("roll:%.2f   pitch:%.2f  \r\n",IMU.roll, IMU.pitch);
    //Serial.println();
  }
#endif

   delay(5);
}


#endif

