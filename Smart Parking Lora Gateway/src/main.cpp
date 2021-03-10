#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "A#"
#define WIFI_PASSWORD "1qaz2wsx"
#define FIREBASE_HOST "myproject-d1dfe-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "QwDv3H18kq70pOPmn2wwhlgsbxegjpzu3FnfufbC"

// Lora radio config for TTGO T-Beam v1.1 board. Different for other boards
const int loraCsPin = 18;    // LoRa radio chip select
const int loraResetPin = 23; // LoRa radio reset
const int loraIrqPin = 26;   // change for your board; must be a hardware interrupt pin
const int loraSF = 10;       // Lora Spreading factor

boolean gotLoraPacket = false;
String loraMsg = "";
String loraRSSI = "";

// Define FirebaseESP8266 data object for data sending and receiving
FirebaseData fbdo;

void onLoraReceive(int packetSize)
{
  if (packetSize == 0)
    return;

  // read packet header bytes:
  String incoming = "";

  while (LoRa.available())
    incoming += (char)LoRa.read();

  gotLoraPacket = true;
  loraMsg = incoming;
  loraRSSI = String(LoRa.packetRssi());
}

void sendLoraMessage(String outgoing)
{
  LoRa.beginPacket();   // start packet
  LoRa.print(outgoing); // add payload
  LoRa.endPacket();     // finish packet and send it
}

void initLora()
{
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(loraCsPin, loraResetPin, loraIrqPin); // set CS, reset, IRQ pin

  // initialize ratio at 868 MHz
  if (!LoRa.begin(868E6))
  {
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }

  LoRa.setSpreadingFactor(loraSF); // ranges from 6-12,default 7 see API docs
  Serial.println("LoRa init succeeded.");

  LoRa.onReceive(onLoraReceive);
}

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Set Firebase info
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Enable auto reconnect the WiFi when connection lost
  Firebase.reconnectWiFi(true);

  initLora();

  LoRa.receive();
}

void loop()
{
  if (gotLoraPacket)
  {
    Serial.print("Received Lora message: ");
    Serial.println(loraMsg);

    if (Firebase.setBool(fbdo, "/p1/isOccupied", loraMsg != "1"))
    {
      //Success
      Serial.print("Set p1 ");
      Serial.println(loraMsg != "1");
    }
    else
    {
      //Failed?, get the error reason from fbdo
      Serial.print("Error in setBool, ");
      Serial.println(fbdo.errorReason());
    }

    Serial.println("Firebase Done");
    gotLoraPacket = false;
  }
}