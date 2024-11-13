#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Konfigurasi DHT
#define DHTPIN 15
#define DHTTYPE DHT22

// Konfigurasi Pin
#define LED_HIJAU 32
#define LED_KUNING 25
#define LED_MERAH 26
#define RELAY_PIN 2
#define BUZZER_PIN 4

DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi WiFi dan MQTT
const char* ssid = "Wokwi-GUEST";        // Ganti dengan SSID WiFi Anda
const char* password = ""; // Ganti dengan Password WiFi Anda

const char* mqtt_server = "test.mosquitto.org";
const char* clientID = "a52cbb99-bec5-4868-a8a5-07fa3a8b78de";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi DHT
  dht.begin();

  // Setup pin sebagai output
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Mulai WiFi dan MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Port default MQTT adalah 1883
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("clientID")) { // ID client unik untuk broker
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendStatusToMQTT(const char* topic, bool status) {
  client.publish(topic, status ? "ON" : "OFF");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca suhu dan kelembapan dari sensor DHT
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  // Cek jika pembacaan sensor valid
  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  // Kirim data suhu dan kelembapan melalui MQTT
  String suhu_str = String(suhu);
  String kelembapan_str = String(kelembapan);
  
  client.publish("sensor/suhu", suhu_str.c_str());
  client.publish("sensor/kelembapan", kelembapan_str.c_str());

  // Atur logika LED dan relay berdasarkan suhu
  bool ledHijauStatus = false;
  bool ledKuningStatus = false;
  bool ledMerahStatus = false;
  bool relayStatus = true;
  bool buzzerStatus = false;

  if (suhu > 35) {
    ledMerahStatus = true;
    buzzerStatus = true;
  } else if (suhu >= 30 && suhu <= 35) {
    ledKuningStatus = true;
  } else {
    ledHijauStatus = true;
  }

  // Update status perangkat ke MQTT
  sendStatusToMQTT("status/led_hijau", ledHijauStatus);
  sendStatusToMQTT("status/led_kuning", ledKuningStatus);
  sendStatusToMQTT("status/led_merah", ledMerahStatus);
  sendStatusToMQTT("status/relay_pompa", relayStatus);
  sendStatusToMQTT("status/buzzer", buzzerStatus);

  // Atur pin output berdasarkan status
  digitalWrite(LED_HIJAU, ledHijauStatus ? HIGH : LOW);
  digitalWrite(LED_KUNING, ledKuningStatus ? HIGH : LOW);
  digitalWrite(LED_MERAH, ledMerahStatus ? HIGH : LOW);
  digitalWrite(RELAY_PIN, relayStatus ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, buzzerStatus ? HIGH : LOW);

  // Tampilkan nilai suhu dan kelembapan di Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print("°C  Kelembapan: ");
  Serial.print(kelembapan);
  Serial.println("%");

  delay(2000); // Delay agar tidak terlalu cepat
}
