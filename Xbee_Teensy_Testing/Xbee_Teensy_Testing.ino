//XBEE setup
#include <XBee.h>

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

//XBEE Packet Setup
byte packet[] = {0x7E, 0x00, 0x25, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x7C, 0x33, 0x35, 0x30, 0x38, 0x34, 0x34, 0x30, 0x39, 0x36, 0x7C, 0x2D, 0x31, 0x30, 0x36, 0x36, 0x34, 0x36, 0x33, 0x32, 0x39, 0x36, 0x7C, 0x6E};
// create the XBee object
XBee xbee = XBee();

uint32_t timer = millis();

const int LED = 4;
const int BUTTON = 3;
int buttonStatus;
bool sButtonStatus;

//Applying setting to OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire); //creating object

void setup() {

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  sButtonStatus = false;
  buttonStatus = digitalRead(BUTTON);

  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  Serial.begin(115200);
  //  while (!Serial);

  Serial1.begin(9600); //for XBEE
  xbee.setSerial(Serial1);

  Serial.printf("buttonState is %i\n Button status is %i\n", sButtonStatus, buttonStatus);

  attachInterrupt(BUTTON, sButton, RISING);

}

void loop() {

  sendAttempt();
}


void sendAttempt() {
  if (sButtonStatus == true) {
    //    Serial.printf("buttonState is %i\n", sButtonStatus);
    digitalWrite(LED, HIGH);
    delay(250);
    sButton();
    zbtSend();
    //        Serial1.write(packet, sizeof(packet));
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


//API stuff

void zbtSend() {
  int i;
  uint8_t sum, tempChecksum, checksum;
  const int PAY_LEN = 25;
  char tempGPS[] = {"|" "3" "5" "0" "8" "4" "4" "0" "9" "6" "|" "-" "1" "0" "6" "6" "4" "6" "3" "2" "9" "6" "|" };
  //  Serial.printf("fixed lat is %i\n fixed long is %i\n", GPS.latitude_fixed, GPS.longitude_fixed);

  //  Serial.println("Payload,");
  //  for (i = 0; i < PAY_LEN; i++) {
  //    Serial.printf("0x%02X,", payload[i]);
  //  }
  Serial.printf("\n");

  const int BUF_LEN = 50;
  byte txBuf[BUF_LEN]; // manual TX packet creation - 6-13 are 64-bit address
  txBuf[0] = 0x7E;    //start delimiter
  txBuf[1] = 0x00;    //MSB
  txBuf[2] = sizeof(txBuf) - 4; // + 14;  //LSB - Packet created with no additional data show 0E so needs size of () + 14
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

  for (i = 17; i < BUF_LEN; i++) {
    txBuf[i] = tempGPS[i - 17];
  }
  Serial.println("BUF LEN,");
  for (i = 0; i < BUF_LEN; i++) {
    // Serial.printf("0x%02X,", txBuf[i]);
    Serial.printf("%02X ", txBuf[i]);
  }
  Serial.printf("\n");

  sum = 0x00;
  for (i = 3; i < BUF_LEN - 1; i++) {
    //tempChecksum = txBuf[i] + tempChecksum;
    sum = sum + txBuf[i];
    Serial.printf("0x%02X\n", sum);

  }
  checksum = 0xFF - sum;
  txBuf[BUF_LEN - 1] = checksum;
  Serial.printf("checksum = 0x%02X\n", checksum);

  for (i = 0; i < BUF_LEN; i++) {
    Serial.printf(" 0x%02X,", txBuf[i]);
  }
  Serial.printf("\n");
  Serial1.write(txBuf, sizeof(txBuf));


  // fixed lat is 350844096
  // fixed long is -1066463296
  //buf is  350844096|-1066463296

}
