#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1015.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define PH_PIN A13
#define PUMP_UP_PIN 5
#define PUMP_DOWN_PIN 18  
#define WATER_LEVEL_PIN A12

Adafruit_ADS1015 ads;  // Objek untuk ADS1115 (analog-to-digital converter)

const float PH4 = 1.7;
const float PH7 = 1.2;
const int WATER_LEVEL_THRESHOLD = 500;  // Nilai ambang untuk tingkat air rendah

const char *ssid = "PAK MIYAS";
const char *password = "miyas1957";
const char *telegramBotToken = "6921611133:AAH9P1LSv8nToyo8UmXfcJuzav3ouVM82pY";
const long chatId = 1774969820;

float readPH();
int readWaterLevel();
void pumpUp();
void pumpDown();
void sendTelegramMessage(const char *message);


void setup() {
  Serial.begin(9600);
  pinMode(PH_PIN, INPUT);
  pinMode(PUMP_UP_PIN, OUTPUT);
  pinMode(PUMP_DOWN_PIN, OUTPUT);

  // Setup koneksi Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  float phValue = readPH();
  int waterLevel = readWaterLevel();

  Serial.print("pH: ");
  Serial.println(phValue, 2);

  Serial.print("Water Level: ");
  Serial.println(waterLevel);

  // Kontrol pH secara otomatis
  if (phValue < 6.5) {
    pumpDown();  // Kontrol pH down
    sendTelegramMessage("pH level is too low, adding pH up. \n (pH level terlalu rendah, menambahkan pH up.)");
  } else if (phValue > 7.5) {
    pumpUp();    // Kontrol pH up
    sendTelegramMessage("pH level is too high, adding pH down. \n (pH level terlalu tinggi, menambahkan pH down.)");
  }

  // Peringatkan jika tingkat air rendah
  if (waterLevel < WATER_LEVEL_THRESHOLD) {
    sendTelegramMessage("Warning: Water level is low. Refill needed!");
  }

  delay(5000);  // Tunda 5 detik sebelum membaca ulang
}

float readPH() {
  int rawValue = analogRead(PH_PIN);
  float voltage = rawValue * (5.0 / 1024.0);
  float pH = 7.0 + ((PH7 - voltage) / ((PH7 - PH4) / 3.0));
  return pH;
}

int readWaterLevel() {
  int waterLevel = analogRead(WATER_LEVEL_PIN);
  return waterLevel;
}

void pumpUp() {
  digitalWrite(PUMP_UP_PIN, HIGH);
  delay(5000);  // Pompa dijalankan selama 5 detik, disesuaikan sesuai kebutuhan
  digitalWrite(PUMP_UP_PIN, LOW);
}

void pumpDown() {
  digitalWrite(PUMP_DOWN_PIN, HIGH);
  delay(5000);  // Pompa dijalankan selama 5 detik, disesuaikan sesuai kebutuhan
  digitalWrite(PUMP_DOWN_PIN, LOW);
}

void sendTelegramMessage(const char *message) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(telegramBotToken) +
               "/sendMessage?chat_id=" + String(chatId) +
               "&text=" + String(message);
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.println("Telegram message sent successfully.");
  } else {
    Serial.println("Error sending Telegram message.");
  }
  http.end();
}


// Platform.ini
// [env:esp32doit-devkit-v1]
// platform = espressif32
// board = esp32doit-devkit-v1
// framework = arduino

// build_flags = 
//     -DCORE_DEBUG_LEVEL=3
//     -DCONFIG_ARDUHAL_LOG_COLORS=1

// monitor_filters = direct

// lib_deps =
//    adafruit/Adafruit ADS1X15@^1.0.2
//    adafruit/Adafruit Unified Sensor@^1.1.4
//    esp32-http-client
