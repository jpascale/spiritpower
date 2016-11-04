#include "MFRC522.h"
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define RST_PIN 15 // RST-PIN for RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

#define LIMIT_TASK_TIME 15000 //15 seconds

#define RING_PIN 5

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, RING_PIN, NEO_GRB + NEO_KHZ800);

//Wifi
const char* ssid = "Arashenbek";
const char* password = "kolobok123";
const int serverport = 5005;
IPAddress serverip(192, 168, 0, 107);

//WiFi config
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets

unsigned long dihmtime;

//Colors
uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t magenta = strip.Color(255, 0, 255);

//Game logic
#define NUM_TAGS 3

//TFID Id's
int redId[] = {4, 196, 108, 114};
int greenId[] = {4, 7, 108, 114};
int blueId[] = {4, 229, 108, 114};
//int redId[] = {4, 195, 108, 114};
//int greenId[] = {4, 163, 108, 114};
//int blueId[] = {4, 228, 108, 114};

int * taskRfid[NUM_TAGS]; //red green blue

//Flex sensor
//TODO: Adjust values
const int FLEX_PIN = A0; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 100.0;//37300.0; // resistance when straight
const float BEND_RESISTANCE = 10000.0; // resistance at 90 deg

#define FLEX_BEND_LIMIT 60
boolean task = false;
int tasknum = 0;
int points = 0;

unsigned long initial_millis;
boolean charged;
boolean bent;
int charge_counter;


void setup() {

  wifiBegin();
  join();
  
  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //GL
  taskRfid[0] = redId;
  taskRfid[1] = greenId;
  taskRfid[2] = blueId;

  //Flex pin initialization
  pinMode(FLEX_PIN, INPUT);

  randomSeed(analogRead(0));

  charged = true;
  bent = false;
  charge_counter = 0;
  initial_millis = millis();
  dihmtime = millis();
}

void loop() { 

  /*if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
  }*/
  if (millis() - dihmtime > 2000) {
    dihm();
    dihmtime = millis();
  }
  ///////// Setting a new task
  if (!task){
    task = true;
    tasknum = random(NUM_TAGS);
    
    switch (tasknum) {
      case 0:
        Serial.println("Touch red!");
        break;
        
      case 1:
        Serial.println("Touch green!");
        debug();
        break;

      case 2:
        Serial.println("Touch blue!");
        break;
    }
    updateTaskPointIndicator();
    //Reset time
    
    charged = true;
    initial_millis = millis();
    delay(1000);
  }

  ////////////////// Reading from the RFID
  if (charged && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (task) {
      //Serial.println("ASDASDADSASDASDADS");
      /*if(str[0] == 16 && tasknum == 0){
         points += 1;
         task = false;
         Serial.print("Touched white, points are ");
         Serial.println(points);
      }
      if(str[0] == 203 && tasknum == 1){
         points += 1;
         task = false;
         Serial.print("Touched blue, points are ");
         Serial.println(points);
      }*/
      /*Serial.println(str[0]);
      Serial.println(taskRfid[tasknum][0]);
      Serial.println(str[1]);
      Serial.println(taskRfid[tasknum][1]);
      Serial.println(str[2]);
      Serial.println(taskRfid[tasknum][2]);
      Serial.println(str[3]);
      Serial.println(taskRfid[tasknum][3]);*/
      //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      byte * buffer = mfrc522.uid.uidByte;
      if (buffer[0] == taskRfid[tasknum][0] && buffer[1] == taskRfid[tasknum][1] && buffer[2] == taskRfid[tasknum][2] && buffer[3] == taskRfid[tasknum][3]) {
        points += 1;
        task = false;
        Serial.print("Touched ");
        //Serial.print(getTaskName());
        Serial.print(", points are ");
        Serial.println(points);
        //updateTaskPointIndicator();
        //delay(2000);
        point();
      }
      
      //updateTaskPointIndicator();
      if (points >= 10){
        updateTaskPointIndicator();
        points = 0;
        win();
        while(1);
        Serial.println("You won! Starting new game.");
        delay(5000);
        RingTurnOff();
        updateTaskPointIndicator();
      }
    }
    delay(1000);
  }

  if (charged){
    checkTime();  
  }

  if (!charged){
    chargeLogic();
  }
  
}

// Helper routine to dump a byte array as hex values to Serial
/*void dump_byte_array(byte *buffer, byte bufferSize) {
  //for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //Serial.print(buffer[i], HEX);
    //Serial.print(buffer[i]);
  //}
  Serial.print(buffer[0]);
  Serial.print(" ");
  Serial.print(buffer[1]);
  Serial.print(" ");
  Serial.print(buffer[2]);
  Serial.print(" ");
  Serial.println(buffer[3]);

  if (buffer[0] == taskRfid[tasknum][0] && buffer[1] == taskRfid[tasknum][1] && buffer[2] == taskRfid[tasknum][2] && buffer[3] == taskRfid[tasknum][3]) {
        points += 1;
        task = false;
        Serial.print("Touched ");
        //Serial.print(getTaskName());
        Serial.print(", points are ");
        Serial.println(points);
        //updateTaskPointIndicator();
        //delay(2000);
      }
      return;
}*/

void RingTurnOff() {
  int i;
  for (i = 0; i < 16; i++) {
     strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}

void LEDUpperBoundSetUp(int upperBound, uint32_t color){
  int i;
  for (i = 0; i <= upperBound; i++) {
     strip.setPixelColor(i, color);
  }
  strip.show();
}

void updateTaskPointIndicator(){
  uint32_t color;

  switch (tasknum) {
    case 0:
      color = red;
      Serial.println("RED");
      break;
    case 1:
      color = green;
      Serial.println("GREEN");
      break;
    case 2:
      color = blue;
      Serial.println("BLUE");
      break;
  }
  int ledUpperBound = 1 + points;

  LEDUpperBoundSetUp(ledUpperBound, color);
}

uint32_t getTaskColor(){
  switch (task) {
    case 0:
      return red;
      break;
    case 1:
      return green;
      break;
    case 2:
      return blue;
      break;
  }
}
char * getTaskName(){
  switch (task) {
    case 0:
      return "red"; 
    case 1:
      return "green";
    case 2:
      return "blue";
  }
}
void debug() {
  Serial.print("tasknum = ");
  Serial.print(tasknum);  
}

//Time functions
void checkTime() {
  if ((millis() - initial_millis) > LIMIT_TASK_TIME){
    Serial.println("Time's up");
    charged = false;
    RingTurnOff();
  } else {
    //Serial.print("There's ");
    //Serial.print((15000.0 - (millis() - initial_millis))/1000);
    //Serial.println(" secs left.");
  }
}

void chargeLogic() {
  int flexADC = analogRead(FLEX_PIN);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  //Serial.println("Resistance: " + String(flexR) + " ohms");

  // Use the calculated resistance to estimate the sensor's
  // bend angle:
  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE,
                   0, 90.0);
  Serial.println("Bend: " + String(angle) + " degrees");
  //Serial.println();

  if (!bent && angle > FLEX_BEND_LIMIT) {
    bent = true;
  }
  if (bent && angle < FLEX_BEND_LIMIT) {
    bent = false;
    charge_counter += 1;
    updateChargeLed(charge_counter);
    if (charge_counter >= 3){
      task = false;
      charged = true;
      charge_counter = 0;
      RingTurnOff();
    }
  }
  
  delay(100);
}

void updateChargeLed(int bents){
  Serial.print("Debug: Update charge leds, bents: ");
  Serial.println(bents);

  LEDUpperBoundSetUp(bents * 4 - 1, magenta);
}

void lose() {
  strip.setPixelColor(0, blue);
  strip.setPixelColor(2, blue);
  strip.setPixelColor(4, blue);
  strip.setPixelColor(6, blue);
  strip.setPixelColor(8, blue);
  strip.setPixelColor(10, blue);
  strip.setPixelColor(12, blue);
  strip.setPixelColor(1, red);
  strip.setPixelColor(3, red);
  strip.setPixelColor(5, red);
  strip.setPixelColor(7, red);
  strip.setPixelColor(9, red);
  strip.setPixelColor(11, red);
  strip.show();
}

void win() {
  strip.setPixelColor(0, green);
  strip.setPixelColor(2, green);
  strip.setPixelColor(4, green);
  strip.setPixelColor(6, green);
  strip.setPixelColor(8, green);
  strip.setPixelColor(10, green);
  strip.setPixelColor(12, green);
  strip.setPixelColor(1, yellow);
  strip.setPixelColor(3, yellow);
  strip.setPixelColor(5, yellow);
  strip.setPixelColor(7, yellow);
  strip.setPixelColor(9, yellow);
  strip.setPixelColor(11, yellow);
  strip.show();
}

/////////////////////////////////
// WIFI
/////////////////////////////////

void wifiBegin() {
  Serial.begin(115200);
  Serial.println();

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void join(){
  Serial.println("Sending JOIN request");
  Udp.beginPacket(serverip, serverport);
  Udp.write("JOIN");
  Udp.endPacket();

 int packetSize = Udp.parsePacket();
  //Wait
  Serial.println("Waiting for reply");
  while(!packetSize){
    Serial.println("Still waiting join");
    delay(250);
    packetSize = Udp.parsePacket();
  }

  int len = Udp.read(incomingPacket, 255);
  if (len > 0)
  {
    incomingPacket[len] = 0;
  }

  Serial.printf("%s\n", incomingPacket);

  int packetSize2 = Udp.parsePacket();

  while(!packetSize2){
    Serial.println("Still waiting join2");
    delay(250);
    packetSize2 = Udp.parsePacket();
  }

  len = Udp.read(incomingPacket, 255);
  if (len > 0)
  {
    incomingPacket[len] = 0;
  }

  Serial.printf("%s\n", incomingPacket);
    
}

void point(){
  Serial.println("Sending POINT request");
  Udp.beginPacket(serverip, serverport);
  Udp.write("POINT");
  Udp.endPacket();

  int packetSize = Udp.parsePacket();
  //Wait
  Serial.println("Waiting for reply");
  while(!packetSize){
    Serial.println("Still waiting point");
    delay(250);
    packetSize = Udp.parsePacket();
  }

  int len = Udp.read(incomingPacket, 255);
  if (len > 0)
  {
    incomingPacket[len] = 0;
  }

  Serial.printf("%s\n", incomingPacket);
}

void dihm(){
  Serial.println("Sending DIHM request");
  Udp.beginPacket(serverip, serverport);
  Udp.write("DIHM");
  Udp.endPacket();

  int packetSize = Udp.parsePacket();
  //Wait
  //Serial.println("Waiting for reply");
  //delay(10);
  //while(!packetSize){
  //  Serial.println("Still waiting dihm");
  //  delay(250);
  //  packetSize = Udp.parsePacket();
  //}


  int len = Udp.read(incomingPacket, 255);
  if (len > 0)
  {
    incomingPacket[len] = 0;
  }

  Serial.printf("%s\n", incomingPacket);

  if (incomingPacket[0] == 'F'){
    Serial.println("I LOST");
    lose();
    while(true);
  }
  
}

