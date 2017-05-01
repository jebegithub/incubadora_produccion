#include <Time.h>
#include <TimeAlarms.h>
#include <DHT.h>
/*Prueba de LCD por protocolo I2C 
Mostraremos un mensaje y lo borraremos continuamente haciendo parpadear el backlight*/
// Importamos las librerias necesarias 
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>  // F Malpartida's NewLiquidCrystal library

/*-----( Declare Constants )-----*/
#define I2C_ADDR    0x3f  // I2C address for the device obtained with i2cscanner
#define DHTPIN 2 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define LED_OFF  0
#define LED_ON  1
#define DEBUG_INTERVAL 6 //loop interations before displaying temperature
#define LCD_REFRESH_INTERVAL 90 //loop iteractions before refreshing the T-H on the screen
#define LCD_REFRESH_INTERVAL_ACTIVITY 45
#define LOOP_DELAY 300 //delay in ms of every loop interaction

#define INCUBATOR_LED 3 //TO-DO confirm the number
#define HATCHER_LED 4 //TO-DO confirm the number

#define PUSH_1 8 //TURNER
//#define PUSH_2 7

#define OUT_FAN_RELAY 11
#define AIR_RELAY 12
#define HEAT_RELAY 10
#define TURN_RELAY 9
#define HATCHER_SWITCH 13 //TO-DO confirm the number

int button1State=0; //LIGHT
int button2State=0; //TURNER

int loop_counter=0;

int air_duration = 60;
int out_fan_duration=30;
int turner_duration = 13;
int turner_delay = 7200;
int out_fan_delay = 3600;
//Time in hours schedule the turner in minutes  (TO-DO TBC)
//Stored values to know when it turned the last time
int turner_day=1;
int turner_hour=0;
int turner_minute=0;

float stored_temp=0;
float stored_hum=0;
float current_temp=0;
float current_hum=0;

float tempMax;
float tempMin;

bool risingTemp = false;
bool hatcher_mode = false;
bool switch_status = false;

String lcd_line1 = "";
String lcd_line2 = "";

//                        addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
DHT dht(DHTPIN, DHTTYPE);



void setup()
{
  Serial.begin(9600);
  lcd.begin (16,2);  // inicializar lcd 
  lcd.setBacklight(LED_ON);
  refresh_lcd ("Iniciando...", "");
  
  pinMode(HATCHER_SWITCH, INPUT);
  pinMode(HATCHER_LED, OUTPUT); 
  pinMode(INCUBATOR_LED, OUTPUT); 
  
  pinMode(PUSH_1, INPUT);
//pinMode(PUSH_2, INPUT);
  
  pinMode(OUT_FAN_RELAY, OUTPUT);
  pinMode(AIR_RELAY, OUTPUT);
  pinMode(HEAT_RELAY, OUTPUT);
  pinMode(TURN_RELAY, OUTPUT);

  digitalWrite(INCUBATOR_LED,LOW);
  digitalWrite(HATCHER_LED,LOW);
  
  setTime(0,00,0,1,1,17); // set time to Saturday 8:29:00am Jan 1 2011
  Alarm.timerRepeat(turner_delay, program_turner);
  Alarm.timerRepeat(out_fan_delay, program_out_fan);
  switch_relays_off();
  check_hatcher();
}

void  loop(){  
  current_hum = dht.readHumidity();
  current_temp = dht.readTemperature();

  if (loop_counter % (LCD_REFRESH_INTERVAL/2) == 0){
  lcd_line1 =  "Act.:" + String(day()-1) + "d " + String(hour()) + "h " + String(minute()) + "m";
  if (switch_status){
      lcd_line2 = "-MODO NACEDORA-";
    }
  else{
      lcd_line2 =  "Giro:" + String(turner_day-1) + "d " + String(turner_hour) + "h " + String(turner_minute) + "m";
    }
  refresh_lcd (lcd_line1,lcd_line2);
  }

 //if (loop_counter % DEBUG_INTERVAL == 0){
    //digitalClockDisplay();
    //Serial.println(current_hum);
    //Serial.println(current_temp);
 // }
  
  if (!digitalRead(PUSH_1)){
    digitalWrite(TURN_RELAY, !digitalRead(TURN_RELAY));
    }

//  if (!digitalRead(PUSH_2)){
//  digitalWrite(LIGHT_RELAY, !digitalRead(LIGHT_RELAY));
//  }
  
  if (current_temp > 1 && current_temp < 60){
    if (current_temp < tempMin && !risingTemp){
        digitalClockDisplay();
        Serial.print("Temperatura actual: ");
        Serial.println(current_temp);
        Serial.println("TEMPERATURA LIMITE INFERIOR ALCANZADA, ENCENDIENDO TODO");
        digitalWrite(HEAT_RELAY, LOW);
        if (!risingTemp){
          risingTemp = true;
        }
        //switch the fans briefly on to move hot air down
        start_fans(6);
      }
      else {
        if (current_temp > tempMax && risingTemp){
          digitalClockDisplay();
          Serial.print("Temperatura actual: ");
          Serial.println(current_temp);
          Serial.println("TEMPERATURA LIMITE SUPERIOR ALCANZADA, APAGANDO TODO");
          digitalWrite(HEAT_RELAY, HIGH);
          risingTemp = false;
          stop_fans();
        }
      }
    }
  else{
      //In case of the termomether giving strange lectures, switch the relays off
      digitalClockDisplay();
      Serial.print("Temperatura actual: ");
      Serial.println(current_temp);
      Serial.println("ERROR DE LECTURA");
      switch_relays_off();
    }
  if ((loop_counter % LCD_REFRESH_INTERVAL == 0) && loop_counter!=0){
    loop_counter = 0;
    //Create lines with the average values of the last loops
    lcd_line1 =  "Temperatura:" + String(stored_temp/LCD_REFRESH_INTERVAL);
    lcd_line2 =  "Humedad:" + String(stored_hum/LCD_REFRESH_INTERVAL);
    //Display the updated values
    refresh_lcd (lcd_line1,lcd_line2);
    //Initializate the stored values
    stored_temp = 0;
    stored_hum = 0;
    }
  else{
    stored_temp = stored_temp + current_temp;
    stored_hum = stored_hum + current_hum;
    }
    loop_counter = loop_counter + 1;
    Alarm.delay(LOOP_DELAY); // wait between clock display
}

void refresh_lcd(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1); 
  lcd.setCursor(0,1);
  lcd.print(line2); 
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.println();
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}
  
void printDigits(int digits)
{
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void switch_relays_off(){
  digitalWrite(OUT_FAN_RELAY, HIGH);
  digitalWrite(AIR_RELAY, HIGH);
  digitalWrite(HEAT_RELAY, HIGH);
  digitalWrite(TURN_RELAY, HIGH);
}

bool check_hatcher(){
  switch_status = digitalRead(HATCHER_SWITCH);
  //Serial.println(switch_status);
   if (switch_status){
    Serial.print("\n---------------------MODO NACEDORA--------------");
    tempMax = 35.3;
    tempMin = 35.1;
    digitalWrite(HATCHER_LED,HIGH);
    }
  else{
    Serial.print("\n---------------------MODO INCUBADORA--------------");
    tempMax = 36.3;
    tempMin = 36.1;
    digitalWrite(INCUBATOR_LED,HIGH);
    }
  Serial.print("\nTemperaturas: \tMIN-> ");
  Serial.print(tempMin);
  Serial.print("\tMAX-> ");
  Serial.println(tempMax);
  return switch_status;
  }

void start_fans(int duration) {
  digitalClockDisplay();
  Serial.print(" Temperatura actual: ");
  Serial.println(current_temp);
  Serial.println("\tActivando Ventiladores interiorires");
  if (digitalRead(AIR_RELAY) == HIGH){
    digitalWrite(AIR_RELAY,LOW);
     Alarm.timerOnce(air_duration, stop_fans); 
    }   
}

void stop_fans(){
  digitalClockDisplay();
  Serial.print(" Temperatura actual: ");
  Serial.println(current_temp);
  Serial.println("\tParando Ventiladores interiorires");
  digitalWrite(AIR_RELAY,HIGH);
  }

void stop_turner(){
  digitalClockDisplay();
  Serial.print(" Temperatura actual: ");
  Serial.println(current_temp);
  Serial.println("\tParando Girador");
  digitalWrite(TURN_RELAY,HIGH);
  }

void stop_out_fan(){
  digitalClockDisplay();
  Serial.print(" Temperatura actual: ");
  Serial.println(current_temp);
  Serial.println("\tParando Ventilador exterior");
  digitalWrite(OUT_FAN_RELAY,HIGH);
  }

void program_turner() {
    digitalClockDisplay();
    Serial.print(" Temperatura actual: ");
    Serial.println(current_temp);
    Serial.print("\tActivando Girador");
    digitalWrite(TURN_RELAY,LOW);
    Alarm.timerOnce(turner_duration, stop_turner); 

    turner_day = day();
    turner_hour = hour();
    turner_minute = minute();
    
}

void program_out_fan(){
    digitalClockDisplay();
    Serial.print(" Temperatura actual: ");
    Serial.println(current_temp);
    digitalWrite(OUT_FAN_RELAY,LOW);
    Serial.print("\tActivando Ventialdor exterior");
    Alarm.timerOnce(out_fan_duration, stop_out_fan); 
  }
