#include <SoftwareSerial.h>
#include <DS3232RTC.h>        // https://github.com/JChristensen/DS3232RTC
#include <LowPower.h>
#include "RTClib.h"

RTC_DS3231 rtc;
SoftwareSerial BT(10, 11); // RX, TX
DateTime currentDT;
DateTime future_alarm;

const byte rtcAlarmPin = 2; // External interrupt on pin D3
int solenoidPin = 4; //water valve
bool opened = false;
char command[30];

// char [] = {hour, minutes, duration}
int al1[] = {0, 0, 0};
int al2[] = {0, 0, 0};
bool active1=false;
bool active2=false;

void clearCommands(){
  int i;
  int sz = sizeof(command) - 1;
  for(i=0;i<sz; i++){
    command[i] = 0;
    }
  }

String getMyDateTime(){
  String dtText = "1|";
  currentDT = rtc.now();
  dtText += String(currentDT.day());
  dtText += "/";
  dtText += String(currentDT.month());
  dtText += "/";
  dtText += String(currentDT.year());
  dtText += " - ";
  dtText += String(currentDT.hour());
  dtText += ":";
  dtText += String(currentDT.minute());
  dtText += ":";
  dtText += String(currentDT.second());
  return dtText; 
}

void setDateTime(char *info){
  char* data = strtok(info, "|");
  char* dt = strtok(data[1], "-");
  char* tm = strtok(NULL,"-");

  String days = String(dt[0]) + String(dt[1]);
  String months = String(dt[3]) + String(dt[4]);
  String years = String(dt[6]) + String(dt[7]) + String(dt[8]) + String(dt[9]);
  String hours= String(tm[0]) + String(tm[1]);
  String mins = String(tm[3]) + String(tm[4]);
  String secs = String(tm[6]) + String(tm[7]);
  
  setTime(hours.toInt(), mins.toInt(), secs.toInt(), days.toInt(), months.toInt(), years.toInt());
  //RTC.set(now()); 
  rtc.adjust(now());
  Serial.println(getMyDateTime());
  }

String getAlarm(char al){
  String dtText;
  if(al == '4'){
    dtText += "4|";
    dtText += al1[0];
    dtText += ":";
    dtText += al1[1];
    dtText += "|";
    dtText += al1[2];
    dtText += "|";
    dtText += active1; 
  }else{
    dtText += "5|";
    dtText += String(al2[0]);
    dtText += ":";
    dtText += String(al2[1]);
    dtText += "|";
    dtText += String(al2[2]);
    dtText += "|";
    dtText += String(active2); 
  }
  return dtText; 
}

void setAlarm(int al, int hrs, int mins, int duration, bool state){
  DateTime now = rtc.now();
  Serial.println(state);
  if(al==6){
    al1[0] = hrs;
    al1[1] = mins;
    al1[2] = duration;
    active1 = state;

    RTC.alarm(ALARM_1);
    if(hrs > now.hour()){
      RTC.setAlarm(ALM1_MATCH_DATE, 0, mins, hrs, now.day());
    } else {
      DateTime future (now + TimeSpan(0, hrs, mins, 0)); //Days, Hours, Minutes, Seconds
      printDateTime(future);
      RTC.setAlarm(ALM1_MATCH_DATE, future.second(), future.minute(), future.hour(), future.day());
    }
    
  }else{
    al2[0] = hrs;
    al2[1] = mins;
    al2[2] = duration;
    active2 = state;

    if(state){
      RTC.alarm(ALARM_2);
      RTC.alarmInterrupt(ALARM_2, true); // Enable alarm 2 interrupt A1IE   
      if(hrs > now.hour()){
        RTC.setAlarm(ALM2_MATCH_DATE, 0, mins, hrs, now.day());
      } else {
        DateTime future (now + TimeSpan(0, hrs, mins, 0)); //Days, Hours, Minutes, Seconds
        printDateTime(future);
        RTC.setAlarm(ALM2_MATCH_DATE, future.second(), future.minute(), future.hour(), future.day());
      }
    }else{
      RTC.alarmInterrupt(ALARM_2, false);
    }  
  }
}

void defineAlarm(char *info){
  char* data = strtok(info, "|");
  char* tm = strtok(NULL, "|");
  char* duration = strtok(NULL, "|");
  char* state = strtok(NULL,"|");
  char* hr = strtok(tm, ":");
  char* mins = strtok(NULL, ":");
  setAlarm(int(atoi(data)), int(atoi( hr )), int(atoi( mins )), int(atoi( duration )), bool(state));
}

void irrigate(){
  if(opened){
    digitalWrite(solenoidPin, LOW); //Switch Solenoid OFF
    opened = false;
  } else{
    digitalWrite(solenoidPin, HIGH); //Switch Solenoid ON
    opened = true;
  }  
}

//duration in seconds
void irrigation(int duration){
  digitalWrite(solenoidPin, HIGH); //Switch Solenoid ON
  opened = true;
  delay(duration*1000);
  digitalWrite(solenoidPin, LOW); //Switch Solenoid OFF
  opened = false;  
}

void sendMsg(String data){
  size_t size = data.length();
  char *msg = malloc(size+1);
  data.toCharArray(msg, size+1);
  BT.println(msg);
  delay(10);
  free(msg);
}

void setup(){
  pinMode(solenoidPin, OUTPUT);
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
    int i = 0;
    while (BT.available() > 0) {
      char c = BT.read();
      delay(10); //Delay added to make thing stable
      command[i] = c;
      i++;
    }
    command[i]='\0';
    delay(10);
    if(command[0] == '1'){
      String response = getMyDateTime();
      sendMsg(response);
     } else if(command[0] == '2'){
      setDateTime(command);
      } else if(command[0] == '3'){
      irrigate();
     } else if((command[0] == '4') || (command[0] == '5')){
      String response = getAlarm(int(command[0]));
      sendMsg(response);
     } else if((command[0] == '6') || (command[0] == '7')){
      defineAlarm(command);
     }
     clearCommands();
     delay(10);
    }
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
