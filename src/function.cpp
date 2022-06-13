#include "function.h"

void Xevao1()
{
  digitalWrite(EX,LOW);
  digitalWrite(EZ,LOW);
  debug.println("Bat Dau Nhap Xe");
  SetZ_Down(8);
  SetX_Up();
  SetZ_Up(8);
  SetX_Down();
  SetZ_Down(32);
  SetX_Up();
  SetZ_Down(8);
  SetZ_Up(40);
  debug.println("Nhap Xe hoan tat");
  digitalWrite(EX,HIGH);
  digitalWrite(EZ,HIGH);
}
void Xera1()
{
  digitalWrite(EX,LOW);
  digitalWrite(EZ,LOW);
  debug.println("Bat Dau Xuat Xe");
  SetZ_Down(40);
  SetX_Up();
  SetZ_Up(8);
  SetX_Down();
  SetZ_Up(32);
  SetX_Up();
  SetZ_Down(8);
  SetZ_Up(8);
  debug.println("Xuat Xe hoan tat");
  digitalWrite(EX,HIGH);
  digitalWrite(EZ,HIGH);

}
void SetZ_Up(int j)
{
  digitalWrite(Dir,LOW);
  // Serial.println("Z_UP");
  for(int i=0;i<j;i++)
  {
       for(int i=0;i<vong;i++)
      {
        digitalWrite(StepZ,!digitalRead(StepZ));
        delay(delayStep);
      }
      delay(1);
  }
  // Serial.println("Z_UP Done");
}
void SetZ_Down(int j)
{
  digitalWrite(Dir,HIGH);
  // Serial.println("Z_DOWN");
  for(int i=0;i<j;i++)
  {
         for(int i=0;i<vong;i++)
      {
        digitalWrite(StepZ,!digitalRead(StepZ));
        delay(delayStep);
      }
      delay(1);
  }
  // Serial.println("Z_DOWN Done");
}
void SetX_Up()
{
  digitalWrite(Dir,LOW);
  // Serial.println("X_UP");
    for(int i=0;i<28;i++)
  {
   for(int i=0;i<vong;i++)
      {
        digitalWrite(StepX,!digitalRead(StepX));
        delay(delayStep);
      }
    delay(1);
   }
  //  Serial.println("X_Up Done");
}
void SetX_Down()
{
    digitalWrite(Dir,HIGH);
    // Serial.println("X_Down");
    for(int i=0;i<28;i++)
  {
   for(int i=0;i<vong;i++)
      {
        digitalWrite(StepX,!digitalRead(StepX));
        delay(delayStep);
      }
    delay(1);
   }
  //  Serial.println("X_Down Done");
}
int Sort_Car(int Arr_Send[])
{
    int i=0;
    for(int j=9;j>=0;j--)
        {
          if(Arr_Send[0]== ListCar[j].RID[0] && Arr_Send[1]== ListCar[j].RID[1] && Arr_Send[2]== ListCar[j].RID[2] && Arr_Send[3]== ListCar[j].RID[3] && ListCar[j].flag.parkingStatus ==true )
          {
            Arr_Send[4]=-j-1;
              break;
          }
          else if(ListCar[j].flag.parkingStatus == false)
          {
            Arr_Send[4]=j+1;
          }
        }
    return Arr_Send[4];
}
void LoadData()
{
  
  while(true)
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
      if(data.indexOf("[STT]") != -1) SlotStatusSerialHandle(data.c_str()); 
      else if(data.indexOf("[RFID]") != -1) SloRFIDSerialHandle(data.c_str());
      else if(data.indexOf("[END]") != -1) {Serial.print("END\n");break;}
      else {
        debug.println("data false");
      }
      
    }  
  }
}
void SlotStatusSerialHandle(const char *s) {
  
  debug.print("Xu ly STT: ");
  debug.println(s);
  Serial.print("OK\n");
  char data_arr[10];
  char *u8_ptr;  // 
  u8_ptr = strstr(s, "[STT]["); // cai nay no tra ve index dau tien nha

  if(u8_ptr != nullptr) u8_ptr += 5;  // 
  debug.println((String)u8_ptr);
  
  for(int i = 0; i < sizeof(u8_ptr); i++) {
    if(i%2) data_arr[i/2] = (uint8_t)(u8_ptr[i] - '0'); 
  }
  for(int i = 0; i < 10; i++) {
    ListCar[i].flag.parkingStatus= (data_arr[i])? 1 : 0;
  }
}
void SloRFIDSerialHandle(const char *s) {
  String datatemp="000";
  Serial.println("OK2\n");
  debug.print("Xu ly RFID: ");
  debug.println(s);
  
  int Sl=s[1]-48;
  for(int i = 0; i < 4; i++)
  {
    datatemp[0]=s[i*4+3];
    datatemp[1]=s[i*4+4];
    datatemp[2]=s[i*4+5];
    int dat=datatemp.toInt();
    ListCar[Sl].RID[i]=dat;
    debug.println(dat);
    
  }
}

