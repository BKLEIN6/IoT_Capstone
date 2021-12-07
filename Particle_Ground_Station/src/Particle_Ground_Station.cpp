/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/benja/Documents/IoT/Capstone/Particle_Ground_Station/src/Particle_Ground_Station.ino"
/*
 * Project Particle_Ground_Station
 * Description: Gound Station for receiving GPS data and posting to a Dashboard
 * Author:Ben Klein
 * Date:05-Dec-2021
 */

//Adafruit dashboard setup
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT.h" 
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h" 
#include "credentials.h"
#include "JsonParserGeneratorRK.h" //this must be above OLED setup -- errors abound if not

//OLED
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

void setup();
void loop();
void findPipe();
void MQTT_connect();
void MQTT_ping();
#line 19 "c:/Users/benja/Documents/IoT/Capstone/Particle_Ground_Station/src/Particle_Ground_Station.ino"
#define OLED_RESET D4
Adafruit_SSD1306 display(OLED_RESET);

const int LED = D3;
int i, timerDelay, last; 
bool switchState;

const int BUF_LEN = 50;
byte tempData[BUF_LEN];

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);

/****************************** Feeds ***************************************/ 
// Setup Feeds to publish or subscribe 
// MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Publish gpsObj1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/GPS");

SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(LED, OUTPUT);
  timerDelay = millis();
  last = millis();

//OLED setup
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
display.display(); // show splashscreen
delay(2000);
display.clearDisplay();// clears splashscreen 
display.display();


   //Connect to WiFi without going to Particle Cloud
  // WiFi.connect();
  // while(WiFi.connecting()) {
  //   Serial.printf(".");
  // }
}

void loop() {
  // MQTT_connect(); 
  // MQTT_ping();
  if(Serial1.available()>0){
    digitalWrite(LED, HIGH);
    tempData[i] = Serial1.read();
    // Serial.printf(" %i 0x%X, ", i, tempData[i]);
    i++;
    timerDelay=millis();
  }
  if(Serial1.available() == 0 && millis()-timerDelay >= 500){
    i=0;
    digitalWrite(LED, LOW);
    findPipe();
  }
}

void findPipe(){  
  const char P_LEN = 5;
  char pipe[P_LEN], lat[15], lon[15];
  uint8_t i,p, latSize, lonSize;
  float actualLat, actualLon;
  p=0;
  // Serial.printf("Chekcing for data");
  if (tempData[0] != 0x7E){
    // Serial.printf("No Data, leaving function\n");
    return;
  }
  // Serial.printf("Start finding pipes");
  for (i=0; i<BUF_LEN; i++){
   if (tempData[i] == 0x7C){
      pipe[p] = i;
      // Serial.printf("| at tempData[%i], pipe[%i] = %i\n", i, p, i);
      p++;
      }
    }
  latSize = pipe[1]-pipe[0]-1;
  lonSize = pipe[2]-pipe[1]-1;
  // Serial.printf("latSize = %i lonSize = %i\n", latSize, lonSize);

  memcpy(lat, &tempData[pipe[0]+1], latSize);
  memcpy(lon, &tempData[pipe[1]+1], lonSize);

  actualLat = atoi((char *)lat)/10000000.0;
  actualLon = atoi((char *)lon)/10000000.0;
  // Serial.printf("Actual Lat = %f\n Actual Lon = %f\n", actualLat, actualLon);
  // Serial.printf("End pipe find\n");

  //created json buffer to send to Adafruit IO dashboard.
  JsonWriterStatic<256> jw;{
    JsonWriterAutoObject obj(&jw);
    jw.insertKeyValue("lat", actualLat);
    jw.insertKeyValue("lon", actualLon);
  }
  
  gpsObj1.publish(jw.getBuffer());

  //OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.printf("Received Coordinates: \n Lat: %f\n Lon: %f\n ", actualLat, actualLon);
  display.display();

  tempData[0] = 0;
}

// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;
 
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("%s\n",(char *)mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds..\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!\n");
}

void MQTT_ping(){
  // Ping MQTT Broker every 2 minutes to keep connection alive
  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      if(! mqtt.ping()) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
}