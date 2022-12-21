import paho.mqtt.client as mqtt
from pathlib import Path

topic = "cluster/temp"
mqtt_server = "192.168.1.201"
tempfile = "/sys/class/thermal/thermal_zone0/temp"
node = "1"

path = Path(tempfile)
content = float(path.read_text())/1000

print(content)
message = node+ str(content)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("cluster/temp")

client = mqtt.Client()
client.on_connect = on_connect
client.connect(mqtt_server, 1883, 60)
client.publish(topic, payload=message, qos=0, retain=False)

