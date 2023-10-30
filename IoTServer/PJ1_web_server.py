from flask import Flask, render_template
from flask_mqtt import Mqtt
import time

app = Flask(__name__)

app.config['MQTT_BROKER_URL'] = 'sweetdream.iptime.org'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'iot'
app.config['MQTT_PASSWORD'] = 'csee1414'
mqtt = Mqtt(app)

global mqtt_message
mqtt_message = ''

print('@@ Use URL:/iot/21900413/(led, dht22, cds)')

@app.route('/iot/21900413/<cmd>')
def get_command(cmd):
    global mqtt_message
    
    pub_topic = 'iot/21900413'
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
    for temp in data.items():
        mqtt_message = temp[1]
    return render_template('PJ1_RPI_index.html', result=mqtt_message)

@app.route('/iot/22100113/<cmd>')
def get_command(cmd):
    global mqtt_message
    
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
    for temp in data.items():
        mqtt_message = temp[1]
    return render_template('PJ1_RPI_index.html', result=mqtt_message)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe(sub_topic_dht22)
    mqtt.subscribe(sub_topic_cds)

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global mqtt_message
    topic = message.topic
    payload = message.payload.decode('utf-8')
    mqtt_message = payload
    print("Topic: ", topic, "message: ", mqtt_message)

@app.route('/')
def index():
    return render_template('lab7_index.html')



if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, port=5000)
