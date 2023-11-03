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

# For save data from nodeMCU1 
mqtt_temp1 = ''
mqtt_hum1 = ''
mqtt_cds1 = ''

# For save data from nodeMCU2
mqtt_temp2 = ''
mqtt_hum2 = ''
mqtt_cds2 = ''

print('@@ Use URL:/iot/21900413/(led, hum, cds)')

# When a command is executed via a button on the web
@app.route('/iot/21900413/<cmd>')
def get_command_1(cmd):
    global mqtt_temp1
    global mqtt_hum1
    global mqtt_cds1

    pub_topic = 'iot/21900413' # MQTT topic 
    data = {}

    # checking command
    if cmd == 'data':
        data['data'] = 1
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

    # publish json data
    # json.dumps(data, indent='\t') convert dictionary to json datastructure
    mqtt.publish(pub_topic, json.dumps(data, indent='\t'))

    # wait response
    time.sleep(1)

    # Show the result to user
    return render_template('PJ1_RPI_index.html', jh_temp=mqtt_temp1, jh_hum=mqtt_hum1, jh_cds=mqtt_cds1, sb_temp=mqtt_temp2, sb_hum=mqtt_hum2, sb_cds=mqtt_cds2)

@app.route('/iot/22100113/<cmd>')
def get_command_2(cmd):

    global mqtt_temp2
    global mqtt_hum2
    global mqtt_cds2

    pub_topic = 'iot/22100113'
    data = {}
    
    # checking command
    if cmd == 'data':
        data['data'] = 1
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

    # publish json data
    # json.dumps(data, indent='\t') convert dictionary to json datastructure
    mqtt.publish(pub_topic, json.dumps(data, indent='\t'))

    # wait response
    time.sleep(1)

    # Show the result to user
    return render_template('PJ1_RPI_index.html', jh_temp=mqtt_temp1, jh_hum=mqtt_hum1, jh_cds=mqtt_cds1, sb_temp=mqtt_temp2, sb_hum=mqtt_hum2, sb_cds=mqtt_cds2)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    # subscribe 2 topic for get a data from two nodeMCU
    mqtt.subscribe('iot/21900413/data')
    mqtt.subscribe('iot/22100113/data')

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global mqtt_temp1
    global mqtt_hum1
    global mqtt_cds1

    global mqtt_temp2
    global mqtt_hum2
    global mqtt_cds2

    # Parsing json data to dictionary
    topic = message.topic
    payload = message.payload
    json_to_dic = json.loads(payload)
    print(payload)

    # Determine where the data comes from and save data
    if topic == 'iot/21900413/data':
        mqtt_temp1 = json_to_dic['temp']
        mqtt_hum1 = json_to_dic['hum']
        mqtt_cds1 = json_to_dic['cds']

    elif topic == 'iot/22100113/data':
        mqtt_temp2 = json_to_dic['temp']
        mqtt_hum2 = json_to_dic['hum']
        mqtt_cds2 = json_to_dic['cds']
    
@app.route('/')
def index():
    return render_template('PJ1_RPI_index.html')



if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, port=5000)
