#define MY_DEBUG 
#define MY_ESP8266_HOSTNAME "Multical401Meter"
#define wifi_ssid ""
#define wifi_password ""

#define mqtt_server "192.168.88.99"
#define mqtt_user ""
#define mqtt_password ""

#define energy_topic "MySensorsOut/100/0/1/0/48"
#define volume_topic "MySensorsOut/100/1/1/0/35"
#define hours_topic "MySensorsOut/100/2/1/0/48"
#define tempin_topic "MySensorsOut/100/3/1/0/0"
#define tempout_topic "MySensorsOut/100/4/1/0/0"
#define tempdiff_topic "MySensorsOut/100/5/1/0/0"
#define power_topic "MySensorsOut/100/6/1/0/18"
#define flow_topic "MySensorsOut/100/7/1/0/48"

#include <PubSubClient.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <Dhcp.h>
#include <EthernetServer.h>
#include <Dns.h>
#include <EthernetUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial arduinoSer(D4, D5, false); //Initialize serial for ESP communication RX D4 TX D5
String inData;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.hostname("HeaterMeter");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(57600);       
  arduinoSer.begin(9600);
  Serial.println("SETUP OK!");

  setup_wifi();
  client.setServer(mqtt_server, 1883);

}

void loop()
{
if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
  while (arduinoSer.available() > 0)
    {
        char recieved = arduinoSer.read();
        if (recieved != '\n')
        {
          inData += recieved; 
        }

        // Process message when new line character is recieved
        if (recieved == '\n')
        {
          StaticJsonBuffer<315> jsonBuffer;
          Serial.println(inData);
          JsonObject& root = jsonBuffer.parseObject(inData);
          
          inData = ""; // Clear recieved buffer

          if (!root.success()) {
            Serial.println("parseObject() failed");
            return;
          }

          const char* energyResult = root["energy"];
          Serial.print("energy:");
          Serial.println(energyResult);
          client.publish(energy_topic, String(energyResult).c_str(), true);

          const char* volumeResult = root["volume"];
          Serial.print("volume:");
          Serial.println(volumeResult);
          client.publish(volume_topic, String(volumeResult).c_str(), true);

          const char* hoursResult = root["hours"];
          Serial.print("hours:");
          Serial.println(hoursResult);
          client.publish(hours_topic, String(hoursResult).c_str(), true);

          const char* tempinResult = root["tempin"];
          Serial.print("tempin;");
          Serial.println(tempinResult);
          client.publish(tempin_topic, String(tempinResult).c_str(), true);

          const char* tempoutResult = root["tempout"];
          Serial.print("tempout:");
          Serial.println(tempoutResult);
          client.publish(tempout_topic, String(tempoutResult).c_str(), true);

          const char* tempdiffResult = root["tempdiff"];
          Serial.print("tempdiff:");
          Serial.println(tempdiffResult);
          client.publish(tempdiff_topic, String(tempdiffResult).c_str(), true);

          const char* powerResult = root["power"];
          Serial.print("power:");
          Serial.println(powerResult);
          client.publish(power_topic, String(powerResult).c_str(), true);

          const char* flowResult = root["flow"];
          Serial.print("flow:");
          Serial.println(flowResult);
          client.publish(flow_topic, String(flowResult).c_str(), true);
          
        }
    }
      
    }

