#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1015.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define PH_PIN A0
#define PUMP_UP_PIN 5
#define PUMP_DOWN_PIN 6
#define WATER_LEVEL_PIN A1

Adafruit_ADS1115 ads;  // Objek untuk ADS1115 (analog-to-digital converter)

const float PH4 = 1.7;
const float PH7 = 1.2;
const int WATER_LEVEL_THRESHOLD = 500;

const char *ssid = "your-ssid";
const char *password = "your-password";
const char *telegramBotToken = "your-telegram-bot-token";
const long chatId = 123456789;  // Ganti dengan ID obrolan Telegram yang sesuai

WiFiClientSecure client;
UniversalTelegramBot bot(telegramBotToken, client);

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
  handleTelegramMessages();  // Periksa pesan Telegram

  float phValue = readPH();
  int waterLevel = readWaterLevel();

  Serial.print("pH: ");
  Serial.println(phValue, 2);

  Serial.print("Water Level: ");
  Serial.println(waterLevel);

  // Kontrol pH secara otomatis
  if (phValue < 6.5) {
    pumpDown();
    sendTelegramMessage("pH level is too low, adding pH up.");
  } else if (phValue > 7.5) {
    pumpUp();
    sendTelegramMessage("pH level is too high, adding pH down.");
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
  delay(5000);  // Sesuaikan dengan waktu operasi pompa
  digitalWrite(PUMP_UP_PIN, LOW);
}

void pumpDown() {
  digitalWrite(PUMP_DOWN_PIN, HIGH);
  delay(5000);  // Sesuaikan dengan waktu operasi pompa
  digitalWrite(PUMP_DOWN_PIN, LOW);
}

void sendTelegramMessage(const char *message) {
  bot.sendMessage(chatId, message, "");
}

void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    Serial.println("got response");

    String chat_id = String(bot.messages[0].chat_id);
    String text = bot.messages[0].text;

    if (text == "/ph") {
      float phValue = readPH();
      String response = "Current pH level: " + String(phValue, 2);
      bot.sendMessage(chat_id, response, "");
    } else if (text == "/waterlevel") {
      int waterLevel = readWaterLevel();
      String response = "Current water level: " + String(waterLevel);
      bot.sendMessage(chat_id, response, "");
    } else if (text == "/status") {
      float phValue = readPH();
      int waterLevel = readWaterLevel();

      String response = "Current Status:\n";
      response += "pH level: " + String(phValue, 2) + "\n";
      response += "Water level: " + String(waterLevel) + "\n";

      bot.sendMessage(chat_id, response, "");
    }

    numNewMessages--;
  }
}

// Platform.ini
// [env:esp32doit-devkit-v1]
// platform = espressif32
// board = esp32doit-devkit-v1
// framework = arduino
// lib_deps =
//    adafruit/Adafruit ADS1X15@^1.0.2
//    adafruit/Adafruit Unified Sensor@^1.1.4
//    knolleary/PubSubClient@^2.8  ; Library for MQTT
//    [ID]: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
