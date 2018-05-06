/************************* Functional parameters *********************************/
#define AIR_DURATION 60
#define OUT_FAN_DURATION 30
#define TURNER_DURATION 13
#define TURNER_DELAY 7200
#define OUT_FAN_DELAY 3600
#define MAX_CONSECUTIVE_FAIL 3

/************************* Pin definition *********************************/
#define HEATER_PIN 16 //WEMOS D0 - Relay R1 -- Orange
#define OUT_FANS_PIN 3 //WEMOS Rx - Relay R4 - Blue
#define IN_FANS_PIN 4 //WEMOS D2 Relay R3 -- Green
#define TURNER_PIN 5 //WEMOS D1 Relay R2 --Yellow
#define SWITCH_PIN 12 //WEMOS D6
#define DHTPIN 2 //WEMOS D4

/************************* Reference temperatures *********************************/
#define HATCHER_MAX_TEMP 37.0
#define HATCHER_MIN_TEMP 36.8
#define INCUBATOR_MAX_TEMP 37.5
#define INCUBATOR_MIN_TEMP 37.2

