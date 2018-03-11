#define MY_DEBUG 
#define MY_ESP8266_HOSTNAME "Multical401Meter"
#define wifi_ssid "Sensors"
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
long lastSuccessfulChecksum;

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
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  pinMode(D4,INPUT);
  
  arduinoSer.begin(9600);
  lastSuccessfulChecksum = 0;
  Serial.println("SETUP OK!");
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
        //Serial.print(recieved);
        if (recieved != '\n')
        {
          inData += recieved;
        }

        // Process message when new line character is recieved
        if (recieved == '\n')
        {
          StaticJsonBuffer<400> jsonBuffer;
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
         

          const char* volumeResult = root["volume"];
          Serial.print("volume:");
          Serial.println(volumeResult);
           

          const char* hoursResult = root["hours"];
          Serial.print("hours:");
          Serial.println(hoursResult);
          

          const char* tempinResult = root["tempin"];
          Serial.print("tempin;");
          Serial.println(tempinResult);
         

          const char* tempoutResult = root["tempout"];
          Serial.print("tempout:");
          Serial.println(tempoutResult);
         

          const char* tempdiffResult = root["tempdiff"];
          Serial.print("tempdiff:");
          Serial.println(tempdiffResult);
        

          const char* powerResult = root["power"];
          Serial.print("power:");
          Serial.println(powerResult);
      

          const char* flowResult = root["flow"];
          Serial.print("flow:");
          Serial.println(flowResult);

          const char* checksumResult = root["checksum"];
          Serial.print("checksum:");
          Serial.println(checksumResult);
          
          long checksum = String(energyResult).toFloat() + String(volumeResult).toFloat() + atol(hoursResult) + String(tempinResult).toFloat() + String(tempoutResult).toFloat() + String(tempdiffResult).toFloat() + String(powerResult).toFloat() + atol(flowResult);
          Serial.print("calculatedChecksum: ");
          Serial.println(checksum);

          long checksumDiff = (checksum - atol(checksumResult));
          Serial.print("checksumDiff: ");
          Serial.println(checksumDiff);
          
          if(checksum == atol(checksumResult) && checksumDiff < 2000 && atol(tempdiffResult) < 100)
          {
          
          lastSuccessfulChecksum = checksum;
          Serial.println("Checksum OK! Sending over MQTT.");
          client.publish(energy_topic, String(energyResult).c_str(), true);
          client.publish("HeaterMeter/energy",String(energyResult).c_str(), true);
          client.publish(volume_topic, String(volumeResult).c_str(), true);
          client.publish("HeaterMeter/volume", String(volumeResult).c_str(), true);
          client.publish(hours_topic, String(hoursResult).c_str(), true);
          client.publish("HeaterMeter/hours", String(hoursResult).c_str(), true);
          client.publish(tempin_topic, String(tempinResult).c_str(), true);
          client.publish("HeaterMeter/tempin", String(tempinResult).c_str(), true);
          client.publish(tempout_topic, String(tempoutResult).c_str(), true);
          client.publish("HeaterMeter/tempout", String(tempoutResult).c_str(), true);
          client.publish(tempdiff_topic, String(tempdiffResult).c_str(), true);
          client.publish("HeaterMeter/tempdiff", String(tempdiffResult).c_str(), true);
          client.publish(power_topic, String(powerResult).c_str(), true);
          client.publish("HeaterMeter/power", String(powerResult).c_str(), true);
          client.publish(flow_topic, String(flowResult).c_str(), true);
          client.publish("HeaterMeter/flow", String(flowResult).c_str(), true);
          }
          
        }
    }
      
    }

