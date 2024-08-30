#ifndef ARM_H
#define ARM_H

#include <AccelStepper.h>
#include <TMCStepper.h>

#define EN_PIN 6    // Enable
#define DIR_PIN 5   // Direction
#define STEP_PIN 4  // Step
#define DIAG_PIN 1
#define SERIAL_PORT Serial1  // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00  // TMC2209 Driver address according to MS1 and MS2

#define R_SENSE 0.11f

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

using namespace TMC2208_n;

volatile bool stallGuardFlag = false;

void stallGuard() {
  stallGuardFlag = true;
  Serial.println("StallGuard");
}

void ArmInit() {
  Serial1.begin(250000, SERIAL_8N1, 3, 2);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(DIAG_PIN, INPUT);

  attachInterrupt(DIAG_PIN, stallGuard, RISING);

  // TMCSTEPPER SETTINGS
  driver.begin();
  driver.toff(3);                // enable driver in software
  driver.rms_current(600, 0.1);  // motor RMS current
  driver.seimin(1);  // minimum current for smart current control 0: 1/2 of IRUN
                     // 1: 1/4 of IRUN
  driver.semin(5);  // [0... 15] If the StallGuard4 result falls below SEMIN*32,
                    // the motor current becomes increased.
  driver.semax(4);  // [0... 15]  If the StallGuard4 result is equal to or above
                    // (SEMIN+SEMAX+1)*32, the motor current becomes decreased
                    // to save energy.
  driver.sedn(2);   // current down step speed 0-11%
  driver.seup(2);   // Current increment steps per measured StallGuard2 value 5
                    // seup0 %00 … %11: 1, 2, 4, 8
  driver.blank_time(24);
  driver.TCOOLTHRS(0x0FFFF);     // FFFFF
  driver.microsteps(16);         // microsteps
  driver.en_spreadCycle(false);  // spreadCycle
  // driver.shaft(false);             // direction
  driver.SGTHRS(50);

  stepper.setMaxSpeed(20000);      // 100mm/s @ 80 steps/mm
  stepper.setAcceleration(10000);  // 2000mm/s^2
  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();
}

void ArmTask() {
  //   xTaskCreatePinnedToCore(MotorTask, "Motor", 5000, NULL, 5, &Motor, 1);
}

void MotorTask(void*) {
  for (;;) {
    stepper.run();
    // vTaskDelay(1);
  }
}

void findZero(){
  attachInterrupt(DIAG_PIN, stallGuard, RISING);
  driver.VACTUAL(-5000);
  long now = millis();
  while(!stallGuardFlag && millis() - now < 13000){
    delay(10);
  }
  driver.VACTUAL(0);
}

#endif