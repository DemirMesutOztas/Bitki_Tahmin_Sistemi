#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *sunucu = "http://192.168.1.114:5001/predict";
const float LATITUDE = 39.93;
const float LONGITUDE = 32.86;

void setup()
{
  Serial.begin(115200);
  delay(1000); // Seri monitörün oturması için kısa bir bekleme

  Serial.print("WiFi Baglaniyor...");
  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi baglandi!");
}

void loop()
{
  // Sensör Okumaları
  float ph = (analogRead(34) / 4095.0) * 7.0 + 3.0;
  float nem = (analogRead(35) / 4095.0) * 100.0;
  float azot = (analogRead(32) / 4095.0) * 140.0;
  float fosfor = (analogRead(33) / 4095.0) * 145.0;
  float potasyum = (analogRead(39) / 4095.0) * 205.0;

  /*Serial.println("--- Toprak Verileri ---");
  Serial.printf("pH: %.2f | Nem: %.2f%% | N: %.2f | P: %.2f | K: %.2f\n", ph, nem, azot, fosfor, potasyum); */

  Serial.println("\r\n==================================================");
  Serial.println("           🌱 TOPRAK ANALİZ SİSTEMİ 🌱            ");
  Serial.println("==================================================");
  Serial.println(" [1] SENSÖR OKUMALARI");
  Serial.printf("  > pH Seviyesi : %.2f\r\n", ph);
  Serial.printf("  > Toprak Nemi : %.2f%%\r\n", nem);
  Serial.printf("  > Azot (N)    : %.2f mg/kg\r\n", azot);
  Serial.printf("  > Fosfor (P)  : %.2f mg/kg\r\n", fosfor);
  Serial.printf("  > Potasyum(K) : %.2f mg/kg\r\n", potasyum);
  Serial.println("--------------------------------------------------");

  // 1. ADIM: Hava Durumu Verisi Çekme (GET)
  float sicaklik = 20.0;
  float yagis = 5.0;

  WiFiClient client;
  HTTPClient http;

  String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(LATITUDE) + "&longitude=" + String(LONGITUDE) + "&current=temperature_2m,relative_humidity_2m&daily=precipitation_sum&timezone=auto&forecast_days=1";

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      sicaklik = doc["current"]["temperature_2m"].as<float>();
      // Eğer daily dizisi boş değilse ilk elemanı al
      if (doc["daily"]["precipitation_sum"].is<JsonArray>())
      {
        yagis = doc["daily"]["precipitation_sum"][0].as<float>();
      }
    }
    else
    {
      Serial.print("Hava durumu JSON cozme hatasi: ");
      Serial.println(error.c_str());
    }
  }
  else
  {
    Serial.printf("Hava durumu API Hatasi! Kod: %d\n", httpCode);
  }
  http.end(); // Bağlantıyı güvenli bir şekilde kapat

  /*Serial.println("--- Hava Durumu ---");
  Serial.printf("Sicaklik: %.2f °C | Yagis: %.2f mm\n", sicaklik, yagis);*/

  Serial.println(" [2] HAVA DURUMU (Canlı API)");
  Serial.printf("  > Sıcaklık    : %.2f °C\r\n", sicaklik);
  Serial.printf("  > Yağış       : %.2f mm\r\n", yagis);
  Serial.println("--------------------------------------------------");

  // 2. ADIM: Kendi Model Sunucuna Veri Gönderme (POST)
  JsonDocument gonderi;
  gonderi["N"] = azot;
  gonderi["P"] = fosfor;
  gonderi["K"] = potasyum;
  gonderi["temperature"] = sicaklik;
  gonderi["humidity"] = nem;
  gonderi["ph"] = ph;
  gonderi["rainfall"] = yagis;

  String jsonBody;
  serializeJson(gonderi, jsonBody);

  WiFiClient client2;
  HTTPClient http2;
  http2.begin(client2, sunucu);

  http2.addHeader("Content-Type", "application/json");
  http2.addHeader("ngrok-skip-browser-warning", "true");

  int sonucKod = http2.POST(jsonBody);

  if (sonucKod == HTTP_CODE_OK)
  {
    String sonuc = http2.getString();
    JsonDocument sonucDoc;
    DeserializationError error = deserializeJson(sonucDoc, sonuc);

    if (!error && sonucDoc.containsKey("bitki"))
    {
      /*String bitki = sonucDoc["bitki"].as<String>();
      Serial.println("--- Model Tahmini ---");
      Serial.print("En ideal bitki: ");
      Serial.println(bitki);*/

      String bitki = sonucDoc["bitki"].as<String>();

      Serial.println(" [3] YAPAY ZEKA MODEL TAHMİNİ");
      Serial.println("  >>==================================<<");
      Serial.printf("  ||  EN İDEAL BİTKİ: %s \r\n", bitki.c_str());
      Serial.println("  >>==================================<<\r\n");
    }
    else
    {
      Serial.println("Tahmin sunucusundan gecersiz JSON yaniti dondu.");
    }
  }
  else
  {
    Serial.printf("Tahmin Sunucusu Hatasi! Kod: %d\n", sonucKod);
    // Ngrok tüneli açık mı veya backend ayakta mı kontrol edin
  }
  http2.end(); // Bağlantıyı kapat

  // İşlemciyi tamamen kilitlememek için delay esnasında arka plan görevlerine izin verilir
  delay(15000);
}