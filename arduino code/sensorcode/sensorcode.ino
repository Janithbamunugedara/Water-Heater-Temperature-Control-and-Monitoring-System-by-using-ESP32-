#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wi-Fi credentials
const char* ssid = "A14";
const char* password = "12345678";

// MQTT settings
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttClientID = "ESP32_SENSOR_NODE";
const char* topic = "waterheater/sensor/temperature";

// DS18B20 setup on GPIO 4
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Wi-Fi & MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  sensors.begin();

  connectWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setBufferSize(512);  // for longer JSON messages
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("📡 Temperature: ");
    Serial.println(tempC);

    String payload = "{\"device_id\":\"ESP32_SENSOR_NODE\",\"temperature_celsius\":";
    payload += String(tempC, 2);
    payload += "}";

    client.publish(topic, payload.c_str());
  } else {
    Serial.println("❌ DS18B20 not detected!");
  }

  delay(3000); // Send every 3 seconds
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int retries = 0;

  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ WiFi Failed. Restarting...");
    delay(2000);
    ESP.restart();
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT... ");
    if (client.connect(mqttClientID)) {
      Serial.println("✅ MQTT Connected!");
    } else {
      Serial.print("❌ MQTT Failed. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}