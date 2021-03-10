#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Lora radio config
const int loraSF = 10;       // Lora SF
const int loraCsPin = 18;    // LoRa radio chip select
const int loraResetPin = 23; // LoRa radio reset
const int loraIrqPin = 26;   // change for your board; must be a hardware interrupt pin

void onTxDone()
{
  Serial.println("TxDone");
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa Sender non-blocking Callback");

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

  LoRa.setSpreadingFactor(10); // ranges from 6-12,default 7 see API docs

  LoRa.onTxDone(onTxDone);
}

int p1 = 0;

void loop()
{
  if (runEvery(1000))
  {
    int new_p1 = digitalRead(2);

    if (new_p1 != p1)
    {
      p1 = new_p1;

      Serial.print("Sending packet non-blocking: ");
      Serial.println(p1);

      // send in async / non-blocking mode
      LoRa.beginPacket();
      LoRa.print(p1);
      LoRa.endPacket(true); // true = async / non-blocking mode
    }
  }
}
