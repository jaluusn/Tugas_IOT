#define BLYNK_TEMPLATE_ID "TMPL6ICVX7BUP"
#define BLYNK_TEMPLATE_NAME "Sistem Hidroponik 02"
#define BLYNK_AUTH_TOKEN "3I3Ng81hT4areKRtVvsMJRflgtFkCRDK"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

// Pin Definitions
#define DHTPIN 14         // DHT22 data pin
#define LDR_PIN 34        // LDR sensor pin
#define POT_PIN 35        // Potensiometer pin
#define RELAY_PIN 12      // Relay module pin

// DHT Sensor Type
#define DHTTYPE DHT22

// LCD Setup (20x4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Blynk Authentication
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Variables
float temperature, humidity;
int ldrValue, potValue;
bool relayState = false;

// Konstanta untuk konversi LDR ke Lux
#define LUX_CONVERSION_FACTOR 0.976

// Blynk Virtual Pins
#define VPIN_TEMPERATURE V0
#define VPIN_HUMIDITY V1
#define VPIN_LIGHT V2
#define VPIN_RELAY V3
#define VPIN_POTENTIOMETER V4
#define VPIN_STATUS V5

// Function declarations
void readSensors();
void displaySensorData();
void controlLogic();
void sendToBlynk();
float convertToLux(int ldrRaw);

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("HIDROPONIK SMART FARM");
  lcd.setCursor(5, 1);
  lcd.print("SISTEM MONITORING");
  delay(3000);
  lcd.clear();

  // Initialize DHT sensor
  dht.begin();

  // Set pin modes
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(LDR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run(); // Jalankan Blynk
  
  // Read all sensors
  readSensors();
  
  // Display sensor values
  displaySensorData();
  
  // Control logic based on sensor readings
  controlLogic();
  
  // Send data to Blynk
  sendToBlynk();
  
  // Small delay between readings
  delay(2000);
}

void readSensors() {
  // Read temperature and humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Read LDR (Light Intensity) dan konversi ke Lux
  ldrValue = analogRead(LDR_PIN);
  
  // Read potentiometer
  potValue = analogRead(POT_PIN);
}

void displaySensorData() {
  lcd.clear();
  
  // Line 1: Temperature and Humidity
  lcd.setCursor(0, 0);
  lcd.print("Suhu:");
  lcd.setCursor(6, 0);
  lcd.print(temperature, 1);
  lcd.print("C");
  
  lcd.setCursor(11, 0);
  lcd.print("Hum:");
  lcd.setCursor(16, 0);
  lcd.print(humidity, 0);
  lcd.print("%");

  // Line 2: Light Intensity in Lux
  float lux = convertToLux(ldrValue);
  lcd.setCursor(0, 1);
  lcd.print("Cahaya:");
  lcd.setCursor(8, 1);
  lcd.print(lux, 0);
  lcd.print(" Lux");
  
  // Line 3: Light Status and Relay Status
  lcd.setCursor(0, 2);
  lcd.print("Status: ");
  lcd.setCursor(8, 2);
  
  // Tampilkan status cahaya
  if (lux < 5000) {
    lcd.print("Kurang Cahaya");
  } else if (lux > 30000) {
    lcd.print("Terlalu Terang");
  } else {
    lcd.print("Cahaya Cukup ");
  }

  // Line 4: Relay Status and System Message
  lcd.setCursor(0, 3);
  lcd.print("Relay:");
  lcd.setCursor(7, 3);
  lcd.print(relayState ? "ON " : "OFF");
  
  lcd.setCursor(11, 3);
  if (temperature > 30.0 || humidity > 80.0) {
    lcd.print("Misting ON");
  } else if (temperature < 18.0 || humidity < 40.0) {
    lcd.print("Suhu/Lembab Low");
  } else {
    lcd.print("Sistem Normal");
  }
}

float convertToLux(int ldrRaw) {
  // Konversi nilai analog LDR ke Lux
  return ldrRaw * LUX_CONVERSION_FACTOR;
}

void controlLogic() {
  // Kontrol relay berdasarkan suhu dan kelembaban
  if (temperature > 30.0 || humidity > 80.0) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
  } else if (temperature < 25.0 && humidity < 60.0) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
  }
}

void sendToBlynk() {
  // Kirim data sensor ke Blynk
  Blynk.virtualWrite(VPIN_TEMPERATURE, temperature);
  Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
  
  float lux = convertToLux(ldrValue);
  Blynk.virtualWrite(VPIN_LIGHT, lux);
  Blynk.virtualWrite(VPIN_POTENTIOMETER, potValue);
  Blynk.virtualWrite(VPIN_RELAY, relayState ? 1 : 0);
  
  // Kirim status sistem
  String systemStatus;
  if (temperature > 30.0 || humidity > 80.0) {
    systemStatus = "Misting ON";
  } else if (temperature < 18.0 || humidity < 40.0) {
    systemStatus = "Suhu/Lembab Low";
  } else {
    systemStatus = "Sistem Normal";
  }
  Blynk.virtualWrite(VPIN_STATUS, systemStatus);
}


const String API_URL = "http://your-laravel-api/api";
const String API_KEY = "your-secret-api-key";

void sendToLaravelAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    http.begin(API_URL + "/sensor-data");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + API_KEY);
    
    String payload = String("{") +
      "\"temperature\":" + temperature + "," +
      "\"humidity\":" + humidity + "," +
      "\"light\":" + convertToLux(ldrValue) + "," +
      "\"relay_status\":" + (relayState ? "1" : "0") +
    "}";
    
    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    }
    
    http.end();
  }
}
// Fungsi untuk mengontrol relay dari Blynk App
BLYNK_WRITE(VPIN_RELAY) {
  int pinValue = param.asInt();
  digitalWrite(RELAY_PIN, pinValue);
  relayState = pinValue;
}