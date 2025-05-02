#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define LED_VERDE 12
#define LED_ROJO 13
#define BUZZER 14

// Credenciales de red WiFi (Wokwi)
const char* ssid = "Wokwi-GUEST";
const char* password = "";  // Sin contraseña
const char* ubidotsToken = "BBUS-0Pk6iw5RHkDsdQyxlgS0DicdDx9ttV";  // 🔥 Reemplaza con tu TOKEN de Ubidots
const char* ubidotsDevice = "esp32";  // Nombre del dispositivo en Ubidots
const char* botToken = "8006396625:AAFu2nSsQfc1PRD6iWlh3P7Pidz-fjpKltw"; // Token del bot de Telegram
const char* chatID = "7990466502"; // Chat ID de Telegram

WiFiClient client;
const int numLecturas = 10;
float temperaturas[numLecturas];
int indice = 0;
bool alertaEnviada = false;

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi");

    // 🔥 Inicializar todas las temperaturas en un valor seguro (por ejemplo, 25°C)
    for (int i = 0; i < numLecturas; i++) {
        temperaturas[i] = 25.0;  
    }
}

float calcularPromedio() {
    float suma = 0.0;
    for (int i = 0; i < numLecturas; i++) {
        suma += temperaturas[i];
    }
    return suma / numLecturas;
}

float calcularPendiente() {
    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (int i = 0; i < numLecturas; i++) {
        sumX += i;
        sumY += temperaturas[i];
        sumXY += i * temperaturas[i];
        sumX2 += i * i;
    }
    return (numLecturas * sumXY - sumX * sumY) / (numLecturas * sumX2 - sumX * sumX);
}

float predecirTemperatura() {
    float pendiente = calcularPendiente();
    float promedio = calcularPromedio();
    return promedio + pendiente * (numLecturas - 1);
}

void enviarAlertaTelegram(float temperatura, float prediccion) {
    HTTPClient http;
    String mensaje = "⚠️ ALERTA: Temperatura anómala!%0A";
    mensaje += "Actual: " + String(temperatura) + "°C%0A";
    mensaje += "Predicción: " + String(prediccion) + "°C";
    
    String url = "https://api.telegram.org/bot" + String(botToken) + "/sendMessage?chat_id=" + String(chatID) + "&text=" + mensaje;
    
    http.begin(url);
    int httpCode = http.GET();
    Serial.print("📡 Código HTTP de respuesta: ");
    Serial.println(httpCode);
    
    if (httpCode > 0) {
        Serial.println("📩 Alerta enviada a Telegram con éxito!");
        alertaEnviada = true;
    } else {
        Serial.println("❌ Error al enviar alerta a Telegram");
        alertaEnviada = false;
    }
    http.end();
}

// 📡 🔥 Función para enviar datos a Ubidots
void enviarDatosUbidots(float temperatura, float prediccion, float error, bool ledVerde, bool ledRojo, bool alertaTelegram) {
    HTTPClient http;
    String url = "http://industrial.api.ubidots.com/api/v1.6/devices/" + String(ubidotsDevice) + "/";
    
    String payload = "{";
    payload += "\"temperatura\":" + String(temperatura) + ",";
    payload += "\"prediccion\":" + String(prediccion) + ",";
    payload += "\"error\":" + String(error) + ",";
    payload += "\"led_verde\":" + String(ledVerde ? 1 : 0) + ",";
    payload += "\"led_rojo\":" + String(ledRojo ? 1 : 0) + ",";
    payload += "\"alerta_telegram\":" + String(ledRojo ? 1 : 0);
    payload += "}";

    Serial.print("📤 Enviando datos a Ubidots: ");
    Serial.println(payload);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Auth-Token", ubidotsToken);
    
    int httpCode = http.POST(payload);
    Serial.print("📡 Ubidots response: ");
    Serial.println(httpCode);
    http.end();
}

void loop() {
    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();

    if (isnan(temperatura) || isnan(humedad)) {
        Serial.println("Error al leer el sensor DHT22");
        return;
    }

    temperaturas[indice] = temperatura;
    indice = (indice + 1) % numLecturas;
    
    float prediccion = predecirTemperatura();
    float error = abs(temperatura - prediccion);

    Serial.print("🌡 Temperatura: ");
    Serial.print(temperatura);
    Serial.print("°C - 🔮 Predicción: ");
    Serial.print(prediccion);
    Serial.print(" - ⚠️ Error: ");
    Serial.println(error);

    bool ledVerde = false;
    bool ledRojo = false;

    if (error > 10.0) {
        digitalWrite(LED_ROJO, HIGH);
        digitalWrite(LED_VERDE, LOW);
        tone(BUZZER, 362, 250);
        Serial.println("⚠️ ALERTA: Temperatura anómala detectada!");
        enviarAlertaTelegram(temperatura, prediccion);
        ledRojo = true;
    } else {
        digitalWrite(LED_ROJO, LOW);
        digitalWrite(LED_VERDE, HIGH);
        Serial.println("✅ Temperatura en rango normal.");
        ledVerde = true;
    }
    
    enviarDatosUbidots(temperatura, prediccion, error, ledVerde, ledRojo, alertaEnviada);
    
    delay(5000);
}
