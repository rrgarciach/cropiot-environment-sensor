#include "Arduino.h"
#include "CropIoTDeviceSettings.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"
#include "Adafruit_BMP085.h"
#include "../include/device_endpoints.h"

// DHT22 module DATA   -> Nodemcu D5
// DHT22 module GND  -> Nodemcu GND
// DHT22 module V+  -> Nodemcu 3v
// BMP180 module SDA   -> Nodemcu D2
// BMP180 module SCL   -> Nodemcu D1
// BMP180 module GND  -> Nodemcu GND
// BMP180 module V+  -> Nodemcu 3v
// MQ135 module A0   -> Nodemcu A0
// MQ135 module GND  -> Nodemcu GND
// MQ135 module V+  -> Nodemcu 3v

#define DHT_PIN D5
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE); // Initialize DHT sensor
Adafruit_BMP085 bmp; // Initialize BMP180 sensor

float humidity;
float temperature;
int ppm;
float pressure;
float altitude;
float temperature2;
float seaLevelPressure;

/* Bug workaround for Arduino 1.6.6, it seems to need a function declaration
for some reason (only affects ESP8266, likely an arduino-builder bug). */
void readDHT22();
void readMQ135();
void readBMP180();
void loadDeviceEndpoints();

void setup()
{
  DEVICE_TYPE = "environment_sensor_v1";
  DEVICE_PASSWORD = "uiui2424";

  Serial.begin(115200);
  Serial.println("Starting...");

  connectWiFi();
  connectMQTT();
  loadDeviceEndpoints();

  dht.begin();
  if (!bmp.begin())
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
}

void loop()
{
  reconnectWiFi();
  reconnectMQTT();
  readDHT22();
  readMQ135();
  readBMP180();
  delay(10*1000);
}

void readDHT22() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");

  String message = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
  mqttClient.publish(TB_V1_TELEMETRY, message.c_str());
}

void readMQ135() {
  ppm = analogRead(0);
  Serial.print("CO2: ");
  Serial.print(ppm, DEC);
  Serial.println(" ppm");

  String message = "{\"ppm\": " + String(ppm, DEC) + "}";
  mqttClient.publish(TB_V1_TELEMETRY, message.c_str());
}

void readBMP180() {
  pressure =  bmp.readPressure();
  pressure /= 100;
  altitude =  bmp.readAltitude();
  temperature2 =  bmp.readTemperature();
  seaLevelPressure = bmp.readSealevelPressure();
  seaLevelPressure /= 100;
  Serial.print("Pressure: " + String(pressure) + " hPa ");
  Serial.print("Altitude: " + String(altitude) + " meters ");
  Serial.print("Temperature: " + String(temperature2) + " *C ");
  Serial.println("Pressure at sealevel (calculated): " + String(seaLevelPressure) + " hPa ");

  String message = "{\"pressure\": " + String(pressure) + ", \"altitude\": " + String(altitude) + ", \"temperature2\": " + String(temperature2) + ", \"seaLevelPressure\": " + String(seaLevelPressure) + "}";
  mqttClient.publish(TB_V1_TELEMETRY, message.c_str());
}

void loadDeviceEndpoints() {
  server.on(DEVICE_URLS.API.DEVICE.DATA, HTTP_GET, [&](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["humidity"] = humidity;
    root["temperature"] = temperature;
    root["ppm"] = ppm;
    root["pressure"] = pressure;
    root["altitude"] = altitude;
    root["temperature2"] = temperature2;
    root["seaLevelPressure"] = seaLevelPressure;
    root.printTo(*response);
    request->send(response);
  });
}
