/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h> 
#include <SoftwareSerial.h> 
#include <stdlib.h>


//define the pins used by the transceiver module
#define ss 6
#define rst 5
#define dio0 2


SoftwareSerial gpsSerial(8,9);//rx,tx 
TinyGPSPlus tinyGPS; // create gps object 

#define gpsPort gpsSerial  // Alternatively, use Serial1 on the Leonardo
#define SerialMonitor Serial


void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  gpsPort.begin(9600); // connect gps sensor 
  Serial.println("GPS Initialized");
  Serial.println("Waiting for GPS.. .");
  LoRa.beginPacket();
  LoRa.print("Waiting for GPS...");
  LoRa.endPacket();
  Serial.println("___________________________");


}

void loop() {

    printGPSInfo();
    smartDelay(10000); 

}

void printGPSInfo() {

    LoRa.beginPacket();
    LoRa.print("lat: ");
    LoRa.print(tinyGPS.location.lat(),6);
    LoRa.print(",lng: ");
    LoRa.println(tinyGPS.location.lng(),6);
    LoRa.endPacket();

    
    Serial.println(tinyGPS.location.lat(),6);
    Serial.println(tinyGPS.location.lng(),6);
    Serial.println(tinyGPS.altitude.meters()); 
    Serial.println("___________________________");
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}
