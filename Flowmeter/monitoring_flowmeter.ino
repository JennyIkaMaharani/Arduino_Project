#include <SD.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <SPI.h>


// Inisialisasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Inisialisasi RTC DS3231
RTC_DS3231 rtc;

// Konfigurasi Flow Meter
#define FLOW_SENSOR_PIN 2  // Harus menggunakan pin interrupt (D2 atau D3 untuk Uno)
volatile int pulseCount;   // Menyimpan jumlah pulsa dari flow meter
float flowRate;            // Laju aliran (L/min)
float totalVolume = 0;     // Accumulator (total air yang telah mengalir)
unsigned long lastTime;    // Untuk menyimpan waktu terakhir update

// Konfigurasi SD card
#define SD_CS 10  // Sesuaikan dengan pin CS SD card
File dataFile;

// ISR untuk menghitung pulsa dari flow meter
void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Flow Meter...");

  // Inisialisasi RTC
  if (!rtc.begin()) {
    Serial.println("RTC tidak terdeteksi!");
    while (1);
  }

  //Inisialisasi SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card gagal!");
    lcd.setCursor(0, 1);
    lcd.print("SD Error!");
    while (1);
  }
  
  // Konfigurasi flow meter
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);

  // Reset nilai
  pulseCount = 0;
  lastTime = millis();
  // Pastikan file ada atau buat baru
  
  // Membuka file di SD card
dataFile = SD.open("flowData.txt", FILE_WRITE);
if (!dataFile) {
  Serial.println("Gagal membuat file!");
} else {
  dataFile.println("Timestamp, Flow Rate (L/min), Total Volume (L)");
  dataFile.close();
}

}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTime;
DateTime now = rtc.now();
  if (elapsedTime >= 1000) { // Update setiap 1 detik
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));

    // Konversi pulsa ke laju aliran (Flow Rate)
    float kFactor = 343; // K-Factor dari flow meter (YF-S201)
    flowRate = (pulseCount / kFactor) * 60.0;

    // Tambahkan ke total volume (Accumulator)
    totalVolume += flowRate / 60.0;  // Karena ini setiap detik

    // Reset counter
    pulseCount = 0;
    lastTime = millis();

    // Tampilkan di Serial Monitor
    Serial.print("Flow Rate: ");
    Serial.print(flowRate);
    Serial.print(" L/min | Total: ");
    Serial.print(totalVolume);
    Serial.println(" L");
    
    // Tampilkan di LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Flow: ");
    lcd.print(flowRate);
    lcd.print(" L/m");
   
    lcd.setCursor(0, 1);
    lcd.print("Total: ");
    lcd.print(totalVolume);
    lcd.print(" L");

    // Pasang kembali interrupt
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);
  }

  // Simpan ke SD card setiap 10 menit
  static unsigned long lastSaveTime = 0;
  if (millis() - lastSaveTime >= 60000) { // 1 menit = 6.000 ms
    lastSaveTime = millis();

    // Ambil waktu dari RTC
     DateTime now = rtc.now();

    // Simpan data ke SD card
    dataFile = SD.open("flowData.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(now.timestamp());
      dataFile.print(", ");
      dataFile.print(flowRate);
      dataFile.print(" L/min, ");
      dataFile.print(totalVolume);
      dataFile.println(" L");
      dataFile.close();
      Serial.println("Data disimpan!");
    } else {
      Serial.println("Gagal menyimpan ke SD!");
    }
  }
 
}
