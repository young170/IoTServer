#include "EspMQTTClient.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
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

// OLED display dimensions
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // display data

// CDS light intensity value
int lightValue;

// USBLED states
enum state {
  CMD_ON_STATE,
  LIGHT_STATE,
  FIRST_DARK_STATE,
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
unsigned long prevMillis = 0;
const long dhtInterval = 10000; // 10s

// MQTT data
#define mqtt_broker "sweetdream.iptime.org"
#define mqtt_clientname "espNode@HGU"
#define MQTTUsername "iot"
#define MQTTPassword "csee1414"

const char *WifiSSID = "NTH413";
const char *WifiPassword = "cseenth413";
const char *mqtt_topic = "iot/22100113";

EspMQTTClient mqtt_client(
  WifiSSID,
  WifiPassword,
  mqtt_broker,      // MQTT Broker server ip
  MQTTUsername,     // Can be omitted if not needed
  MQTTPassword,     // Can be omitted if not needed
  mqtt_clientname,  // Client name that uniquely identify your device
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
  WiFi.enableInsecureWEP(true);
  WiFi.begin(WifiSSID, WifiPassword);

  // connect NodeMCU to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");

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
}

////////////// loop() //////////////
void loop() {
  // mqtt subscribe
  // if (!mqtt_client.isConnected()) {
  //   Serial.print(".");
  //   mqtt_client.setMqttReconnectionAttemptDelay(500);
  // }
  // Serial.println("mqtt connected");
  mqtt_client.loop();

  // current time
  unsigned long currMillis = millis();

  // read current light value
  lightValue = analogRead(CDS_PIN);

  // read DHT22 every 10000 millis, 10s
  if (currMillis - prevMillis >= dhtInterval) {
    prevMillis = currMillis;

    // delay(dht.getMinimumSamplingPeriod());
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();

    // check validity of DHT22 values
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor");
    } else {
      // display dht + cds values to OLED display, and publish data to mqtt_topic
      display_sensors(temperature, humidity, lightValue);
      // mqtt_client.publish("iot/22100113", "test_display()");
    }
  }

  // control USBLED depending on cds value
  if (CMD_ON_STATE != currState) {                    // CMD has highest precedence
    if (lightValue > 190) {                           // if bright state
      currState = LIGHT_STATE;                        // current state = bright state
    } else if (lightValue < 140) {                    // if dark state
      if (LIGHT_STATE == currState) {                 // if prev state = light state, an event
        currState = FIRST_DARK_STATE;
        lightTimer = millis();                        // start 10s timer
      } else {                                        // if prev state = dark state, not an event
        if (millis() - lightTimer > lightInterval) {  // if 10s passed
          currState = DARK_STATE;                     // turn off USBLED
        } else {                                      // if 10s did not passed
          currState = FIRST_DARK_STATE;               // keep USBLED on
        }
      }
    }
  }

  // if CMD_ON is given, or event occured: turn on USBLED
  if (CMD_ON_STATE == currState || FIRST_DARK_STATE == currState) {
    digitalWrite(RELAY_PIN, RELAY_ON);
  } else {
    digitalWrite(RELAY_PIN, RELAY_OFF);
  }
}

void onConnectionEstablished() {
  mqtt_client.subscribe(mqtt_topic, [](const String & payload) {
    Serial.println(payload);

    /*
    payload json
    cds
    dht
    led
    change led state
    */
  });

  // // publish data to mqtt
  // StaticJsonBuffer<200> jsonBuffer;
  // char *json;
  // sprintf(json, "{\"temp\":%3.2f,\"cds\":%d,\"hum\":%3.2f}", temperature, lightValue, humidity);
  // JsonObject& root = jsonBuffer.parseObject(json);
  // if(!root.success()) {
  //   Serial.println("parseObject() failed");
  // }
  mqtt_client.publish("iot/22100113", "test_connected()");
}
