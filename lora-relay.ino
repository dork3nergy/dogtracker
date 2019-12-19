/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h> 
#include <stdlib.h>
#include "LowPower.h"



//define the pins used by the transceiver module
#define ss 6
#define rst 5
#define dio0 2


#define SerialMonitor Serial

const int WaitForPacket = 8000; // How long to scan for packets when away in ms
String LoRaData = "";


void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Relay");

  //setup LoRa transceiver module
  
  
  LoRa.setPins(ss, rst, dio0);

  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  
  LoRa.setSyncWord(0xF3);
  
  // Other Settings for LoRa
  
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(11);
  LoRa.setSignalBandwidth(62.5E3);
  //LoRa.setCodingRate4(5);
  //LoRa.enableCrc();



  Serial.println("LoRa Initializing OK!");
  relayData("Relay Online...");
  

}

void loop() {

 
    long int startMillis = millis();
    long int currentMillis = startMillis;
    bool packetDetected = false;
    Serial.println("Scanning for Packets ...");
    while(currentMillis <= startMillis+WaitForPacket) {

      if (getPacket()) {
        Serial.println(LoRaData); 
        relayData(LoRaData);
        startMillis = millis();
      }
      currentMillis = millis();
    }
    
    Serial.println("Sleeping");
    delay(200);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

}

void relayData(String message) {
  delay(3000);
  Serial.print("Relaying : ");
  Serial.println(message);
  String tempmsg = "*"+message;
  LoRa.beginPacket();
  LoRa.print(tempmsg);
  LoRa.endPacket();
}


bool getPacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("[Relay] Caught packet ");
    // read packet
    while (LoRa.available()) {
        LoRaData = LoRa.readString();
    }
    return true;
  }
  return false;
}
