# IoTServer
IoT Server using NodeMCU and Raspberry pi

## Description
* Class
  * ECE30003-01 IoT 시스템 설계
* Contributors
  * 22100113 김성빈
  * 21900413 심정훈

## Directory Structure
```
Doc/
    IoTServer_Report.docx
Design/
    IoTServer/
        IoTServer.ino
    PJ1_web_server.py
    templates/
        PJ1_RPI_index.html
```

## Build
How to compile and run the code.<br>
This description assumes the user has the Arduino IDE and appropriate libraries installed.<br>
1. Modify the `mqttTopic` variable to match your topic.
2. Modify Wifi data, `WifiSSID` and `WifiPassword` to match your Wifi.
3. Compile code.
4. Upload to connected NodeMCU 1.0 (ESP-12E Module) board.

## Libraries
Main libraries used.<br>
1. ESPMQTTClient
2. Arduino_JSON
3. DHTesp

## NodeMCU
### NodeMCU Pins
* `LED_PIN`: D0
* `DHT_PIN`: D3
* `RELAY_PIN`: D4
* `CDS_PIN`: A0
