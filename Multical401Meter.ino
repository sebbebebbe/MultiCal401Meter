#include <ArduinoJson.h>
#include <SoftwareSerial.h>

// Pin definitions
#define PIN_KAMSER_RX  9  // Kamstrup IR interface RX
#define PIN_KAMSER_TX  10  // Kamstrup IR interface TX
#define PIN_LED        13  // Standard Arduino LED

#define PIN_ESP_RX 4
#define PIN_ESP_TX 5

SoftwareSerial kamSer(PIN_KAMSER_RX, PIN_KAMSER_TX, false);  // Initialize serial
SoftwareSerial esp8266Ser(PIN_ESP_RX, PIN_ESP_TX, false); //Initialize serial for ESP communication

void setup () {
  Serial.begin(57600);
  esp8266Ser.begin(9600);
  Serial.print("Initializing...");
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, 0);
  
  // setup kamstrup serial
  pinMode(PIN_KAMSER_RX,INPUT);
  pinMode(PIN_KAMSER_TX,OUTPUT);

  pinMode(PIN_ESP_RX, INPUT);
  pinMode(PIN_ESP_TX, OUTPUT);
  
  Serial.println("done.");
}

bool parity_check(unsigned input) {
    bool inputparity = input & 128;
    int x = input & 127;
 
    int parity = 0;
    while(x != 0) {
        parity ^= x;
        x >>= 1;
    }

    if ( parity & 0x1 != inputparity )
      return(1);
    else
      return(0);
}

void loop () {
    byte sendmsg1[] = { 175,163,177 };            //   /#1 with even parity
    
    byte r  = 0;
    byte to = 0;
    byte i;
    char message[255];
    int parityerrors;
    
    digitalWrite(PIN_LED, 1);
    
    kamSer.begin(300);
    for (int x = 0; x < 3; x++) {
      kamSer.write(sendmsg1[x]);
    }
    
    kamSer.flush();
    kamSer.begin(1200);

    to = 0;
    r = 0;
    i = 0;
    parityerrors = 0;
    char *tmpstr;
    float m_energy, m_volume, m_tempin, m_tempout, m_tempdiff, m_power;
    long m_hours, m_flow;
    
    while(r != 0x0A)
    {
      if (kamSer.available())
      {
        // receive byte
        r = kamSer.read();
        if (parity_check(r))
        {
           parityerrors += 1;
        }
        r = r & 127; // Mask MSB to remove parity
        
        message[i++] = char(r);
      }
      else
      {
        to++;
        delay(25);
      }
    
      if (i>=79)
      {
        if ( parityerrors == 0 )
        {
          Serial.print("OK: " );
          message[i] = 0;
          
          tmpstr = strtok(message, " ");
          if (tmpstr)
           m_energy = atol(tmpstr)/1000.0;
          else
           m_energy = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_volume = atol(tmpstr)/100.0;
          else
           m_volume = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_hours = atol(tmpstr);
          else
           m_hours = 0;
  
          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_tempin = atol(tmpstr)/100.0;
          else
           m_tempin = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_tempout = atol(tmpstr)/100.0;
          else
           m_tempout = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_tempdiff = atol(tmpstr)/100.0;
          else
           m_tempdiff = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_power = atol(tmpstr)/10.0;
          else
           m_power = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
           m_flow = atol(tmpstr);
          else
           m_flow = 0;

          Serial.print(m_energy,3);
          Serial.print(" ");
          Serial.print(m_volume);
          Serial.print(" ");
          Serial.print(m_hours);
          Serial.print(" ");
          Serial.print(m_tempin);
          Serial.print(" ");
          Serial.print(m_tempout);
          Serial.print(" ");
          Serial.print(m_tempdiff);
          Serial.print(" ");
          Serial.print(m_power,1);
          Serial.print(" ");
          Serial.print(m_flow);
          Serial.println("");

          StaticJsonBuffer<257> jsonBuffer;
          JsonObject& root = jsonBuffer.createObject();
          root["energy"] = m_energy;
          root["volume"] = m_volume;
          root["hours"] = m_hours;
          root["tempin"] = m_tempin;
          root["tempout"] = m_tempout;
          root["tempdiff"] = m_tempdiff;
          root["power"] = m_power;
          root["flow"] = m_flow;
          
          root.prettyPrintTo(Serial);
          root.printTo(esp8266Ser);
          esp8266Ser.println("");
          
        }
        else
        {
          Serial.print("ERR(PARITY): " );
          message[i] = 0;
          Serial.println(message);
        }
        break;
      } 
      if (to>100)
      {
        message[i] = 0;
        Serial.print("ERR(TIMEOUT):" );
        Serial.println(message);
        break;
      }
    }
    digitalWrite(PIN_LED, 0);

    delay(30000);
}
