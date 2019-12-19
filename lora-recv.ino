
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <time.h>


//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
#define batpin 34
#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
#define SLEEP_DAY  60      // Time ESP32 will go to sleep (in seconds)
#define SLEEP_NIGHT 600 // 10 Minutes
#define SLEEP_NAP 15
#define PACKET_SCAN_TIME 30000 // in milliseconds
RTC_DATA_ATTR int retryloop;

String LoRaData = "";

const int nightModeStart = 1; // hour [ 1 = 1 am ]
const int nightModeEnd = 4;
const int resetStart = 0;
const char* ssid="<your router ssid>";
const char* password = "<your router password>";
const uint16_t port = 8090; //This is the port you are going to send data to
const char* host="<IP of machine running the python script loramon.py";
const int TOTAL_RETRIES = 12;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

WiFiClient client;


void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
  pinMode(batpin, INPUT);
  while (!Serial);
  Serial.println("Starting LoRa Receiver");
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //915E6 for North America
  Serial.print("Initializing LoRa .");
  while (!LoRa.begin(915E6)) {
    Serial.print(".");
    delay(500);
  }
  
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF

  //LoRa Setting Here
  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  //LoRa.setCodingRate4(8);
  LoRa.setSyncWord(0xE3);
  //LoRa.enableCrc();
 
  Serial.println(" [OK!]");
  
  connectWifi();

  // Get Battery voltage 
     float r1=26820.0;
     float r2=100000.0;
     float VBAT = ((((3.3f*analogRead(batpin))/4095)/ r2*(r1+r2)) );
     String batstatus = "Battery Voltage = " + String(VBAT, 2) + "V";
     Serial.println(batstatus);
     deliverMessage(batstatus); 
   
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
    
    long int startMillis = millis();
    long int currentMillis = startMillis;
    bool packetDetected = false;
    if (retryloop > 0) {
      String msg = "Retry Attempt : " + String(retryloop) + "/" + String(TOTAL_RETRIES);
      deliverMessage(msg);
    }



    Serial.println("Scanning for LoRa Packets ...");
    deliverMessage("Scanning for LoRa Packets...");
    while(currentMillis <= startMillis+PACKET_SCAN_TIME) {
      if (getPacket()) {
        // Packet Received!
        deliverMessage(LoRaData);
        packetDetected = true;
        // fastblink();
        retryloop = 0;
      }
      currentMillis = millis();
    }

    // If packet was detected keep checking for more

    if(packetDetected) {
      startMillis = millis();
      currentMillis = startMillis;
      retryloop = 0;
      while (currentMillis <= startMillis + PACKET_SCAN_TIME) {
        if(getPacket()) {
          deliverMessage(LoRaData);
          startMillis = millis();
        }
        currentMillis = millis();
      }
    } else {
      if (retryloop == 0) {

        if (nightMode()) {
          goToSleep(SLEEP_NIGHT);

        } else {
          if (VBAT < 4.18) {
            goToSleep(SLEEP_DAY);
          } else {
            goToSleep(SLEEP_NAP);
          }
        }
        esp_deep_sleep_start();
      }
    }
    Serial.println("");

    // Stopped receiving packets....

    if(retryloop == 0) {
      Serial.println("Stopped Receiving Packets.....");
      deliverMessage("Stopped Receiving Packets...");
    }
    if (retryloop == TOTAL_RETRIES) {
        // Retries Failed, [Sleep 5 min]
        retryloop = 0;
        deliverMessage("Giving Up...");
        if (VBAT < 4.18) {
          goToSleep(SLEEP_DAY);
        } else {
          goToSleep(SLEEP_NAP);
        }
      
      } else {
        retryloop++;
        delay(1000);
        goToSleep(SLEEP_NAP);
      }      
}

void loop() {
  // UNUSED
}

bool getPacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");
    // read packet
    while (LoRa.available()) {
        LoRaData = LoRa.readString();
        Serial.println(LoRaData); 
        int rssi = LoRa.packetRssi();
        Serial.print("RSSI : ");
        Serial.println(rssi);
        String msg = "RSSI : " + String(rssi);
        deliverMessage(msg);

    }
    return true;
  }
  return false;
}

void deliverMessage(String message) {
    // Send Packet to Socket
    if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
    }

    if (client.connect(host,port)) {
        // Do nothing
    } else {
      Serial.println("Connection to Socket Client Failed");
      client.flush();
      client.stop();
      return;
    }
    if (client.connected()) {
        client.print(message);
        delay(500);
    } else {
        Serial.println("Host not available");
    }
    client.stop();
    delay(500);
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void doReset() {
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;
  
  if ((hours = resetStart) && (minutes < 2)) {
    ESP.restart();
  }
}

boolean nightMode() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }

  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;

  if ((hours >= nightModeStart) && (hours <= nightModeEnd)) {
    if ((hours == nightModeEnd) && (minutes > 40)) {
      return false;
    } else {

      return true;
    }
  } else {
    return false;
  }
    

}
String sleepString(int sleepsec){
  String sleeptime=String(sleepsec)+" Sec";
  String sec = "";
  if (sleepsec > 60) {
    int minutes = sleepsec/60;
    int seconds = 0;
    seconds = sleepsec % 60;
    if (seconds < 10) {
      sec = String('0' + String(seconds));
    } else {
      sec = String(seconds);
    }
      sleeptime = String(minutes)+":"+sec+" Min";
  }
  String msg = "Sleeping [" + sleeptime + "]";
  return msg;
 
}

void goToSleep(int sleeptime){

    deliverMessage(sleepString(sleeptime));
    esp_sleep_enable_timer_wakeup(sleeptime  * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void connectWifi() {

   // INITIALIZE WIFI

    int timerstart = millis();
    

    Serial.print("Initializing WiFi");
   
    WiFi.begin(ssid,password);
    Serial.print("Connecting.");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() > timerstart + 30000) {
        esp_restart();
      }
    }
    Serial.println("[OK!]");
    delay(500);
    Serial.println(WiFi.localIP());  
}
