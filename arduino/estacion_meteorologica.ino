// ==========================================
// MINI ESTACIÓN METEOROLÓGICA
// Arduino UNO R4 WiFi + Sensores
// ==========================================

#include <WiFiS3.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

// ==================== CONFIGURACIÓN WIFI ====================
const char* ssid = "RED_WiFi";           // Cambiar por tu SSID
const char* password = "contraseña";     // Cambiar por tu contraseña
const char* serverIP = "192.168.0.50";   // IP del servidor local (cambiar)
const int serverPort = 3000;

// ==================== SENSORES ====================
// DHT22 o DHT11
#define DHTPIN 2          // Pin digital donde conectas DHT
#define DHTTYPE DHT22     // DHT22 (o DHT11 si usas ese)
DHT dht(DHTPIN, DHTTYPE);

// BMP280 o BME280 (usa I2C)
Adafruit_BMP280 bmp280;

// LDR (fotoresistencia)
#define LDR_PIN A0        // Pin analógico para LDR

// ==================== VARIABLES ====================
unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 10000; // 10 segundos

float temperature = 0;
float humidity = 0;
float pressure = 0;
int lightValue = 0;

WiFiClient client;

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n\nIniciando Estación Meteorológica...");
  
  // Inicializar sensores
  dht.begin();
  
  if (!bmp280.begin(0x76)) {  // Dirección I2C por defecto
    Serial.println("ERROR: No se pudo inicializar BMP280!");
    while (1) delay(10);
  }
  
  // Configurar BMP280
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                     Adafruit_BMP280::SAMPLING_X2,
                     Adafruit_BMP280::SAMPLING_X16,
                     Adafruit_BMP280::FILTER_X16,
                     Adafruit_BMP280::STANDBY_MS_500);
  
  // Conectar a WiFi
  connectToWiFi();
}

// ==================== LOOP ====================
void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Intentando reconectar...");
    connectToWiFi();
  }
  
  // Leer sensores cada READ_INTERVAL
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();
    
    readSensors();
    sendDataToServer();
    printDebugInfo();
  }
  
  delay(100);
}

// ==================== FUNCIONES ====================

void connectToWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nError: No se pudo conectar a WiFi");
  }
}

void readSensors() {
  // Leer DHT22
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Validar lectura DHT
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("ERROR: Fallo en lectura DHT");
    return;
  }
  
  // Leer BMP280
  pressure = bmp280.readPressure() / 100.0F;  // Convertir a hPa
  
  // Leer LDR
  lightValue = analogRead(LDR_PIN);
  
  Serial.println("Sensores leídos correctamente");
}

void sendDataToServer() {
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Conectado al servidor...");
    
    // Construir URL con parámetros
    String url = "/api/add?temp=" + String(temperature, 2) +
                 "&hum=" + String(humidity, 2) +
                 "&pres=" + String(pressure, 2) +
                 "&light=" + String(lightValue);
    
    // Enviar GET request
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(serverIP);
    client.print(":");
    client.println(serverPort);
    client.println("Connection: close");
    client.println();
    
    // Esperar respuesta
    unsigned long timeout = millis() + 5000;
    while (client.connected() && millis() < timeout) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    
    client.stop();
    Serial.println("Datos enviados exitosamente");
  } else {
    Serial.println("ERROR: No se pudo conectar al servidor");
  }
}

void printDebugInfo() {
  Serial.println("====== LECTURA ACTUAL ======");
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  Serial.print("Presión: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.print("Luz (ADC): ");
  Serial.print(lightValue);
  Serial.print(" / 1023 = ");
  Serial.print((lightValue / 1023.0) * 100);
  Serial.println(" %");
  
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("============================\n");
}
