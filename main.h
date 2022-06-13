#ifndef _MAIN_H_
#define _MAIN_H_

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <string.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "function.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

/*PIN*/
#define SS_PIN 10
#define RST_PIN 9
#define IR_IN_OUT A0
#define Dir 8
#define EX 6
#define EZ 7
#define StepX 4
#define StepZ 5
#define vong 200
#define delayStep 1
/*variable*/
typedef struct {
  uint8_t RID[4];
  struct {
    uint8_t parkingStatus: 1; 
    uint8_t last_status : 1; 
    uint8_t spare : 6; // ko sài thì nó vẫn ở đó à oke  test thử hỉ
  }flag;
}parking_t;

extern SoftwareSerial debug;
extern  parking_t ListCar[10];

extern  QueueHandle_t xQueue;
extern  TaskHandle_t QuetTheHandle;
// extern TaskHandle_t NhanHandle;
extern  TaskHandle_t VanHanhHandle;

extern  long double timeout;



#endif