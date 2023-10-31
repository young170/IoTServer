#if 0
/*
 * Example of parsing JSON with <Arduino_JSON.h>
 * Change the WifiSSID, WifiPassword for testing
 */

#include "EspMQTTClient.h"
#include <Arduino_JSON.h>

#define LED_PIN 16

#define mqtt_clientname "21900413@JeongHun"
#define mqtt_broker "sweetdream.iptime.org"
#define MQTTUsername "iot"
#define MQTTPassword "csee1414"

const char *WifiSSID = "sjh";
const char *WifiPassword = "123456789f";

int led_state = LOW;
String command = "";

JSONVar rpi_json;

EspMQTTClient mqtt_client(
    WifiSSID,
    WifiPassword,
    mqtt_broker,     // MQTT Broker server ip
    MQTTUsername,    // Can be omitted if not needed
    MQTTPassword,    // Can be omitted if not needed
    mqtt_clientname, // Client name that uniquely identify your device
    1883             // The MQTT port, default to 1883. this line can be omitted
);

void setup()
{
  Serial.begin(115200);

  // For Debuging
  mqtt_client.enableDebuggingMessages();                                          // Enable debugging messages sent to serial output
}

void onConnectionEstablished()
{
  // topic be changed by our NodeMCU
  mqtt_client.subscribe("iot/21900413", [](const String &payload)
                        {  
                          rpi_json = JSON.parse(payload); 

                          if ((int) rpi_json["cds"] == 1)
                            Serial.print("cds is 1");
                          else if ((int) rpi_json["dht"] == 1)
                            Serial.print("dht is 1");
                          else if ((int) rpi_json["led"] == 1)
                            Serial.print("led is 1");
                          else if ((int) rpi_json["led_on"] == 1)
                            Serial.print("led_on is 1");
                          else if ((int) rpi_json["led_off"] == 1)
                            Serial.print("led_off is 1");
                          else if ((int) rpi_json["usb"] == 1)
                            Serial.print("usb is 1");
                          else if ((int) rpi_json["usb_on"] == 1)
                            Serial.print("usb_on is 1");
                          else if ((int) rpi_json["usb_off"] == 1)
                            Serial.print("usb_off is 1");
                          
                        });
}

void loop()
{
  mqtt_client.loop();

}
#endif
