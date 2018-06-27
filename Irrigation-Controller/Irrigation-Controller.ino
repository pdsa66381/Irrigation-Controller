#include <SoftwareSerial.h>
#include <DS3232RTC.h>        // https://github.com/JChristensen/DS3232RTC
#include <LowPower.h>
#include "RTClib.h"

RTC_DS3231 rtc;
SoftwareSerial BT(10, 11); // RX, TX

const byte rtcAlarmPin = 2; // External interrupt on pin D3
String command = ""; // Stores response of bluetooth device
String response = "";
DateTime currentDT;
DateTime alarm1;
DateTime alarm2;
int duration1;
int duration2;

void setAlarm(int al, String dt, int duration){
  if(al==1){
      duration1=duration;
    }else{
        duration2=duration;
      }
  }

void setup(){
  duration1=0;
  duration2=0;
  Serial.begin(9600);
  Serial.println("Type AT commands!");
  delay(200);
  BT.begin(9600); // HC-06 usually default baud-rate
  delay(200);

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // set the alarm
  currentDT = rtc.now();
  Serial.println(getMyDateTime());
}

void loop(){
  if(BT.available()>0){
    while (BT.available() > 0) {
      char c = BT.read();
      delay(10); //Delay added to make thing stable
      command += c;
    }
  }
  delay(10);
  if(command == '1'){
    response = "1|" + getMyDateTime();
    size_t size = response.length();
    char *msg = malloc(size+1);
    response.toCharArray(msg, size+1);
    BT.println(msg);
    delay(10);
    free(msg);
   } else if(command == '2'){
    delay(10);
    int i=0;
    char rcvDt[20];
    while(BT.available()){
      char c_msg = BT.read(); //Conduct a serial read
      rcvDt[i]=c_msg;
      i++;
      delay(10); //Delay added to make thing stabl
    }
    setTime(rcvDt);
    } else if(command == '3'){
    water(command);
   } else if(command == '4'){
    Serial.println(command);
   } else if(command == '5'){
    Serial.println(command);
   } else if(command == '6'){
    Serial.println(command);
   } else if(command == '7'){
    Serial.println(command);
   }
   command = "";
   delay(10);
}

void setTime(char dt[20]){
//  int days = 0;
//  int months = 0;
//  int years = 0;
//  int hours = 0;
//  int minutes = 0;
//  int seconds = 0;
//  int i;
//  for (i = 0; i < 19; i++) {
//    Serial.println(myPins[i]);
//    }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  currentDT = rtc.now();
  }

void water(String msg){
  Serial.println("OPEN WATER");
}

void printDateTime(DateTime t)
{
  Serial.print((t.day() < 10) ? "0" : ""); Serial.print(t.day(), DEC); Serial.print('/');
  Serial.print((t.month() < 10) ? "0" : ""); Serial.print(t.month(), DEC); Serial.print('/');
  Serial.print(t.year(), DEC); Serial.print(' ');
  Serial.print((t.hour() < 10) ? "0" : ""); Serial.print(t.hour(), DEC); Serial.print(':');
  Serial.print((t.minute() < 10) ? "0" : ""); Serial.print(t.minute(), DEC); Serial.print(':');
  Serial.print((t.second() < 10) ? "0" : ""); Serial.println(t.second(), DEC);
}

String getMyDateTime(){
  String dtText = String(currentDT.day());
  dtText += "/";
  dtText += String(currentDT.month());
  dtText += "/";
  dtText += String(currentDT.year());
  dtText += " - ";
  dtText += String(currentDT.hour());
  dtText += ":";
  dtText += String(currentDT.minute());
  return dtText; 
}
