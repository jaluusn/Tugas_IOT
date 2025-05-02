#define BLYNK_TEMPLATE_ID "TMPL6Ojxa290V"
#define BLYNK_TEMPLATE_NAME "System Hidroponik 01"
#define BLYNK_AUTH_TOKEN "JmSVnU8G6Tj8ytFCEkEqjkr_Ffwr15pi"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"
char pass[] = "";

#define TANK_HEIGHT 70      // Ketinggian tandon air (dalam cm)
#define MIN_WATER_PERCENT 20 // Persentase minimum air dalam tandon sebelum menghidupkan pompa
#define MAX_WATER_PERCENT 85 // Persentase maksimum air dalam tandon sebelum mematikan pompa

bool isPumpAirRunning = false;
bool isPumpNutrisiRunning = false;
bool isPumpPhDownRunning = false;
bool isPumpPhUpRunning = false;
bool isPumpMistingRunning = false;
bool isLEDRunning = false;

#define VPIN_Jarak          V0
// #define VPIN_Lux            V1
#define VPIN_Suhu           V1 
#define VPIN_Kelembaban     V2 
#define VPIN_Ph             V4 
#define VPIN_tds            V5 
#define VPIN_Status         V6
// #define VPIN_led            V7  

LiquidCrystal_I2C lcd(0x27, 20, 4); // Alamat I2C dan ukuran LCD 20x4

#define Led 15
#define PumpAir 2
#define PumpNutrisi 4
#define PumpMisting 5
#define PumpPhUp 18
#define PumpPhDown 19
#define ldrPin   13
#define DHTPIN 25
#define trigPin 27
#define echoPin 26
#define phSensorPin 35
#define TdsSensorPin 34

float pHValue = 0.0;           // Variabel untuk menyimpan nilai pH
float lowerPHLimit = 6.0;      // Batas bawah rentang pH yang diinginkan
float upperPHLimit = 7.0;

float lowerTemperatureLimit = 15.0;   
float upperTemperatureLimit = 25.0;   
float lowerHumidityLimit = 80.0;      
float upperHumidityLimit = 90.0;

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
unsigned long previousMillis = 0;
const long interval = 1000;  // Interval pembacaan setiap 1 detik
// const long intervalJarak = 1000;

const float GAMMA = 0.7; //nilai ketetapan gama untuk mencari tingkat kecerahan(lux)
const float RL10 = 50;

int sensorIndex = 0;

BlynkTimer timer;

void sendSensor(){
 

   unsigned long currentMillis = millis();

    lcd.clear(); // Hapus isi layar LCD sebelum menampilkan data sensor baru

    // Menampilkan teks "IoT System" di tengah baris pertama
    lcd.setCursor((20 - 10) / 2, 0); // Pusatkan teks "IoT System"
    lcd.print("IoT System");

    // Membaca sensor DHT22
    if (sensorIndex == 0) {
      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();

      if (!isnan(humidity) && !isnan(temperature)) {
        Blynk.virtualWrite(VPIN_Kelembaban, humidity);
        Blynk.virtualWrite(VPIN_Suhu, temperature);

        // Menampilkan data sensor kelembaban dan suhu di tengah baris kedua
        lcd.setCursor((20 - 21) / 2, 1); // Pusatkan data sensor
        lcd.print("Kelembaban: ");
        lcd.print(humidity);
        lcd.print("%");
        lcd.setCursor((20 - 14) / 2, 2); // Pusatkan data sensor
        lcd.print("Suhu: ");
        lcd.print(temperature);
        lcd.write(0xDF);
        lcd.print("C");

        if (temperature < lowerTemperatureLimit || temperature > upperTemperatureLimit || humidity < lowerHumidityLimit || humidity > upperHumidityLimit) {
          // Turn on the misting pump
          digitalWrite(PumpMisting, HIGH);
          isPumpMistingRunning=true;
          Serial.println("Turn on the misting pump");
        } else {
          // Turn off the misting pump if temperature and humidity are within the desired range
          digitalWrite(PumpMisting, LOW);
          isPumpMistingRunning=false;
          Serial.println("Turn off the misting pump");
        }
        String pumpMistingStatus = isPumpMistingRunning ? "ON" : "OFF";
        Blynk.virtualWrite(VPIN_Status,"status pompa Misting "+String(pumpMistingStatus));
        lcd.setCursor((20 - 18) / 2, 3);
        lcd.print("Pump Misting :  ");
        lcd.print(pumpMistingStatus);
      }
    }

    // Membaca sensor ultrasonik HC-SR04
    else if (sensorIndex == 1) {
      long duration;
      float distance_cm;

      // Kirim sinyal ultrasonik
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // Baca durasi pulsa Echo
      duration = pulseIn(echoPin, HIGH);

      // Hitung jarak dalam centimeter
      distance_cm = (float)duration / 58.2;

      float tankHeight = TANK_HEIGHT - distance_cm;
      float waterPercent = (tankHeight / TANK_HEIGHT) * 100.0;
      Serial.println(waterPercent);
      Serial.println(distance_cm);
      Blynk.virtualWrite(VPIN_Jarak, waterPercent);
      
      // Menampilkan data sensor jarak di tengah baris kedua
      lcd.setCursor((20 - 15) / 2, 1); // Pusatkan data sensor
      lcd.print("Jarak: ");
      lcd.print(distance_cm);
      lcd.print(" cm");
      lcd.setCursor((20 - 18) / 2, 2); // Pusatkan data sensor
      lcd.print("Persentase: ");
      lcd.print(waterPercent);
      lcd.print(" %");

      if (waterPercent < MIN_WATER_PERCENT) {
          Serial.println("Menghidupkan Pompa");
          digitalWrite(PumpAir, HIGH); // Hidupkan pompa
          isPumpAirRunning = true;
      } 
      if (waterPercent >= MAX_WATER_PERCENT) {
          Serial.println("Mematikan Pompa");
          digitalWrite(PumpAir, LOW);  // Matikan pompa
          isPumpAirRunning = false;
      }
      String pumpAirStatus = isPumpAirRunning ? "ON" : "OFF";
      Blynk.virtualWrite(VPIN_Status,"status pompa Air "+String(pumpAirStatus));
      lcd.setCursor((20 - 8) / 2, 3);
      lcd.print("Pump ");
      lcd.print(pumpAirStatus);
      
    }

    // Membaca sensor LDR
    else if (sensorIndex == 2) {
      int ldrValue = analogRead(33); //membaca nilai analog pada pin A0 dari sensor ldr
      float voltase = ldrValue * 5 / 4095.0; //rumus mengubah nilai sinyal analog dari ldr menjadi nilai voltase
      float resistansi = 2000 * voltase / (1 - voltase / 5); //rumus mencari nilai resistansi dari nilai voltase sebelumnya
      float kecerahan = pow(RL10 * 1e3 * pow(10, GAMMA) / resistansi, (1 / GAMMA)); //rumus mencari nilai tingkat kecerahan (lux)
      Blynk.virtualWrite(VPIN_Status, "kecerahan = "+String(kecerahan));

      // Menampilkan data sensor LDR di tengah baris kedua
      lcd.setCursor((20 - 15) / 2, 1); // Pusatkan data sensor
      lcd.print("LDR: ");
      lcd.print(kecerahan);
      lcd.print("lux");
      
      if (kecerahan < 5000) {
        // Hidupkan lampu LED jika kurang dari 5000 lux
        digitalWrite(Led, HIGH);
        Blynk.virtualWrite(VPIN_Status, "Led Status On");
        isLEDRunning=true;
      } else if (kecerahan > 7000) {
        // Matikan lampu LED jika lebih dari 7000 lux
        digitalWrite(Led, LOW);
        Blynk.virtualWrite(VPIN_Status, "Led Status OFF");
        isLEDRunning=false;
      }
      String LedStatus = isLEDRunning? "ON" : "OFF";
      lcd.setCursor((20 - 9) / 2, 3);
      lcd.print("Led : ");
      lcd.print(LedStatus);
    }
    // Membaca sensor pH
    else if (sensorIndex == 3) { 
      // float phSensor = map(analogRead(phSensorPin), 0, 4095, 0, 14);
      float inputVoltage = 3.3; // Tegangan referensi (misalnya, 3.3V)
      int maxValue = 4095;     // Maksimum nilai dari pembacaan analog 12-bit
      float maxVoltage = 14.0; // Maksimum rentang yang Anda inginkan

      int analogValue = analogRead(phSensorPin); // Membaca nilai analog 12-bit dari pin A0
      float voltage = (analogValue / (float)maxValue) * inputVoltage;
      pHValue= (voltage / inputVoltage) * maxVoltage;
      Blynk.virtualWrite(VPIN_Ph, pHValue);
      
      // Menampilkan data sensor pH di tengah baris kedua
      lcd.setCursor((20 - 18) / 2, 1); // Pusatkan data sensor
      lcd.print("Nilai Ph air: ");
      lcd.print(pHValue);
      if (pHValue < lowerPHLimit) {
        // Aktifkan pompa PH up
        digitalWrite(PumpPhUp, HIGH);
        isPumpPhUpRunning = true;
        digitalWrite(PumpPhDown, LOW); // Pastikan pompa PH down mati
        isPumpPhDownRunning = false;
        Serial.println("Aktifkan pompa PH up");
        String pumpPhUpStatus = isPumpPhUpRunning? "ON" : "OFF";
        Blynk.virtualWrite(VPIN_Status,"Status Pompa Ph up "+String(pumpPhUpStatus));
        lcd.setCursor((20 - 11) / 2, 2);
        lcd.print("Ph Up : ");
        lcd.print(pumpPhUpStatus);
        
      } else if (pHValue > upperPHLimit) {
        // Aktifkan pompa PH down
        digitalWrite(PumpPhUp, LOW); // Pastikan pompa PH up mati
        isPumpPhUpRunning=false;
        digitalWrite(PumpPhDown, HIGH);
        isPumpPhDownRunning=true;
        Serial.println("Aktifkan pompa PH down");
        String pumpPhDownStatus = isPumpPhDownRunning? "ON" : "OFF";
        Blynk.virtualWrite(VPIN_Status,"Status Pompa Ph Down "+String(pumpPhDownStatus));
        lcd.setCursor((20 - 13) / 2, 3);
        lcd.print("Ph Down : ");
        lcd.print(pumpPhDownStatus);
      } else {
        // Matikan kedua pompa jika pH berada di rentang yang diinginkan
        digitalWrite(PumpPhUp, LOW);
        isPumpPhUpRunning=false;
        digitalWrite(PumpPhDown, LOW);
        isPumpPhDownRunning=true;
        Serial.println("Matikan kedua pompa");
      }
    }

    // Membaca sensor TDS
    else if (sensorIndex == 4) {
      int TdsSensor = map(analogRead(TdsSensorPin), 0, 4095, 0, 1000);

      // Menampilkan data sensor TDS di tengah baris kedua
      lcd.setCursor((20 - 18) / 2, 1); // Pusatkan data sensor
      lcd.print("Nilai TDS: ");
      lcd.print(TdsSensor);
      lcd.print(" PPM");

      Blynk.virtualWrite(VPIN_tds, TdsSensor);
      if (TdsSensor < 560) {
        // Aktifkan pompa
        digitalWrite(PumpNutrisi, HIGH);
        isPumpNutrisiRunning=true;
        Serial.println("Aktifkan pompa Nutrisi");
      }
  
      // Cek apakah nilai TDS mencapai atau melebihi 840
      if (TdsSensor >= 840) {
        // Matikan pompa
        digitalWrite(PumpNutrisi, LOW);
        isPumpNutrisiRunning=false;
        Serial.println("Matikan pompa Nutrisi");
        }
        
      String pumpNutrisiStatus = isPumpNutrisiRunning? "ON" : "OFF";
      Blynk.virtualWrite(VPIN_Status,"Status Pompa Nutrisi "+String(pumpNutrisiStatus));
      lcd.setCursor((20 - 8) / 2, 3);
      lcd.print("Pump ");
      lcd.print(pumpNutrisiStatus);

      }

    sensorIndex++; // Pindah ke sensor berikutnya
    if (sensorIndex > 4) {
      sensorIndex = 0; // Kembali ke sensor pertama setelah selesai
    }
}

void setup(){
  Serial.begin(115200);
  dht.begin();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(Led,OUTPUT);
  pinMode(PumpAir, OUTPUT);
  pinMode(PumpNutrisi,OUTPUT);
  pinMode(PumpMisting,OUTPUT);
  pinMode(PumpPhUp,OUTPUT);
  pinMode(PumpPhUp,OUTPUT);

  digitalWrite(PumpPhUp, LOW);  // Matikan pompa PH up saat awalnya
  digitalWrite(PumpPhDown, LOW);
  digitalWrite(PumpAir,LOW);
  digitalWrite(PumpNutrisi,LOW);
  digitalWrite(PumpMisting,LOW);
  digitalWrite(Led,LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor((20-3)/2, 0);
  lcd.print("IoT");
  lcd.setCursor((20-11)/2, 1);
  lcd.print("hydroponics");
  lcd.setCursor((20-6)/2, 2);
  lcd.print("system");

  Blynk.begin(auth,ssid,pass);

  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  
}

void loop(){
// readSensor();
Blynk.run();
timer.run();
}
