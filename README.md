# 🌤️ Mini Estación Meteorológica - Arduino UNO R4 WiFi

Proyecto escolar de una estación meteorológica conectada a WiFi que muestra datos en tiempo real en una página web accesible desde cualquier dispositivo en la red local.

## 🔧 Hardware Necesario

- **Arduino UNO R4 WiFi**
- **Sensor DHT22** (o DHT11) - Temperatura y Humedad
- **Sensor BMP280** (o BME280) - Presión barométrica
- **Sensor LDR** (fotoresistencia) - Intensidad de luz
- **Pantalla LCD 16x2** con módulo I2C (opcional)
- **Protoboard + cables Dupont**
- **Fuente de alimentación 5V**
- **Caja plástica perforada**

## 💻 Software Necesario

- Arduino IDE
- Node.js (v14+)
- Git

## 🚀 Instalación

### 1. Clonar el repositorio
```bash
git clone https://github.com/FranSammauro/estacionMeteorologica.git
cd estacionMeteorologica
```

### 2. Instalar dependencias Node.js
```bash
npm install
npm install express socket.io sqlite3 cors
```

### 3. Iniciar el servidor
```bash
node server.js
```

El servidor estará disponible en: `http://localhost:3000` (o tu IP local)

### 4. Configurar Arduino

1. Abre Arduino IDE
2. Ve a **Archivo > Ejemplos > WiFiS3 > WiFiWebClient** para verificar WiFi
3. Abre el archivo `arduino/weather_station.ino`
4. **Configura estos parámetros:**
   ```cpp
   const char* ssid = "RED_WiFi";         // Tu SSID
   const char* password = "contraseña";   // Tu contraseña
   const char* serverIP = "192.168.0.50"; // IP del servidor
   ```
5. **Instala estas librerías:**
   - DHT sensor library (Adafruit)
   - Adafruit BMP280 Library
   - Wire (ya incluida)

6. Carga el sketch en el Arduino

## 📡 Conexiones de Hardware

### DHT22 (Temperatura/Humedad)
```
DHT22 VCC  → 5V
DHT22 GND  → GND
DHT22 DATA → Pin 2 (Arduino)
Resistencia 10kΩ entre VCC y DATA
```

### BMP280 (Presión) - I2C
```
BMP280 VCC → 5V
BMP280 GND → GND
BMP280 SCL → SCL (Pin A5 en UNO)
BMP280 SDA → SDA (Pin A4 en UNO)
```

### LDR (Luz)
```
LDR Pin 1 → 5V
LDR Pin 2 → A0 (Arduino) + Resistencia 10kΩ a GND
```

## 📊 API REST

### Enviar datos (desde Arduino)
```
GET /api/add?temp=25.4&hum=61&pres=1013&light=512
```

### Obtener último dato
```
GET /api/current
```

### Obtener historial (últimas 100 lecturas)
```
GET /api/history
```

### Obtener estadísticas
```
GET /api/stats
```

## 🌐 Acceso desde diferentes dispositivos

- **Mismo equipo del servidor**: `http://localhost:3000`
- **Otro equipo en la red**: `http://<IP_SERVIDOR>:3000`
  
Para encontrar la IP del servidor:
- **Windows**: `ipconfig` (busca IPv4)
- **macOS/Linux**: `ifconfig` (busca inet)

## 📁 Estructura del Proyecto

```
weatherstation/
├── server.js              # Servidor Express + Socket.io
├── package.json           # Dependencias
├── data/
│   └── weather.db        # Base de datos SQLite (se crea automáticamente)
├── public/
│   └── index.html        # Dashboard web
└── arduino/
    └── weather_station.ino # Código Arduino
```

## 🔌 Datos de Base de Datos

La tabla `readings` almacena:
- `id` - ID único
- `temperature` - Temperatura en °C
- `humidity` - Humedad en %
- `pressure` - Presión en hPa
- `light` - Valor LDR (0-1023)
- `timestamp` - Fecha y hora

## ⚙️ Configuración Avanzada

### Cambiar intervalo de lecturas (Arduino)
```cpp
const unsigned long READ_INTERVAL = 10000; // En milisegundos (10 segundos)
```

### Cambiar puerto del servidor
```bash
PORT=8080 node server.js
```

### Cambiar dirección I2C del BMP280
Si tu sensor no responde en 0x76, prueba con 0x77:
```cpp
if (!bmp280.begin(0x77)) { ... }
```

## 🐛 Troubleshooting

### Arduino no se conecta a WiFi
- Verifica SSID y contraseña (case-sensitive)
- Revisa que la red sea 2.4GHz (WiFi 6/5G no soportado)
- Asegúrate de usar Arduino UNO R4 WiFi (no R3)

### Servidor no recibe datos del Arduino
- Verifica que Arduino esté en la misma red WiFi
- Comprueba la IP del servidor: `ipconfig` / `ifconfig`
- Abre el monitor serie (9600 baud) para ver mensajes

### Los gráficos no se actualizan
- Abre la consola del navegador (F12) para ver errores
- Verifica que Socket.io esté conectado
- Recarga la página

## 📝 Licencia

MIT - Proyecto educativo

## 👤 Autor

Proyecto escolar - Francisco Sammauro