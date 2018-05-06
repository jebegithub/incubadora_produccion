/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "incubadora"
#define WLAN_PASS       "huevo1234"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Jebe"
#define AIO_KEY         "84c40e9594a448568f40b493608721c0"

bool Wifi_connect();
bool MQTT_connect();
void publishValues(float temp, float hum);
