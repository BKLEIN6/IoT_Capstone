/*
   Projuect: Capstone GPS Transmitter
   Description: Sends GPS data to a Gound Station over XBEE wireless network
   Author: Ben Klein
   Date: 01-Dec-2021
*/

// OLED Setup
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Ultimate GPS Setup
#include <Adafruit_GPS.h>

// Name of the hardware serial port
#define GPSSerial Serial2

//OLED Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

const int LED = 4;
const int BUTTON = 3;
int timer, buttonStatus;
bool sButtonStatus;

//Applying setting to OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); //creating object

void setup() {

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  sButtonStatus = false;
  buttonStatus = digitalRead(BUTTON);
  timer = millis();

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  Serial.begin(115200);
  //  while (!Serial);

  Serial1.begin(9600); //for XBEE

  attachInterrupt(BUTTON, sButton, RISING);

  // GPS SETUP
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600); //Serial2
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  //Starting OLED
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  gpsLoop();
  sendAttempt();
}

void gpsLoop() {
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    OLED();
  }
}

void sendAttempt() {
  if (sButtonStatus == true) {
    digitalWrite(LED, HIGH);
    delay(250);
    sButton();
    SOLED();
    packetSend();
  }
  else {
    //    Serial.printf("buttonState is %i\n", sButtonStatus);
    digitalWrite(LED, LOW);
  }
}

void sButton() {
  sButtonStatus = !sButtonStatus;
  Serial.printf("buttonState is %i\n", sButtonStatus);
}

//API frame creator and sender
void packetSend() {
  int i;
  uint8_t sum, tempChecksum, checksum;
  const int PAY_LEN = 30;
  byte payload[PAY_LEN];
  const int BUF_LEN = 40;
  byte txBuf[BUF_LEN]; // manual TX packet creation - 6-13 are 64-bit address

  sprintf((char *)payload, "|%i|%i|", GPS.latitude_fixed, GPS.longitude_fixed);

  txBuf[0] = 0x7E;    //start delimiter
  txBuf[1] = 0x00;    //MSB
  txBuf[2] = sizeof(txBuf) - 4; // LSB needed-4 to be added to make sure checksum values added up correctly
  txBuf[3] = 0x10; // API Frame type
  txBuf[4] = 0x01; //Frame ID
  txBuf[5] = 0x00; // start of 64-bit address
  txBuf[6] = 0x13;
  txBuf[7] = 0xA2;
  txBuf[8] = 0x00;
  txBuf[9] = 0x41;
  txBuf[10] = 0xCF;
  txBuf[11] = 0x10;
  txBuf[12] = 0x39;
  txBuf[13] = 0xFF; //MSB 16 bit address
  txBuf[14] = 0xFE; //LSB 16 bit address
  txBuf[15] = 0x00; //Broadcast Radius
  txBuf[16] = 0x00; //Options

  // This section puts the GPS data into the packet.
  for (i = 17; i < BUF_LEN; i++) {
    txBuf[i] = payload[i - 17];
  }

  // This section ceates the base for the checksum needed at the end of the packet
  sum = 0x00;
  for (i = 3; i < BUF_LEN - 1; i++) {
    sum = sum + txBuf[i];
  }

  // this created the checksum and puts it at the end of the frame
  checksum = 0xFF - sum;
  txBuf[BUF_LEN - 1] = checksum;
  Serial1.write(txBuf, sizeof(txBuf));
}

void OLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);//inverse text
  display.setCursor(0, 10);
  display.setRotation(0);
  display.printf("Your location is: \n Lat: %f\n Lon: %f\n ", GPS.latitudeDegrees, GPS.longitudeDegrees);
  display.display();
}

void SOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);//inverse text
  display.setCursor(0, 10);
  display.setRotation(0);
  display.printf("Your location has\n been sent!\n Lat: %f\n Lon: %f\n ", GPS.latitudeDegrees, GPS.longitudeDegrees);
  display.display();
}
