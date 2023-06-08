#include<Arduino.h>
#include<analogWrite.h>
#include <string.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ThingSpeak.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <time.h>

/* OM2M integration starts */

#define CSE_IP "esw-onem2m.iiit.ac.in"
#define CSE_PORT 443
#define OM2M_ORGIN "iudxvc:erwsrt"
#define OM2M_MN "/~/in-cse/in-name/"
#define OM2M_AE "Team-1"
#define OM2M_DATA_CONT "Node-1/Data"
#define INTERVAL 15000L

const char * ntpServer = "pool.ntp.org";

HTTPClient http;

void CreateCI(int cnt1, int cnt2)
{
    String data;
    String server = "https://" + String() + CSE_IP + ":" + String() + CSE_PORT + String() + OM2M_MN;

    http.begin(server + String() + OM2M_AE + "/" + OM2M_DATA_CONT + "/");

    http.addHeader("X-M2M-Origin", OM2M_ORGIN);
    http.addHeader("Content-Type", "application/json;ty=4");
    http.addHeader("Content-Length", "100");

    data = "[" + String(cnt1) + ", " + String(cnt2) + "]"; 
    String req_data = String() + "{\"m2m:cin\": {"

      +
      "\"con\": \"" + data + "\","

      +
      "\"lbl\": \"" + "V1.0.0" + "\","

      //+ "\"rn\": \"" + "cin_"+String(i++) + "\","

      +
      "\"cnf\": \"text\""

      +
      "}}";
    int code = http.POST(req_data);
    http.end();
    Serial.println(code);  
} 

/* OM2M code ends */

const int inputPin1  = 12;    // Pin 15 of L293D IC
const int inputPin2  = 14;    // Pin 10 of L293D IC
const int EN1 = 13;           // Pin 1  of L293D IC

unsigned long channel_num = 1922371;

const char* ssid="Pranav";
const char* password="13254769";

const char* myWriteAPIKey = "GZO40J315M8AWC35";
const char* myReadAPIKey = "9PK1URG5QE0ECH0F";
WiFiClient client; 
WebServer server(80);   


int encoder_pin = 2;
int rpm;
volatile byte pulses;
double time_old;
unsigned int pulses_per_turn = 20; // Depends on the number of spokes on the encoder wheel

void count() // Counting the number of pulses for calculation of rpm
{
  pulses++;  
}

void setup()
{
    pinMode(EN1, OUTPUT);   // enabling pins of DC motor
    pinMode(inputPin1, OUTPUT);
    pinMode(inputPin2, OUTPUT);

    pulses=0;
    time_old=0;
  
    pinMode(encoder_pin,INPUT); // Setting encoder-1 pin as Input
    attachInterrupt(digitalPinToInterrupt(encoder_pin), count, FALLING ); // Triggering count function everytime the encoder pin1 turns from 1 to 0
  
    Serial.begin(115200);
    // pwm modulation value varies between 0 and 255
    
    WiFi.begin(ssid,password);   
    while(WiFi.status()!=WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    
    Serial.println(WiFi.localIP());
    ThingSpeak.begin(client);      
    server.begin();  

     /* OM2M code starts */
     
     configTime(0, 0, ntpServer);

     /* ends */
}

float read_voltage = 0, old_voltage = -1;
int speed;
bool flag = false;

void loop()
{
    server.handleClient();

    // reading the voltage value from Thingspeak;

    read_voltage = ThingSpeak.readFloatField(channel_num, 1, myReadAPIKey);
   
    int statusCode = ThingSpeak.getLastReadStatus();

    if (statusCode != 200) 
    {
      Serial.println("Error reading value from ThingsSpeak");
    }

    if (read_voltage < 0) read_voltage=0; // stop the motor when user leaves

    if (read_voltage != old_voltage)  // new value updated
    {
      pulses = 0;
      time_old = millis();
      old_voltage = read_voltage;

      Serial.println("Input Voltage: " + String(read_voltage));
      flag = true;
    }
    
    // DC motor always running

    speed = (read_voltage*255)/9; // pwm modulation_line to controal voltage.
    analogWrite(EN1, speed);
    digitalWrite(inputPin1, HIGH);
    digitalWrite(inputPin2, LOW);  

    if(flag == true and millis()-time_old >= 1000 and read_voltage!=0) // update the rpm value after 1 second.
    {
        detachInterrupt(digitalPinToInterrupt(encoder_pin));

        rpm = (60 * 1000 / pulses_per_turn )/ (millis() - time_old)* pulses; 
        Serial.println("Output RPM = "+ String(rpm));

        int statuscode1 = ThingSpeak.writeField(channel_num, 2, rpm, myWriteAPIKey);

        /* uploading data in om2m server */
        
        CreateCI(read_voltage, rpm);

        /* uploading ends */
        
        if (statuscode1 == 200)
        {
            Serial.println("RPM Value Updated Successfully");
        }  
        
        flag = false;

        // Triggering count function everytime the encoder pin1 turns from 1 to 0 
        attachInterrupt(digitalPinToInterrupt(encoder_pin), count, FALLING ); 
    }
}
