#include "MFRC522.h"
#define RST_PIN 15 // RST-PIN for RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void setup() {
  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
}

void loop() { 
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    Serial.println("A");
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    Serial.println("B");
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
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
  Serial.print(buffer[3]);
  Serial.print(" ");
  Serial.print(buffer[4]);
  Serial.print(" ");
  Serial.print(buffer[5]);
  Serial.print(" ");
  Serial.print(buffer[6]);
}
