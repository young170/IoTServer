#include "EspMQTTClient.h"
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

// NodeMCU pin numbers
#define D0 16
#define D3 0
#define D4 2

// Sensors and actuators
#define LED_PIN D0
#define CDS_PIN A0
#define RELAY_PIN D4
#define DHTPIN D3
// DHT type
#define DHTTYPE DHT22

// Relay states, active-low
#define RELAY_OFF HIGH
#define RELAY_ON LOW
int relayState = LOW;

// OLED display dimensions
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // display data

// CDS light intensity value
int lightValue;

// led light state
int ledLightState = LOW;

// USBLED states
enum state {
  CMD_ON_STATE = 0,
  LIGHT_STATE,
  EVENT_STATE,
  DARK_STATE
};
enum state currState = LIGHT_STATE;

// USBLED light interval
unsigned long lightTimer = 0;
const long lightInterval = 10000; // 10s

// DHT22 data
DHTesp dht;
float temperature, humidity;

// DHT22 read dhtInterval
unsigned long dhtCurrMillis;
unsigned long dhtPrevMillis = 0;
const long dhtInterval = 10000; // 10s

// cds read cdsInterval
unsigned long cdsCurrMillis;
unsigned long cdsPrevMillis = 0;
const long cdsInterval = 1000; // 1s

// MQTT data
#define mqttBroker "sweetdream.iptime.org"
#define mqttClientname "22100113@SB"
#define MQTTUsername "iot"
#define MQTTPassword "csee1414"

const char *WifiSSID = "NTH413";
const char *WifiPassword = "cseenth413";
String mqttTopic = "iot/22100113";

EspMQTTClient mqttClient(
  WifiSSID,
  WifiPassword,
  mqttBroker,      // MQTT Broker server ip
  MQTTUsername,     // Can be omitted if not needed
  MQTTPassword,     // Can be omitted if not needed
  mqttClientname,  // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void display_sensors(float temperature, float humidity, int lightValue) {
  // setup display
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // write to display buffer, sensor values
  display.print(F("T: "));
  display.print(temperature);
  display.println(F("C"));

  display.print(F("H: "));
  display.print(humidity);
  display.println(F("%"));

  display.print(F("L: "));
  display.println(lightValue);

  display.display();
}

////////////// setup() //////////////
void setup() {
  Serial.begin(9600);

  // init WiFi connection
  WiFi.mode(WIFI_STA);

  // init OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  display.clearDisplay();

  // init pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // init state, off

  pinMode(CDS_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF); // init state, off

  // init DHT22
  dht.setup(DHTPIN, DHTesp::DHTTYPE);

  // enable mqtt debugging
  // mqttClient.enableDebuggingMessages();
  // mqttClient.enableHTTPWebUpdater();
}

////////////// loop() //////////////
void loop() {
  // mqtt subscribe
  mqttClient.loop();

  // current time, cds and dht
  cdsCurrMillis = millis();
  dhtCurrMillis = millis();

  // read current light value, 1s
  if (cdsCurrMillis - cdsPrevMillis >= cdsInterval) {
    cdsPrevMillis = cdsCurrMillis;
    lightValue = analogRead(CDS_PIN);
  }

  // read DHT22 every 10000 millis, 10s
  if (dhtCurrMillis - dhtPrevMillis >= dhtInterval) {
    dhtPrevMillis = dhtCurrMillis;

    // delay(dht.getMinimumSamplingPeriod());
    temperature = dht.getTemperature();
    humidity = dht.getHumidity();

    // check validity of DHT22 values
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor");
    }
    else {
      // display dht + cds values to OLED display, and publish data to mqttTopic
      display_sensors(temperature, humidity, lightValue);
      cds_dht_mqtt_publish();
    }
  }

  // // control USBLED depending on cds value
  if (CMD_ON_STATE != currState) {                    // CMD has highest precedence
    if (lightValue > 100) {                           // if bright state
      currState = LIGHT_STATE;                        // current state = bright state
    }
    else if (lightValue < 50) {                     // if dark state
      if (LIGHT_STATE == currState) {                 // if prev state = light state, an event
        currState = EVENT_STATE;
        lightTimer = millis();                        // start 10s timer
      }
      else {                                        // if prev state = dark state, not an event
        if (millis() - lightTimer > lightInterval) {  // if 10s passed
          currState = DARK_STATE;                     // turn off USBLED
        }
        else {                                      // if 10s did not passed
          currState = EVENT_STATE;                    // keep USBLED on
        }
      }
    }
  }

  // if CMD_ON is given, or event occured: turn on USBLED
  if (CMD_ON_STATE == currState || EVENT_STATE == currState) {
    relayState = RELAY_ON;
    digitalWrite(RELAY_PIN, relayState);
  }
  else {
    relayState = RELAY_OFF;
    digitalWrite(RELAY_PIN, relayState);
  }
}

// publish {temperature, humidity, light_intensity} to mqtt
void cds_dht_mqtt_publish(void) {
  // JSON init
  JSONVar myObject;
  myObject["temp"] = (int) temperature;
  myObject["hum"] = (int) humidity;
  myObject["cds"] = lightValue;  
  String jsonString = JSON.stringify(myObject);

  String pubTopic = mqttTopic += "/data"
  mqttClient.publish(pubTopic, jsonString);
}

// on mqtt connection established, read from subscribed topic
void onConnectionEstablished() {
  mqttClient.subscribe(mqttTopic, [](const String & payload) {
    // JSON init
    JSONVar rpiJson = JSON.parse(payload);

    if ((int) rpiJson["cds"] == 1) {
      String cds_mqttTopic = mqttTopic += "/cds";
      mqttClient.publish(mqttTopic, String(lightValue));

      Serial.print("cds");
    }
    else if ((int) rpiJson["temp"] == 1) {
      String temp_mqttTopic = mqttTopic += "/temp";
      mqttClient.publish(mqttTopic, String(temperature));

      Serial.print("temperature");
    }
    else if ((int) rpiJson["hum"] == 1) {
      String temp_mqttTopic = mqttTopic += "/hum";
      mqttClient.publish(mqttTopic, String(humidity));

      Serial.print("humidity");
    }
    else if ((int) rpiJson["led"] == 1) { // led toggle
      ledLightState = !ledLightState; // inverse current state
      digitalWrite(LED_PIN, ledLightState);

      Serial.print("led toggle");
    }
    else if ((int) rpiJson["led_on"] == 1) {
      ledLightState = HIGH;
      digitalWrite(LED_PIN, ledLightState);

      Serial.print("led_on");
    }
    else if ((int) rpiJson["led_off"] == 1) {
      ledLightState = LOW;
      digitalWrite(LED_PIN, ledLightState);

      Serial.print("led_off");
    }
    else if ((int) rpiJson["usb"] == 1) { // usbled toggle
      if (currState != EVENT_STATE) { // usbled toggle doesn't affect event state
        relayState = !relayState; // inverse current state
        digitalWrite(RELAY_PIN, relayState);
        
        if (RELAY_ON == relayState) { // if toggling turned usbled on
          currState = CMD_ON_STATE;
        } else { // if toggling turned usbled off
          currState = DARK_STATE;
        }
      }

      Serial.print("usb toggle");
    }
    else if ((int) rpiJson["usb_on"] == 1) {
      relayState = RELAY_ON;
      digitalWrite(RELAY_PIN, relayState);
      currState = CMD_ON_STATE;

      Serial.print("usb_on");
    }
    else if ((int) rpiJson["usb_off"] == 1) {
      lightTimer = -10; // handle edge case, usb_off right away

      relayState = RELAY_OFF;
      digitalWrite(RELAY_PIN, relayState);
      currState = DARK_STATE;

      Serial.print("usb_off");
    }
  });

  mqttClient.publish(mqttTopic, "Greetings from NodeMCU");
}
