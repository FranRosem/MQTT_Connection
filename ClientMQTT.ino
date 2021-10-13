#include <WiFi.h>
#include <PubSubClient.h>

/* Wifi Properties */
WiFiClient wifiClient;
const char* WIFI_SSID = "TIGO-SHARON1";
const char* WIFI_PASS = "wa8ufm98dasd";

/* Broker MQTT Properties */
PubSubClient mqttClient(wifiClient);
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* IN_TOPIC_LEDS = "ucb/grupo6/leds";   // subscribe
const char* OUT_TOPIC_DISTANCE = "ucb/grupo6/distance"; // publish

/* MCU ESP32 Properties */
const char* CLIENT_ID = "Oksem1"; // unique client id
const int TRIGGER_PIN = 26;
const int ECHO_PIN = 27;
const int LED_GREEN = 13;
const int LED_YELLOW = 12;
const int LED_RED = 14;
unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

/* Functions */
void turnOnLed(int ledPin) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void turnOffLed(int ledPin) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return 0.01723 * pulseIn(echoPin, HIGH);
}

String getUltrasonicDistance() {
  return "Distance: " + String(readUltrasonicDistance(TRIGGER_PIN, ECHO_PIN)) + " cm.";
}

void turnOnLeds(const char* topic, String message){
  if (String(topic) == IN_TOPIC_LEDS) {
    if (message == "1ON")  turnOnLed(LED_RED);
    else if (message == "1OFF") turnOffLed(LED_RED);
    else if (message == "2ON")  turnOnLed(LED_YELLOW);
    else if (message == "2OFF") turnOffLed(LED_YELLOW);
    else if (message == "3ON")  turnOnLed(LED_GREEN);
    else if (message == "3OFF") turnOffLed(LED_GREEN);
  }
  Serial.println("Message from topic " + String(IN_TOPIC_LEDS) + ": " + message);
}

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += String((char) payload[i]);
  }
  turnOnLeds(topic, message);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Couldn't connect to WiFi.");
    while(1) delay(100);
  }
  Serial.println("Connected to " + String(WIFI_SSID));
}

boolean mqttClientConnect() {
  Serial.println("Connecting to MQTT broker...");
  if (mqttClient.connect(CLIENT_ID)) {
    Serial.println("Connected to " + String(MQTT_BROKER));
    mqttClient.subscribe(IN_TOPIC_LEDS);
    Serial.println("Subscribed to " + String(IN_TOPIC_LEDS));
  }
  else {
    Serial.println("Couldn't connect to MQTT broker.");
  }
  return mqttClient.connected();
}

void publishMessage(const char* topic, String message) {
  mqttClient.publish(topic, message.c_str());
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

void loop() {
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    if (now - previousConnectMillis >= 5000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) {
        previousConnectMillis = 0;
      }
      else delay(1000);
    }
  }
  else {
    mqttClient.loop();
    delay(100);
    if (now - previousPublishMillis >= 1000) {
      previousPublishMillis = now;
      publishMessage(OUT_TOPIC_DISTANCE, getUltrasonicDistance());
    }
  }
}