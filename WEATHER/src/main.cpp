#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

//BUKA YANG NAMANYA OPEN WEATHER API  https://openweathermap.org/ 

const char* ssid = "Wokwi-GUEST";
const char* password = "";
String apiKey = "4ab694cd206f1084e10b52d10dd52e55"; //GANTI DISINIIIIIIIIII
String city = "Malang";
String units = "metric";

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int nextButtonPin = 14;
const int prevButtonPin = 27;

int page = 0;
const int maxPage = 2;

unsigned long lastButtonPress = 0;
unsigned long lastUpdate = 0;
String temp = "";
String desc = "";
String humidity = "";
String wind = "";

void fetchWeather() {
  String server = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&appid=" + apiKey;
  HTTPClient http;
  http.begin(server);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);

    int tempIndex = payload.indexOf("temp");
    temp = payload.substring(tempIndex + 6, payload.indexOf(",", tempIndex));

    int descIndex = payload.indexOf("description");
    desc = payload.substring(descIndex + 14, payload.indexOf("\"", descIndex + 14));

    int humIndex = payload.indexOf("humidity");
    humidity = payload.substring(humIndex + 9, payload.indexOf(",", humIndex));

    int windIndex = payload.indexOf("speed");
    wind = payload.substring(windIndex + 7, payload.indexOf(",", windIndex));
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
}

void displayPage() {
  lcd.clear();
  switch (page) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Temp: " + temp + " C");
      lcd.setCursor(0, 1);
      lcd.print(desc);
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Humidity: " + humidity + "%");
      lcd.setCursor(0, 1);
      lcd.print("Wind: " + wind + " m/s");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("City:");
      lcd.setCursor(6, 0);
      lcd.print(city);
      lcd.setCursor(0, 1);
      lcd.print("Unit: " + units);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(prevButtonPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(1000);

  fetchWeather();
  displayPage();
}

void loop() {
  // Refresh data setiap 60 detik
  if (millis() - lastUpdate > 60000) {
    fetchWeather();
    displayPage();
    lastUpdate = millis();
  }

  if (digitalRead(nextButtonPin) == LOW && millis() - lastButtonPress > 300) {
    page = (page + 1) % (maxPage + 1);
    displayPage();
    lastButtonPress = millis();
  }

  if (digitalRead(prevButtonPin) == LOW && millis() - lastButtonPress > 300) {
    page = (page - 1 + (maxPage + 1)) % (maxPage + 1);
    displayPage();
    lastButtonPress = millis();
  }

  delay(50);
}
