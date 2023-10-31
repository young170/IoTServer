from flask import Flask, render_template
from flask_mqtt import Mqtt
import time
import json

app = Flask(__name__)

app.config['MQTT_BROKER_URL'] = 'sweetdream.iptime.org'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'iot'
app.config['MQTT_PASSWORD'] = 'csee1414'
mqtt = Mqtt(app)

global mqtt_message
mqtt_message = ''

global mqtt_temp
global mqtt_hum
global mqtt_cds

mqtt_temp = ''
mqtt_hum = ''
mqtt_cds = ''

print('@@ Use URL:/iot/21900413/(led, dht22, cds)')

@app.route('/iot/21900413/<cmd>')
def get_command_1(cmd):
    global mqtt_temp
    global mqtt_hum
    global mqtt_cds

    pub_topic = 'iot/21900413'
    data = {}

    if cmd == 'temp':
        data['temp'] = 1
    elif cmd == 'cds':
        data['cds'] = 1
    elif cmd == 'dht22':
        data['dht22'] = 1
    elif cmd == 'led':
        data['led'] = 1
    elif cmd == 'led_on':
        data['led_on'] = 1
    elif cmd == 'led_off':
        data['led_off'] = 1
    elif cmd == 'usb':
        data['usb'] = 1
    elif cmd == 'usb_on':
        data['usb_on'] = 1
    elif cmd == 'usb_off':
        data['usb_off'] = 1

    mqtt.publish(pub_topic, json.dumps(data, indent='\t'))

    time.sleep(1)
    return render_template('PJ1_RPI_index.html', jh_temp=mqtt_temp, jh_hum=mqtt_hum, jh_cds=mqtt_cds)

@app.route('/iot/22100113/<cmd>')
def get_command_2(cmd):

    global mqtt_temp
    global mqtt_hum
    global mqtt_cds

    pub_topic = 'iot/22100113'
    data = {}

    if cmd == 'temp':
        data['temp'] = 1
    elif cmd == 'cds':
        data['cds']
    elif cmd == 'dht22':
        data['dht22'] = 1
    elif cmd == 'led':
        data['led'] = 1
    elif cmd == 'led_on':
        data['led_on'] = 1
    elif cmd == 'led_off':
        data['led_off'] = 1
    elif cmd == 'usb':
        data['usb'] = 1
    elif cmd == 'usb_on':
        data['usb_on'] = 1
    elif cmd == 'usb_off':
        data['usb_off'] = 1

    mqtt.publish(pub_topic, json.dumps(data, indent='\t'))
    time.sleep(1)
    return render_template('PJ1_RPI_index.html', sb_temp=mqtt_temp, sb_hum=mqtt_hum, sb_cds=mqtt_cds)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe("iot/21900413/hdt")
    mqtt.subscribe("iot/21900413/cds")

    mqtt.subscribe("iot/22100113/hdt")
    mqtt.subscribe("iot/22100113/cds")

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global mqtt_temp
    global mqtt_hum
    global mqtt_cds

    topic = message.topic
    payload = message.payload.decode('utf-8')
    json_to_dic = json.loads(payload)
    mqtt_temp = json_to_dic['temp']
    mqtt_hum = json_to_dic['hum']
    mqtt_cds = json_to_dic['cds']
    print("Topic: ", topic, "temp: ", mqtt_temp, "hum: ", mqtt_hum, "cds: ", mqtt_cds)

@app.route('/')
def index():
    return render_template('PJ1_RPI_index.html')



if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, port=5000)
