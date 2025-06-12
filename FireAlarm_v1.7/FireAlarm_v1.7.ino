#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

byte mac[] = { 0x8C, 0x4C, 0xAD, 0xF0, 0xBE, 0x86 };

EthernetClient client;

int HTTP_PORT = 5006;
String HTTP_METHOD = "GET";
// const char* HOST_NAME = "evoluzn.org";
const char* HOST_NAME = "192.168.29.199";
String PATH_NAME = "/alert_detected/29";

const byte interruptPin = 2;
unsigned long nowTime = 0;
long interval = 1000;
unsigned long PreviousTime = 0;
static int count = 0;
bool AlertFlag = true;
bool counterFlag = false;
String jsonData;
String devID = "AQRL_FA_03";
byte Category = 1;
byte subCategory = 1;
const size_t maxJsonSize = 100;
unsigned int delayCounter = 0;


bool EthernetSetup() {
  Ethernet.init(10);
  Ethernet.begin(mac);
  if (Ethernet.hardwareStatus()) {
    if (Ethernet.linkStatus() == LinkON) {
      Serial.println("ethernet is connected");

      return true;
    } else {
      Serial.println("link failed");
    }
  } else {
    Serial.println("hardware fault");
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
  Serial.println("Starting ETHERNET connection...");
  
  EthernetSetup();
}

void loop() {
  if (counterFlag == true && AlertFlag == true) {
    delayCounter += 1;
    Serial.print("delayCounter: ");
    Serial.println(delayCounter);
    if (delayCounter >= 50) {
      Serial.println("Reset delay Counter ");
      delayCounter = 0;
      count = 0;
      counterFlag = false;
    }
  }

  if (count >= 10) {
    AlertFlag = false;
    jsonData = convertInJson(devID, Category, subCategory);
    if (validateJson(jsonData)) {
      count = 0;
      counterFlag = false;
      notify();
      delay(10000);
      AlertFlag = true;
    }
  }
}

void blink() {
  if (AlertFlag) {
    count++;
    Serial.print("count: ");
    Serial.println(count);
    counterFlag = true;
    delayCounter = 0;
    delay(10);
  }
}

void notify() {
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    Serial.println("Connected to server");
    client.println(HTTP_METHOD + " " + PATH_NAME + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.print("Content-Type: application/json\r\n");
    client.println("Connection: close");
    client.print(jsonData);
    client.println();

    unsigned long timeout = millis() + 2000;
    // Wait for the response and print it to the Serial monitor
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {
    Serial.println("connection failed");
  }
}

String convertInJson(String devID, byte Category, byte subCategory) {
  DynamicJsonDocument sensorData(maxJsonSize);
  sensorData["device_id"] = devID;
  sensorData["Catergory"] = 1;
  sensorData["subCatergory"] = 1;

  // Serialize the JSON object to a string
  String jsonData;
  serializeJson(sensorData, jsonData);

  Serial.print("Generated JSON data: ");
  Serial.println(jsonData);
  return jsonData;
}

bool validateJson(String jsonData) {
  DynamicJsonDocument jsonDocument(maxJsonSize);
  DeserializationError error = deserializeJson(jsonDocument, jsonData);

  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    return false;
  } else {
    Serial.println("JSON data is valid.");
    return true;
  }
}
