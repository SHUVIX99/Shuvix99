
#include "main.h"
SoftwareSerial debug(2,3);
MFRC522 mfrc522(SS_PIN, RST_PIN);
parking_t ListCar[10];

QueueHandle_t xQueue;
TaskHandle_t QuetTheHandle;
TaskHandle_t VanHanhHandle;
long double timeout; 
static void QuetThe( void *pvParameters );
static void VanHanh( void *pvParameters );


void setup() {
      Serial.begin(9600);   
      debug.begin(9600);
      SPI.begin();    
      mfrc522.PCD_Init();
      xQueue = xQueueCreate( 5, 10);
      LoadData();
      Serial.print("END\n");
      debug.println("Setup");
      xTaskCreate( QuetThe, "QuetThe", 130,NULL, 1,&QuetTheHandle );
      xTaskCreate( VanHanh, "VanHanh", 150,NULL, 1,&VanHanhHandle );
      vTaskStartScheduler();
}

static void QuetThe( void *pvParameters )
{
  int Arr_Send[5]={0,0,0,0,0};        //The 4 values of this array contain the value of the RFID Card
  pinMode(IR_IN_OUT,INPUT_PULLUP);    // Infrared sensor
  char *u8_ptr;  
  for( ;; )
  { 
    if(Serial.available()) 
    {
      timeout = millis() + 100; 
      String data = "";
    
      while(timeout > millis()) 
      {
        if(Serial.available()){
          data += (char)Serial.read();
          timeout = millis() + 20;
        }
      }
      if(data.indexOf("Xuat:") != -1) 
      {
          u8_ptr = strstr(data.c_str(), "Xuat:");
          if(u8_ptr != nullptr) u8_ptr += 5; 
          debug.println((String)u8_ptr);
          if(*u8_ptr =='1') Arr_Send[4]=-1;
          else if(*u8_ptr =='2') Arr_Send[4]=-2;
          else if(*u8_ptr =='3') Arr_Send[4]=-3;
          else if(*u8_ptr =='4') Arr_Send[4]=-4;
          else if(*u8_ptr =='5') Arr_Send[4]=-5;
          else if(*u8_ptr =='6') Arr_Send[4]=-6;
          else if(*u8_ptr =='7') Arr_Send[4]=-7;
          else if(*u8_ptr =='8') Arr_Send[4]=-8;
          else if(*u8_ptr =='9') Arr_Send[4]=-9;
          else if(*u8_ptr == '0') Arr_Send[4]=-10;

          if(  xQueueSendToBack( xQueue, &Arr_Send, 0 ) != pdPASS )  /// pdPass la nhiem vu hanh cong true con  errQUEUEFULL la false
                {
                  Serial.println( "Could not send to the queue.\r\n" );
                }
      } 
      else {
        debug.println("data false");
      }
    }
    if( Arr_Send[0]==0 &&  Arr_Send[1]==0 &&  Arr_Send[2]==0 &&  Arr_Send[3]==0 ) // Check if you have received your new card
    {
      debug.println("Chua nhan duoc the");
      delay(1000); 
    }
    if (!mfrc522.PICC_IsNewCardPresent()) continue; 	// Neu khong phat hien co the moi thi bo qua, khong lam phan con lai, chay ve dau ham for(;;), luu y, la the moi, the cu da doc chua bo ra khoi dau doc thi se khong tinh.
		if (!mfrc522.PICC_ReadCardSerial()) continue; 	// Neu khong doc duoc id the thi cung bo qua het phan con lai, quay ve dau ham for(;;) 
    for (byte i = 0; i < mfrc522.uid.size; i++)     //Store value of RFID Card into Arr_Send array 
    { 
      Arr_Send[i]=mfrc522.uid.uidByte[i];
      debug.print(Arr_Send[i],HEX); 
      debug.print("");
    }
    debug.println();
    mfrc522.PICC_HaltA();  
    mfrc522.PCD_StopCrypto1();
    /*Sort vehicles based on comparison with previously stored RFID into Arr_Send[4]
      IT has a value from -10 :10 with positive values indicating the vehicle entry at that location 
      and the negative value indicating exiting the vehicle at that location. 
      The value of 0 alone means running out of Slot.
    */
    if(Arr_Send[0] != 0 && Arr_Send[1] != 0 && Arr_Send[2] != 0 && Arr_Send[3] != 0) 
    {
    
      Arr_Send[4]=Sort_Car(Arr_Send); // Sort Car Return location

      if(Arr_Send[4]==0)
        {
          Serial.println("Bãi giữ xe đã đầy"); 
        }
      else if(Arr_Send[4]>0)
        {
          if(digitalRead(IR_IN_OUT)==LOW) 
            {
              debug.print("Import:"); debug.println(Arr_Send[4]);
              if(  xQueueSendToBack( xQueue, &Arr_Send, 0 ) != pdPASS )  /// pdPass la nhiem vu hanh cong true con  errQUEUEFULL la false
                {
                  Serial.println( "Could not send to the queue.\r\n" );
                }
              //  vTaskPrioritySet(VanHanhHandle,2);
            }
          else   debug.println("Vị trí nhập đang trống");  
        }
      else
        {
          if(digitalRead(IR_IN_OUT)==LOW)
            {
              debug.println("Vị trí xuất đang đầy");
            }
          else
            {
              debug.print("Export:"); debug.println(Arr_Send[4]);
              if(  xQueueSendToBack( xQueue, &Arr_Send, 0 ) != pdPASS )  /// pdPass la nhiem vu hanh cong true con  errQUEUEFULL la false
                {
                  Serial.println( "Could not send to the queue.\r\n" );
                }
              //vTaskPrioritySet(VanHanhHandle,2);
            }
        }
    }
    Arr_Send[0]=0; Arr_Send[1]=0; Arr_Send[3]=0; Arr_Send[2]=0; Arr_Send[4]=0; // refresh memory
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

static void VanHanh( void *pvParameters )
{
  int Arr_Receive[5]; //Receive Quêu from Task Quetthe
  int A4988;
  pinMode(EX,OUTPUT);
  pinMode(EZ,OUTPUT);
  pinMode(Dir,OUTPUT);
  pinMode(StepX,OUTPUT);
  pinMode(StepZ,OUTPUT);
  for(;;)
  {
    debug.println("Task2 nè \r");
    if( xQueueReceive( xQueue, &Arr_Receive,portMAX_DELAY) == pdPASS )
    {
      while(true) // send location to My App until "OK1" signal is received through Uart communication from My APP
        {
          timeout = millis();
          switch (Arr_Receive[4])
            {
              case 1:  {Serial.print("Nhap:1\n"); break;}
              case 2:  {Serial.print("Nhap:2\n"); break;}
              case 3:  {Serial.print("Nhap:3\n"); break;}
              case 4:  {Serial.print("Nhap:4\n"); break;}
              case 5:  {Serial.print("Nhap:5\n"); break;}
              case 6:  {Serial.print("Nhap:6\n"); break;}
              case 7:  {Serial.print("Nhap:7\n"); break;}
              case 8:  {Serial.print("Nhap:8\n"); break;}
              case 9:  {Serial.print("Nhap:9\n"); break;}
              case 10: {Serial.print("Nhap:10\n"); break;}
              case -1:  {Serial.print("Xuat:1\n"); break;}
              case -2: {Serial.print("Xuat:2\n"); break;}
              case -3:  {Serial.print("Xuat:3\n"); break;}
              case  -4:  {Serial.print("Xuat:4\n"); break;}
              case -5:  {Serial.print("Xuat:5\n"); break;}
              case -6:  {Serial.print("Xuat:6\n"); break;}
              case -7:  {Serial.print("Xuat:7\n"); break;}
              case -8:  {Serial.print("Xuat:8\n"); break;}
              case -9:  {Serial.print("Xuat:9\n"); break;}
              case -10: {Serial.print("Xuat:10\n"); break;}
            }

          while(millis()-timeout<800)
            {
              // debug.println(millis());
              if(Serial.available()) 
              {
                timeout = millis() + 100; 
                String data = "";
                while(timeout > millis()) 
                {
                  if(Serial.available()){
                    data += (char)Serial.read();
                    timeout = millis() + 20;
                    }
                }
                if(data.indexOf("OK1") != -1) {goto T1;}
              }
            }
        }
      //send RFID Card to My App until "OK2" signal is received through Uart communication from My APP
      T1:
      if(Arr_Receive[4]>0)
        {
          int8_t vl=Arr_Receive[4]-1;
            while(true)
            {
                timeout = millis();
                Serial.print("[RFID]");
                Serial.print(vl);
                Serial.print("[");
                Serial.print((Arr_Receive[0] < 10) ? "00" :(Arr_Receive[0] < 100 ? "0" :"") );
                Serial.print(Arr_Receive[0]);
                Serial.print(",");
                Serial.print((Arr_Receive[1] < 10) ? "00" :(Arr_Receive[1] < 100 ? "0" :"") );
                Serial.print(Arr_Receive[1]);
                Serial.print(",");
                Serial.print((Arr_Receive[2] < 10) ? "00" :(Arr_Receive[2] < 100 ? "0" :"") );
                Serial.print(Arr_Receive[2]);
                Serial.print(",");
                Serial.print((Arr_Receive[3] < 10) ? "00" :(Arr_Receive[3] < 100 ? "0" :"") );
                Serial.print(Arr_Receive[3]);
                Serial.print("]\n");
                while(millis()-timeout<1000)
                {
                  if(Serial.available()) 
                  {
                    timeout = millis() + 100; 
                    String data = "";
                    while(timeout > millis())
                    {
                      if(Serial.available()){
                      data += (char)Serial.read();
                      timeout = millis() + 20;
                      }
                    }
                    if(data.indexOf("OK2") != -1) {goto T2;}
                  }
                }
            }
        }
      T2:
       //Update the variables to the database
      A4988=Arr_Receive[4];
      if(Arr_Receive[4]>0) 
        {
            ListCar[Arr_Receive[4]-1].RID[0]=Arr_Receive[0];
            ListCar[Arr_Receive[4]-1].RID[1]=Arr_Receive[1];
            ListCar[Arr_Receive[4]-1].RID[2]=Arr_Receive[2];
            ListCar[Arr_Receive[4]-1].RID[3]=Arr_Receive[3];
            ListCar[Arr_Receive[4]-1].flag.parkingStatus=true;
        }
      else if(Arr_Receive[4]<0)
        {
            Arr_Receive[4]=-Arr_Receive[4];
            ListCar[Arr_Receive[4]-1].RID[0]=0;
            ListCar[Arr_Receive[4]-1].RID[1]=0;
            ListCar[Arr_Receive[4]-1].RID[2]=0;
            ListCar[Arr_Receive[4]-1].RID[3]=0;
            ListCar[Arr_Receive[4]-1].flag.parkingStatus=false;
        }
      else
          {
            //LCD bao cao bai do da day
          }
        //code A4988
        if(A4988==1) Xevao1();
        else if(A4988==-1) Xera1();
      int i=3;
      while(i)
        {
            i--;
            Serial.print("XuLyXong");
            delay(700);
        }
      debug.println("XuLyXong");
     
      
      
    }
     vTaskDelay(100/portTICK_PERIOD_MS);
     //vTaskPrioritySet(VanHanhHandle, 0 ); 
  }
}

void loop() {
 
}