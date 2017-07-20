//**************
//IR receive demo v1.0
//******************************
#include <IRSendRev.h>
//#include <IRSendRevInt.h>
#define IR_OUT_PIN 2//The OUT pin of the Infrared Receiver is connected to D2 of Arduino/Catduino
void setup()
{
    Serial.begin(38400);
    IR.Init(IR_OUT_PIN);
    Serial.println("init over");
}

unsigned char dta[20];

void loop()
{
    if(IR.IsDta())
    {
       // IR.Recv(dta);
        int length= IR.Recv(dta);
        for (int i =0;i<length;i++)
        {
          Serial.print(dta[i]);
          Serial.print("\t");
        }
        Serial.println();
// Very Important:
// the received data is comprised of the trsmission parameters , please refer to 
// the sendTest.ino in the library ;
    }
    
    
}
