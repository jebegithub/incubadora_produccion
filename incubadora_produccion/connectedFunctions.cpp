#include "connectedFunctions.h"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temp_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish hum_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");

bool Wifi_connect(){
  bool result = false;
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("                                                                   [ACTION]");
    Serial.print("Not connected. Will try to connect to SSID ");
    Serial.println(WLAN_SSID);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    unsigned long started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started <= 3000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("                                                                   [INFO]");
    Serial.print("Wifi status after attempt -> ");
    Serial.println(WiFi.status());
    if (WiFi.status() == WL_CONNECTED){
        Serial.println("                                                                   [INFO]");
        Serial.println("Successfully connected to Wifi");
        Serial.println("                                                                   [INFO]");
        Serial.println("IP address: "); Serial.println(WiFi.localIP());
        result=true;
        }
    else{
        Serial.println("                                                                                [ERROR]");
        Serial.println("Failure connecting, will try on the next publish attempt");
        result = false;
      }
    }
    else{
       result = true;
       } 
    return result;
  }

  bool MQTT_connect() {
  int8_t result;
  Serial.println("                                                                   [ACTION]");
  Serial.println("Connecting to MQTT... ");
  result = mqtt.connect();
  Serial.println("                                                                   [INFO]");
  Serial.print("Result -> ");
  Serial.println(result);
  if (result !=0){
    Serial.println("                                                                                [ERROR]");
    Serial.print("Error connecting to MQTT -> ");
    Serial.println(mqtt.connectErrorString(result));
    }
  return (result == 0);
}

void publishValues(float temp, float hum){
  Serial.println("                                                                   [INFO]");
  Serial.println("Will try to publish ");
  if (!temp_feed.publish(temp)) {
    Serial.println("                                                                                [ERROR]");
    Serial.println(F("Failed to publish on Temperature feed"));
    }
  else{
    Serial.println("                                                                   [SUCCESS]");
    Serial.println(F("Success publishing on Temperature feed"));
    if (!hum_feed.publish(hum)) {
      Serial.println("                                                                                [ERROR]");
      Serial.println(F("Failed to publish on Humidity feed"));
      }
    else{
      Serial.println("                                                                   [SUCCESS]");
      Serial.println(F("Success publishing on Humidity feed"));
      }
    }
  mqtt.disconnect();
  }
