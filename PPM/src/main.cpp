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
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "Servo16.h"
//#include <WIFIUdp.h> //引用以使用异步UDP
#include "SparkFun_BNO080_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_BNO080
#include <AsyncUDP.h> //引用以使用异步UDP
#include "esp_rom_sys.h"
#include "PPM.h"
#include <soc/rtc_cntl_struct.h>
#include "esp_timer.h"

#include <stdio.h>
#include <SimpleSerialShell.h>
#include "esp_log.h"
#include "driver/rmt.h"
#include "musical_buzzer.h"

BNO080 myIMU;

AsyncUDP  udp;//声明UDP对象

const char* wifi_SSID="Work";  //存储AP的名称信息
const char* wifi_Password="xhaxx518";  //存储AP的密码信息

uint16_t udp_port=1122;  //存储需要监听的端口号

char incomingPacket[255];  //存储Udp客户端发过来的数据
IPAddress remote_IP(192,168,56,19);// 自定义远程监听 IP 地址



void taskUDPread( void * parameter );
void taskSerialRead( void * parameter );
void onPacketCallBack(AsyncUDPPacket packet);
static bool timer_callback(void *args);

Servo M1;
esp_timer_handle_t timer1;

uint8_t ppmnum[8]={10, 20, 30, 40, 50, 60, 70, 5 };
int cnt = 0;

void timer1Interrupt(void *args){

  static int i = 0;
  static int j = 0;

  cnt++;

  if (cnt<=200)
  {
      digitalWrite(14,HIGH);
  }
  else if (cnt>=400)
  {
    cnt = 0;
    i = 0;
  }
  else
  {
      digitalWrite(14,LOW);
  }


   //Serial.println("BNO080 Read Example");



}

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

typedef struct {
    example_timer_info_t info;
    uint64_t timer_counter_value;
} example_timer_event_t;


static const char *TAG = "example";

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO_NUM (4)

/**
 * @brief Musical Notation: Beethoven's Ode to joy
 *
 */
static const musical_buzzer_notation_t notation[] = {
    {740, 400}, {740, 600}, {784, 400}, {880, 400},
    {880, 400}, {784, 400}, {740, 400}, {659, 400},
    {587, 400}, {587, 400}, {659, 400}, {740, 400},
    {740, 400}, {740, 200}, {659, 200}, {659, 800},

    {740, 400}, {740, 600}, {784, 400}, {880, 400},
    {880, 400}, {784, 400}, {740, 400}, {659, 400},
    {587, 400}, {587, 400}, {659, 400}, {740, 400},
    {659, 400}, {659, 200}, {587, 200}, {587, 800},

    {659, 400}, {659, 400}, {740, 400}, {587, 400},
    {659, 400}, {740, 200}, {784, 200}, {740, 400}, {587, 400},
    {659, 400}, {740, 200}, {784, 200}, {740, 400}, {659, 400},
    {587, 400}, {659, 400}, {440, 400}, {440, 400},

    {740, 400}, {740, 600}, {784, 400}, {880, 400},
    {880, 400}, {784, 400}, {740, 400}, {659, 400},
    {587, 400}, {587, 400}, {659, 400}, {740, 400},
    {659, 400}, {659, 200}, {587, 200}, {587, 800},
};
static rmt_item32_t morse_esp[10] = {
	// E : dot

	{{{ 200, 0, 200, 0 }}}, // dot

	{{{ 1000, 1, 500, 0 }}}, // dot
	{{{ 1000, 1, 100, 0 }}}, // SPACE
  {{{ 1000, 1, 500, 0 }}}, // dot
	{{{ 1000, 1, 100, 0 }}}, // SPACE
  {{{ 1000, 1, 800, 0 }}}, // dot
	{{{ 1000, 1, 100, 0 }}}, // SPACE
  {{{ 1000, 1, 500, 0 }}}, // dot
	{{{ 1000, 1, 100, 0 }}}, // SPACE

  {{{ 500, 1, 500, 1 }}} // dot
 // {{{ 0, 1, 0, 0 }}}

};

uint32_t RCchannel[8]={1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};


void RMTsend(uint32_t *channel, uint8_t num)
{
  uint32_t sum = 0;
  morse_esp[0] = {{{ 200, 0, 200, 0 }}};

  for (int i = 0; i < num; i++)
  {
     morse_esp[i+1] = {{{(RCchannel[i]-500), 1, 500, 0 }}};
     sum+=RCchannel[i];
  }



  morse_esp[num+1]={{{(19000-sum), 1, 600, 1 }}};

  rmt_write_items(RMT_CHANNEL_0, morse_esp, sizeof(morse_esp) / sizeof(morse_esp[0]), true);

}


int chset(int argc, char **argv) {
  if (argc == 3) {
    auto ch = atoi(argv[1]);
    auto num = atoi(argv[2]);
    RCchannel[ch -1] = num;
  
  }
  return 0;
}

void setup()
{
  Serial1.begin(115200);
  Serial1.setPins(14,13);

  Serial.begin(115200);
  Serial.println();
 // Serial1.println("BNO080 Read Example");

  pinMode(9,OUTPUT);

  shell.attach(Serial);
  shell.addCommand(F("RC <ch> <num>"), chset);  // switch 2 on // switch 2 off

    // Apply default RMT configuration
    rmt_config_t dev_config = {                                                \
        .rmt_mode = RMT_MODE_TX,                     \
        .channel = RMT_TX_CHANNEL,                       \
        .gpio_num = GPIO_NUM_38,                            \
        .clk_div = 80,                               \
        .mem_block_num = 1,                          \
        .flags = 0,                                  \
        .tx_config = {                               \
            .carrier_freq_hz = 50,                \
            .carrier_level = RMT_CARRIER_LEVEL_HIGH, \
            .idle_level = RMT_IDLE_LEVEL_LOW,        \
            .carrier_duty_percent = 33,              \
            .carrier_en = false,                     \
            .loop_en = true,                        \
            .idle_output_en = false,                  \
        }                                            \
    };
    //dev_config.tx_config.loop_en = true; // Enable loop mode

    // Install RMT driver
    ESP_ERROR_CHECK(rmt_config(&dev_config));
    ESP_ERROR_CHECK(rmt_driver_install(RMT_TX_CHANNEL, 0, 0));

  //xTaskCreatePinnedToCore(taskUDPread, "taskUDPread", 4096, NULL, 1, NULL,1);
  xTaskCreate(taskSerialRead, "taskSerialRead", 4096, NULL, 1, NULL);

}

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
uint8_t Mozabuff[12];




typedef struct
{
    union
    {
        uint8_t data[16];
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

                    uint32_t SGP_F : 1;
                    uint32_t SGP_B : 1;
                    uint32_t SGP_S1 : 1;
                    uint32_t SGP_S2 : 1;
                    uint32_t SGP_S3 : 1;

                    uint32_t Other : 3;

                  
                    /* data */
                };

            };
        };

    };
}MOZA_t;

MOZA_t MozaData;




typedef struct
{
    union
    {
      uint8_t data[15];
      struct
      {
          uint8_t head;
          uint8_t code;
          uint16_t ch1;
          uint16_t ch2;
          uint16_t ch3;
          uint16_t ch4;
          uint16_t ch5;
          uint16_t ch6;
          uint8_t crc;
          
      };

    };
}UserData_t;

UserData_t UserData;

static bool IRAM_ATTR timer_callback(void *args){

}

void onPacketCallBack(AsyncUDPPacket packet)
{
  // Serial.print("UDP数据包来源类型: ");
  // Serial.println(packet.isBroadcast() ? "广播数据" : (packet.isMulticast() ? "组播" : "单播"));
  // Serial.print("远端地址及端口号: ");
  // Serial.print(packet.remoteIP());
  // Serial.print(":");
  // Serial.println(packet.remotePort());
  // Serial.print("目标地址及端口号: ");
  // Serial.print(packet.localIP());
  // Serial.print(":");
  // Serial.println(packet.localPort());
  // Serial.print("数据长度: ");
  // Serial.println(packet.length());
  // Serial.print("数据内容: ");
  // Serial.write(packet.data(), packet.length());


  memcpy(MozaData.data,packet.data(),packet.length());
  int ang = MozaData.fSteeringWheelAngle*100;
  M1.writeMicroseconds(1500 + ang/10);
  //Serial.printf("Angle:%d   brake:%d   throttle:%d buttons:%d \r\n",ang/10, MozaData.brake,MozaData.throttle,MozaData.buttons);
 // Serial.println();

  // packet.print("reply data");
  // broadcastPort = packet.remotePort();


 
}

void taskUDPread( void * parameter )
{
  uint8_t cr = 0;
  //int ang = 0;
  while (1)
  {
   // if (Udp.parsePacket()) 
    {
      //char val = Udp.read();
     // int len = Udp.read(MozaData.mozaData,16);

    //  int ang = MozaData.fSteeringWheelAngle*100;

      // if(ang > 500)
      // {
      //   cr = 1;
      // }

      // if(ang < -500)
      // {
      //   cr = 0;
      // }

      // if(cr == 1)
      // {
      //   ang-=50;
      // }
      // else
      // {
      //   ang+=50;
      // }

      // M1.writeMicroseconds(1500 + ang/10);

      // Serial.printf("Angle:%d   brake:%d   throttle:%d buttons:%d \r\n",ang/10, MozaData.brake,MozaData.throttle,MozaData.buttons);
    
    // Udp.endPacket();
    }
   // Serial.println("123");
    delay(5);

  }

  
}


void taskSerialRead( void * parameter )
{
  while (1)
  {
    int  numBytes = Serial.available();
     uint8_t RxData[16];


    if (numBytes == 16) {

      Serial.read(MozaData.data,numBytes);
      //Serial1.printf("Angle:%.2f   brake:%d   throttle:%d  \r\n",MozaData.fSteeringWheelAngle, MozaData.brake, MozaData.throttle);
      digitalWrite(9,HIGH);
    }
    else if (numBytes == -1)
    {
      //Serial1.println("err");
    }
    else{

    }

   
    // numBytes = Serial1.available();


   
    // if (numBytes == 14) 
    // {
    //     // 逐个读取字节并存储到字节数组中
    //   Serial1.read(RxData,numBytes);
    //  // Serial1.write(UserData.data,numBytes);
    //  // Serial1.println("er1");
    //   if(RxData[0] == 0x5A)
    //   {
    //      memcpy(UserData.data,RxData,numBytes);
    //     // Serial.write(UserData.data,numBytes);
    //     // Serial1.println("err2");
    //     Serial.println(UserData.ch1);
    //   }
     
    //   //Serial1.printf("Angle:%.2f   brake:%d   throttle:%d  \r\n",MozaData.fSteeringWheelAngle, MozaData.brake, MozaData.throttle);
    //   digitalWrite(9,HIGH);
    // }

    //Serial1.printf("Angle:%.2f   brake:%d   throttle:%d  \r\n",MozaData.fSteeringWheelAngle, MozaData.brake, MozaData.throttle);
      delay(10);
      digitalWrite(9,LOW);

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


uint32_t ch_check(uint16_t ch)
{
  static uint16_t last_ch = 1500;
  if (ch >= 1000 && ch <= 2000)
  {
    last_ch = ch;
    return ch;
  }
  else
  {
    return last_ch;
  }
  

}


void loop()
{

  shell.executeIfInput();

  int ang = MozaData.fSteeringWheelAngle;
  int thro = MozaData.throttle ;
  int brake = MozaData.brake;
  RCchannel[0] = 1600 +  MozaData.fSteeringWheelAngle;//+ ang;
  RCchannel[1] = 1500 + thro - brake;

  // RCchannel[0] = ch_check(UserData.ch1);
  // RCchannel[1] = ch_check(UserData.ch2);
  // RCchannel[2] = ch_check(UserData.ch1);
  // RCchannel[3] = ch_check(UserData.ch2);
  // RCchannel[4] = ch_check(UserData.ch3);
  // RCchannel[5] = ch_check(UserData.ch4);
  // RCchannel[6] = ch_check(UserData.ch5);
  // RCchannel[7] = ch_check(UserData.ch6);

  RMTsend(RCchannel,8);
  Serial.printf("ch1:%d   ch3:%d  \r\n",RCchannel[0], RCchannel[1]);
//Serial.printf("Angle:%.2f   brake:%d   throttle:%d  \r\n",MozaData.fSteeringWheelAngle, MozaData.brake, MozaData.throttle);
//Serial.write(MozaData.data,16);
// Serial.print("Received: ");
   delay(25);
}


#endif

