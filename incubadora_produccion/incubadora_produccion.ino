#include <TimeAlarms.h>
#include <Wire.h>
#include <DHT.h>
#include <Time.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"` -  Library for the SSD1306 OLED screen
#include "incubadora.h"
#include "custom_font.h"
#include "connectedFunctions.h"

SSD1306  display(0x3c, D3, D5);
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE, 15);

//Temp values are modified on boot up by setReferenceTemps function
float tempMax = 0;
float tempMin = 0;
float currentTemp = 0;
float currentHum = 0;
int readFailure = 0;
int loopCounter = 0;
bool hatcher_status = true;

void setup() {
  pinMode(IN_FANS_PIN, OUTPUT);
  pinMode(TURNER_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT); 
  hatcher_status = digitalRead(SWITCH_PIN);
  Serial.begin(9600);
  //Serial.setTimeout(2000);
  delay(10);
  pinMode(OUT_FANS_PIN, OUTPUT); //RX pin wont be used for serial, so it is set up after setting serial
  Serial.println("                                                                   [ACTION]");
  Serial.println("Device Started");
  Wifi_connect();
  //TO-DO check if the screen can be started before the Wifi
  display.init();
  display.flipScreenVertically();
  setTime(0,00,0,1,1,18);
  //TO-DO check if relays can be turned off before
  switch_relays_off();
  setReferenceTemps(hatcher_status);
  if (!hatcher_status){
    Serial.println("                                                                   [ACTION]");
    Serial.println("Programming turner and air routines");
    Alarm.timerRepeat(TURNER_DELAY, program_turner);
    Alarm.timerRepeat(OUT_FAN_DELAY, program_out_fan);
  }
}

void loop() {
 Serial.println("----------------------------------------------------------------------------------------");
 digitalClockDisplay();
 Serial.print("   ");
 Serial.print("LOOP: ");
 Serial.print(loopCounter);
 Serial.print("   ");
 Serial.print("HEAT: ");
 Serial.print(digitalRead(HEATER_PIN));
 Serial.print("   ");
 Serial.print("IN_FAN: ");
 Serial.print(digitalRead(IN_FANS_PIN));
 Serial.print("   ");
 Serial.print("OUT_FAN: ");
 Serial.print(digitalRead(OUT_FANS_PIN));
 Serial.print("   ");
 Serial.print("TUNER: ");
 Serial.println(digitalRead(TURNER_PIN));
 
 currentHum = dht.readHumidity();
 currentTemp = dht.readTemperature();
 Serial.print("\t Tempreature: ");
 Serial.print(currentTemp);
 Serial.print("\t Humidity: ");
 Serial.println(currentHum);
  if (isnan(currentTemp) || currentTemp < 1 || currentTemp > 50) {
      readFailure ++;
      Serial.println("                                                                                [ERROR]");
      Serial.print("Failed to read from DHT sensor!, Consecutive failures = ");
      Serial.println(readFailure);
      if (readFailure > MAX_CONSECUTIVE_FAIL){
        Serial.println("                                                                   [ACTION]");
        Serial.println("SWITCHING ALL RELAYS OFF DUE TO SEVERAL READING ERRORS");
        switch_relays_off();
        }
      }
  else{
    readFailure = 0;
    if (currentTemp < tempMin){
      if (digitalRead(HEATER_PIN) == HIGH){       
        Serial.println("                                                                   [ACTION]");
        Serial.println("LOWER TEMP LIMIT REACHED ----- TURN HEAT ON");
        digitalWrite(HEATER_PIN, LOW);
      }
      if (digitalRead(IN_FANS_PIN) == HIGH){
        start_in_fans();
      }
      }
    if (currentTemp > tempMax){
      if (digitalRead(HEATER_PIN) == LOW){       
        Serial.println("                                                                   [ACTION]");
        Serial.println("HIGH TEMP LIMIT REACHED +++++ TURN HEAT OFF");
        digitalWrite(HEATER_PIN, HIGH);
      }
      stop_in_fans();
      }
    }
    
  if (loopCounter % 300 == 0){
    loopCounter = 0;
    if (Wifi_connect() && MQTT_connect()){
      publishValues(currentTemp, currentHum);
      }
     else{
      Serial.println("                                                                                [ERROR]");
      Serial.println("Server is unreachable to publish the data on the feed");
      }
    }
  else if (loopCounter % 12 == 0){
    displayString(timeToString(), 24, 0, 26, true);
    displayString(returnDigits(day()-1) + " DÍAS", 18, 0, 0, false);
    }
   else if (loopCounter % 9 == 0){
    displayString("MODO", 18, 0, 0, true);
    displayString(hatcher_status ? "NACEDORA" : "INCUBADORA", 18, 0, 32, false);
    } 
   else if (loopCounter % 6 == 0){
    displayString("HUMEDAD", 18, 0, 0, true);  
    displayString( floatToString(currentHum) + "%", 24, 0, 32, false);
    }  
  else if (loopCounter % 3 == 0){
    displayString("TEMPERATURA", 16, 0, 0, true);
    displayString( floatToString(currentTemp) + "ºC", 24, 0, 32, false);
    }
  loopCounter ++;
  Alarm.delay(1000);
}

String floatToString(float value){
  char buffer[5];
  String floatString = dtostrf(value, 5, 2, buffer);
  return floatString;
  }

void start_in_fans() {
  if (digitalRead(IN_FANS_PIN) == HIGH){
    digitalWrite(IN_FANS_PIN,LOW);
     Alarm.timerOnce(AIR_DURATION, stop_in_fans); 
    }
  Serial.println("                                                                   [ACTION]");  
  Serial.println("Switching ON inner fans");
}

void stop_in_fans(){
  if (digitalRead(IN_FANS_PIN) == LOW){
    digitalWrite(IN_FANS_PIN,HIGH);
    Serial.println("                                                                   [ACTION]");
    Serial.println("Switching OFF inner fans");
    }
  }

void switch_relays_off(){
  digitalWrite(OUT_FANS_PIN, HIGH);
  digitalWrite(IN_FANS_PIN, HIGH);
  digitalWrite(TURNER_PIN, HIGH);
  digitalWrite(HEATER_PIN, HIGH);
  Serial.println("                                                                   [INFO]");
  Serial.println("All realays are switched off");  
}

void displayString(String s, int fontsize, int x, int y, bool clearScreen){
  if (clearScreen){
    display.clear();
    }
  switch (fontsize){
    case 10: display.setFont(ArialMT_Plain_10);break;
    case 16: display.setFont(Dialog_bold_16);break;
    case 18: display.setFont(Dialog_plain_18);break;
    case 24: display.setFont(ArialMT_Plain_24);break;
    default: display.setFont(Dialog_plain_18);break;
    }
  display.drawString(x,y,s);
  display.display();
}

void program_turner() {
  digitalWrite(TURNER_PIN,LOW);
  Alarm.timerOnce(TURNER_DURATION, stop_turner);
  Serial.println("                                                                   [INFO]");
  Serial.print("Turner will stop in ");
  Serial.print(TURNER_DURATION);
  Serial.println("seconds ");
}

void program_out_fan(){
  digitalWrite(OUT_FANS_PIN,LOW);
  Alarm.timerOnce(OUT_FAN_DURATION, stop_out_fan);
  Serial.println("                                                                   [INFO]");
  Serial.print("Outer fan will stop in ");
  Serial.print(OUT_FAN_DURATION);
  Serial.println("seconds ");
  }

void stop_turner(){
  digitalWrite(TURNER_PIN,HIGH);
  Serial.println("                                                                   [ACTION]");
  Serial.println("Stopping turner");
  }

void stop_out_fan(){
  digitalWrite(OUT_FANS_PIN,HIGH);
  Serial.println("                                                                   [ACTION]");
  Serial.println("Stopping outer fan");
  }


String timeToString(){
  String timeString = returnDigits(hour()) + ":" + returnDigits(minute()) + ":" + returnDigits(second());
  return timeString;
  }

String returnDigits(int digits)
{
  String fullDigits = String(digits);
  if(digits < 10)
    fullDigits = "0" + fullDigits;
  return fullDigits;
}
  
void setReferenceTemps(bool hatcher_status){
   if (hatcher_status){
    Serial.println("                                                                   [INFO]");
    Serial.println("------------------HATCHER MODE--------------");
    tempMax = HATCHER_MAX_TEMP;
    tempMin = HATCHER_MIN_TEMP;
    }
  else{
    Serial.println("                                                                   [INFO]");
    Serial.println("------------------INCUBATOR MODE------------");
    tempMax = INCUBATOR_MAX_TEMP;
    tempMin = INCUBATOR_MIN_TEMP;
    }
  Serial.println("                                                                   [ACTION]");
  Serial.println("Setting reference temperatures");
  Serial.print("Temp MAX = ");
  Serial.println(tempMax);
  Serial.print("Temp MIN = ");
  Serial.println(tempMin);
  }

void digitalClockDisplay()
{
  Serial.print(returnDigits(hour()));
  Serial.print(":");
  Serial.print(returnDigits(minute()));
  Serial.print(":");
  Serial.print(returnDigits(second()));
}



