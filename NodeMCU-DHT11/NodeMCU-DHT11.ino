#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>

#include "credentials.h"

#define DHTPIN    D2
#define DHTTYPE   DHT11
#define BAUDRATE  115200


unsigned long prevMillis = 0; //Previous milliseconds
const long interval = 2000;

float humidity;     // percentage
float temperature;  // Celsius
String currTime;

DHT dht(DHTPIN, DHTTYPE);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup() {
  Serial.begin(BAUDRATE);
  connectToWiFi(SSID, PASSWORD);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  timeClient.begin();
  timeClient.setTimeOffset(-10800); //Brasilia Time(BRT) UTC -3
  dht.begin();
}

void loop() {
  // Milliseconds passed since the program started
  unsigned long currMillis = millis();

  // Sending data according to the interval
  if (currMillis - prevMillis >= interval) {
    prevMillis = currMillis;     
    timeClient.update();
    currTime = timeClient.getFormattedTime();
    readSensors();
    displaySensors();
    sendDataToFirebase();
  }
}

void connectToWiFi(char const *ssid, char const *password) {
  // Setting the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);  
  
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  // connecting to WiFi
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  
  // Keep Loading while the wifi is not connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("WiFi connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void readSensors() {  
  // Reading Humidity value
  humidity = dht.readHumidity();
  
  // Reading temperature value
  temperature = dht.readTemperature();
  
  // Check if failed and exit.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read!"));
    return;
  }
}

void displaySensors() {
  Serial.println(currTime);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
}

void sendDataToFirebase() {
  String humValue = Firebase.pushFloat("dht11/humidity", humidity);
  if (Firebase.failed()) {
    Serial.print("\nError pushing /dht11/humidity failed: " + humValue);
    Serial.println(Firebase.error());
    return;
  }

  String tempValue = Firebase.pushFloat("dht11/temperature", temperature);
  if (Firebase.failed()) {
    Serial.print("\nError pushing /dht11/temperature failed:" + tempValue);
    Serial.println(Firebase.error());
    return;
  }

  Firebase.pushString("dht11/time", currTime);
}
