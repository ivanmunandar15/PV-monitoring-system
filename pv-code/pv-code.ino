#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <Hash.h>

// Library untuk sensor DHT11
#include <DHT.h>

// Konfigurasi DHT11
#define DHTPIN 13     // Pin data sensor DHT11 (sesuaikan dengan pin yang Anda gunakan)
#define DHTTYPE DHT11 // Tipe sensor DHT (DHT11)

// Konfigurasi sensor arus ACS712
#define CURRENT_SENSOR_PIN A0    // Pin ADC untuk sensor arus
#define VOLTAGE_SENSOR_PIN 12    // Pin ADC untuk sensor tegangan (Gunakan pin D6/GPIO12)
float mVperAmp = 66;             // Sensitivitas ACS712 30A - 66mV/A
float offsetVoltage = 2500;      // Tegangan offset (2.5V untuk VCC 5V)

// Inisialisasi sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Konfigurasi WiFi dan Socket.IO
const char* ssid = "Delvi Wifi 2.4Ghz";
const char* password = "Delvi3107";
const char* socketServer = "192.168.1.9";
const int socketPort = 3000;

ESP8266WiFiMulti WiFiMulti;
SocketIOclient socketIO;

#define USE_SERIAL Serial

// Variabel untuk menyimpan data sensor
float temperature = 0;
float humidity = 0;
float current = 0;          // Arus dalam ampere
float voltage = 0;          // Tegangan dalam volt

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            USE_SERIAL.printf("[IOc] get event: %s\n", payload);
            break;
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

// Fungsi untuk membaca arus dari sensor ACS712
float readCurrent() {
    float avgADC = 0;
    
    // Ambil beberapa sampel untuk mengurangi noise
    for(int i = 0; i < 10; i++) {
        avgADC += analogRead(CURRENT_SENSOR_PIN);
        delay(5);
    }
    avgADC /= 10;
    
    // Konversi ADC ke mV (ESP8266 ADC maksimum 3.3V, resolusi 1023)
    float adcVoltage = avgADC * (3300.0 / 1023.0);
    
    // Konversi ke arus
    float currentValue = ((adcVoltage - offsetVoltage) / mVperAmp);
    
    // Jika arus terlalu kecil, anggap 0
    if(abs(currentValue) < 0.1) {
        currentValue = 0;
    }
    
    return currentValue;
}

// Fungsi untuk membaca tegangan (0-25V)
float readVoltage() {
    int sensorValue = analogRead(A0);
    
    // Konversi nilai ADC ke tegangan
    // Faktor kalibrasi: Sesuaikan berdasarkan pembagi tegangan yang Anda gunakan
    // Asumsi pembagi tegangan R1=30kΩ, R2=7.5kΩ (untuk membaca 0-25V)
    float calibrationFactor = 5.0; // Sesuaikan nilai ini berdasarkan pengukuran nyata
    
    return sensorValue * (3.3 / 1023.0) * calibrationFactor;
}

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    // Inisialisasi sensor DHT11
    dht.begin();
    USE_SERIAL.println("DHT11 sensor initialized");
    
    // Inisialisasi pin analog untuk sensor arus dan tegangan
    pinMode(CURRENT_SENSOR_PIN, INPUT);
    pinMode(VOLTAGE_SENSOR_PIN, INPUT);
    USE_SERIAL.println("Current and voltage sensors initialized");

    // Countdown sebelum memulai
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    // Nonaktifkan mode AP jika aktif
    if(WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
    }

    // Koneksi ke WiFi dengan kredensial yang diberikan
    WiFiMulti.addAP(ssid, password);

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    String ip = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

    // Konfigurasi Socket.IO server
    socketIO.begin(socketServer, socketPort, "/socket.io/?EIO=4");
    
    // Set callback untuk event Socket.IO
    socketIO.onEvent(socketIOEvent);
}

unsigned long sensorTimestamp = 0;    // create_at untuk membaca sensor
unsigned long messageTimestamp = 0;   // create_at untuk mengirim pesan

void loop() {
    socketIO.loop();

    uint64_t now = millis();

    // Baca sensor DHT11, arus, dan tegangan setiap 2 detik
    if(now - sensorTimestamp > 2000) {
        sensorTimestamp = now;

        // Baca nilai suhu dan kelembaban
        humidity = dht.readHumidity();
        temperature = dht.readTemperature();

        // Baca nilai arus dan tegangan
        current = readCurrent();
        voltage = readVoltage();

        // Cek apakah pembacaan DHT11 berhasil
        if (isnan(humidity) || isnan(temperature)) {
            USE_SERIAL.println("Gagal membaca data dari sensor DHT11!");
        } else {
            USE_SERIAL.printf("Suhu: %.1f°C, Kelembaban: %.1f%%\n", temperature, humidity);
        }
        
        // Tampilkan nilai arus dan tegangan
        USE_SERIAL.printf("Arus: %.2fA, Tegangan: %.2fV\n", current, voltage);
        
        // Hitung daya (Watt)
        float power = voltage * current;
        USE_SERIAL.printf("Daya: %.2fW\n", power);
    }

    // Kirim data ke server setiap 5 detik
    if(now - messageTimestamp > 5000) {
        messageTimestamp = now;

        // Buat JSON untuk data sensor
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();
        
        // Nama event: "sensor_data"
        array.add("sensor_data");
        
        // Parameter event (data sensor)
        JsonObject sensorData = array.createNestedObject();
        sensorData["temperature"] = temperature;
        sensorData["humidity"] = humidity;
        sensorData["current"] = current;
        sensorData["voltage"] = voltage;
        sensorData["power"] = voltage * current;
        sensorData["create_at"] = (uint32_t) now;
        sensorData["device_id"] = WiFi.macAddress(); // Gunakan MAC address sebagai ID perangkat
        
        // JSON ke String
        String output;
        serializeJson(doc, output);
        
        // Kirim event
        socketIO.sendEVENT(output);
        
        // Debug
        USE_SERIAL.println(output);
    }
}