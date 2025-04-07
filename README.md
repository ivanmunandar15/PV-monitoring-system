# ☀️ Fullstack IoT Solar Panel Monitoring System

Sistem monitoring **panel surya** berbasis IoT yang mampu menampilkan data **suhu**, **tegangan**, **arus**, dan **daya** secara **real-time** menggunakan **ESP8266**, berbagai sensor, dan teknologi **WebSocket**, dengan tampilan web modern berbasis **Next.js** dan backend **Express.js**. Data juga disimpan ke **Supabase** untuk keperluan historis dan analisis.

---

## 🧰 Teknologi yang Digunakan

### ⚙️ Hardware:
- **ESP8266**: Mikrokontroler Wi-Fi
- **ACS712**: Sensor arus AC/DC
- **Voltage Sensor (0–25V)**: Pengukur tegangan dari panel surya
- **DHT11**: Sensor suhu dan kelembaban
- **MPPT Controller**: Mengatur titik daya maksimum dari panel surya

### 🖥️ Software:
- **Next.js**: Frontend antarmuka web (React)
- **Express.js**: Backend server (Node.js)
- **WebSocket**: Pengiriman data real-time
- **Supabase (PostgreSQL)**: Database penyimpanan historis
- **Chart.js / Recharts (opsional)**: Visualisasi data grafik


## ⚡ Fitur

- Monitoring data secara real-time:
  - Tegangan panel surya
  - Arus listrik
  - Daya (otomatis dihitung dari V x I)
  - Suhu & kelembaban
- Penyimpanan data historis ke **Supabase**
- Komunikasi cepat dan ringan menggunakan WebSocket
- Visualisasi data dengan grafik interaktif
- Antarmuka modern, ringan, dan responsif


---

## 📦 Struktur Proyek
solar-panel-monitoring/ ├── esp_code/ # Kode Arduino untuk ESP8266 ├── web_monitoring/ (submodule) # Kode website (Next.js + Express.js) └── README.md


## 🛠️ Instalasi & Setup

Ikuti langkah-langkah berikut untuk menjalankan sistem monitoring dari sisi perangkat IoT hingga tampilan web.

### 1. Clone Repositori
Clone repositori utama beserta submodule website:
git clone --recurse-submodules https://github.com/ivanmunandar15/solar-panel-monitoring.git
cd solar-panel-monitoring


### 2. Upload Kode ke ESP8266
- Gunakan Arduino IDE / PlatformIO
- Hubungkan ESP8266 ke sensor:
  - **ACS712** → Input arus dari panel
  - **Voltage Sensor** → Output panel surya (maks. 25V)
  - **DHT11** → Pin digital
  - **MPPT** → Terhubung ke panel dan battery/controller
- Atur:
  - SSID & password Wi-Fi
  - Alamat WebSocket server (IP dari backend)


### 3. Setup Supabase
- Buat akun di [https://supabase.com](https://supabase.com)
- Buat project dan tabel misalnya `monitoring_data` dengan field:
  - `id`, `voltage`, `current`, `power`, `temperature`, `humidity`, `created_at`
- Salin **anon public key** dan **project URL**

### 4. Jalankan Backend (Express.js)
cd web_server
npm install
npm run dev

### 5. Jalankan Backend (Express.js)
cd web_client
npm install
npm run dev


## 🔁 Struktur Data
[ESP8266 + Sensor + MPPT]
        │
     WebSocket
        ↓
[Express.js Backend] ──> Supabase (Store Data)
        │
     WebSocket
        ↓
[Next.js Frontend]


📌 Catatan
Pastikan ESP8266 dan server berada di jaringan yang sama.

Gunakan .env untuk konfigurasi Supabase di backend.

MPPT tidak dihubungkan langsung ke ESP, tetapi mempengaruhi data yang dibaca.

👨‍💻 Kontributor
Ivan Munandar – @ivanmunandar15
