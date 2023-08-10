#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>


#define DHTPIN 2
#define DHTTYPE DHT11

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

DHT dht(DHTPIN, DHTTYPE);
static char celsiusTemp[7];
static char fahrenheitTemp[7];

const char* serverAddress = "192.168.1.7";
const int serverPort = 3000;
const char* serverUrl = "http://192.168.1.7:3000/api/control";
const int ledpin = D6;

WiFiManager wifiManager;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const long utcOffsetInSeconds = 7 * 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  pinMode(ledpin, OUTPUT);
  Serial.begin(9600);

  wifiManager.autoConnect("AutoConnectAP");

  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);


  timeClient.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis1 >= 60000) {
    previousMillis1 = currentMillis;
    char formattedTime[20];
    formatTime(timeClient.getEpochTime(), formattedTime);
    timeClient.update();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    lcd.setCursor(0, 0);
    lcd.print(formattedTime);

    lcd.setCursor(1, 1);
    lcd.print(temperature);
    lcd.print(" C");
    lcd.setCursor(9, 1);
    lcd.print(humidity);
    lcd.print(" %");

    Serial.println(temperature);
    Serial.println(humidity);
    Serial.println(formattedTime);
    sendDataToServer(temperature, humidity, timeClient.getEpochTime());
  }
  if (currentMillis - previousMillis2 >= 100) {
    previousMillis2 = currentMillis;
    getDataFromServer();
  }
}



void formatTime(unsigned long epochTime, char* buffer) {
  sprintf(buffer, "%02d:%02d %02d/%02d/%04d",
          hour(epochTime), minute(epochTime),
          day(epochTime), month(epochTime), year(epochTime));
}

void sendDataToServer(float temperature, float humidity, unsigned long timestamp) {
  char formattedTime[20];
  formatTime(timestamp, formattedTime);

  String jsonData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + ",\"timestamp\":\"" + String(formattedTime) + "\"}";

  WiFiClient client;

  if (client.connect(serverAddress, serverPort)) {
    client.println("POST /api/data HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(jsonData.length()));
    client.println();
    client.println(jsonData);
    client.println();
    Serial.println("Data sent to server");
  } else {
    Serial.println("Failed to connect to server");
  }
  client.stop();
}

void getDataFromServer() {
  HTTPClient http;
  WiFiClient client;
  http.begin(client, serverUrl);

  int httpResponseCode = http.GET();

  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    DynamicJsonDocument jsonDoc(1024);
    DeserializationError error = deserializeJson(jsonDoc, response);
    int data_rcv = jsonDoc[0]["value"];
    Serial.println(response);
    Serial.println("Server response: ");
    Serial.println(data_rcv);
    controlLed(data_rcv);
    // Thực hiện xử lý với dữ liệu nhận được từ máy chủ
  } else {
    Serial.printf("HTTP Request failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}

void controlLed(int controlLed) {
    if (controlLed == 0) {
      digitalWrite(ledpin, 0);
      Serial.println("tat den");
    };
    if (controlLed == 1) {
      digitalWrite(ledpin, 1);
      Serial.println("bat den");
    };
}
