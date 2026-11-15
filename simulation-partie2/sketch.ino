#include <DHTesp.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <math.h>

// Partie 2 : surveillance simulee et telemetrie MQTT.
// Les entrees sont des simulateurs Wokwi, pas des capteurs medicaux.
constexpr int PIN_PULSE = 34;  // potentiometre : pouls simule, 40 a 180 bpm
constexpr int PIN_DHT = 15;     // DHT22 : temperature ambiante et humidite simulees
constexpr int PIN_BUZZER = 27;
constexpr int LED_NORMAL = 26;
constexpr int LED_PULSE = 25;  // jaune : pouls eleve
constexpr int LED_TEMP = 33;   // rouge : temperature elevee
constexpr float SEUIL_PULSE_BPM = 140.0f;
constexpr float SEUIL_TEMP_C = 39.0f;  // seuil de demonstration, a valider

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_HOST = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "wearable/esprit/khalifa-demo-742/telemetry";

DHTesp dht;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
unsigned long lastSampleMs = 0;
unsigned long lastMqttAttemptMs = 0;
constexpr unsigned long SAMPLE_PERIOD_MS = 2000;
constexpr unsigned long MQTT_RETRY_MS = 5000;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Connexion au Wi-Fi Wokwi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  const unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) {
    delay(250);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connecte.");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi indisponible : les alertes locales restent actives.");
  }
}

void connectMqttIfNeeded() {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (millis() - lastMqttAttemptMs < MQTT_RETRY_MS) return;
  lastMqttAttemptMs = millis();
  Serial.print("Connexion MQTT... ");
  String clientId = "wokwi-gilet-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("OK");
  } else {
    Serial.print("echec, code ");
    Serial.println(mqtt.state());
  }
}

void publishTelemetry(int pulseBpm, float temperatureC, float humidityPct,
                      bool sensorOk, bool alarm) {
  if (!mqtt.connected()) {
    Serial.println("MQTT non connecte : mesure non publiee.");
    return;
  }

  char tempText[16];
  char humidityText[16];
  if (sensorOk) {
    snprintf(tempText, sizeof(tempText), "%.2f", temperatureC);
    snprintf(humidityText, sizeof(humidityText), "%.2f", humidityPct);
  } else {
    strcpy(tempText, "null");
    strcpy(humidityText, "null");
  }

  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"pulse\":%d,\"temperature\":%s,\"humidity\":%s,\"alarm\":%s,\"sensor_ok\":%s}",
           pulseBpm, tempText, humidityText,
           alarm ? "true" : "false", sensorOk ? "true" : "false");
  if (mqtt.publish(MQTT_TOPIC, payload)) {
    Serial.print("MQTT publie sur ");
    Serial.print(MQTT_TOPIC);
    Serial.print(" : ");
    Serial.println(payload);
  } else {
    Serial.println("Echec de publication MQTT.");
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  dht.setup(PIN_DHT, DHTesp::DHT22);
  pinMode(LED_NORMAL, OUTPUT);
  pinMode(LED_PULSE, OUTPUT);
  pinMode(LED_TEMP, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(256);

  Serial.println("Smart Vest - simulation Partie 2");
  connectWiFi();
}

void loop() {
  connectWiFi();
  connectMqttIfNeeded();
  mqtt.loop();

  const unsigned long now = millis();
  if (now - lastSampleMs < SAMPLE_PERIOD_MS) return;
  lastSampleMs = now;

  const int rawPulse = analogRead(PIN_PULSE);
  const int pulseBpm = 40 + (int)lroundf((rawPulse / 4095.0f) * 140.0f);
  const TempAndHumidity reading = dht.getTempAndHumidity();
  const bool sensorOk = isfinite(reading.temperature) && isfinite(reading.humidity);
  const bool pulseHigh = pulseBpm > SEUIL_PULSE_BPM;
  const bool temperatureHigh = sensorOk && reading.temperature >= SEUIL_TEMP_C;
  const bool alarm = pulseHigh || temperatureHigh;

  digitalWrite(LED_PULSE, pulseHigh ? HIGH : LOW);
  digitalWrite(LED_TEMP, temperatureHigh ? HIGH : LOW);
  digitalWrite(LED_NORMAL, (!alarm && sensorOk) ? HIGH : LOW);
  if (alarm) tone(PIN_BUZZER, 2200);
  else noTone(PIN_BUZZER);

  Serial.printf("Pouls simule : %d bpm | Temperature : ", pulseBpm);
  if (sensorOk) Serial.printf("%.2f C | Humidite : %.2f %%\n", reading.temperature, reading.humidity);
  else Serial.println("indisponible (verifier le DHT22)");
  if (pulseHigh) Serial.println("ALERTE : pouls superieur a 140 bpm");
  if (temperatureHigh) Serial.println("ALERTE : temperature anormale");

  publishTelemetry(pulseBpm, reading.temperature, reading.humidity, sensorOk, alarm);
}
