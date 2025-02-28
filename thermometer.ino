#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <max6675.h>

// Pin konfigurasi untuk MAX6675_1
#define MAX6675_CLK1 6
#define MAX6675_CS1 5
#define MAX6675_DO1 4

// Pin konfigurasi untuk MAX6675_2
#define MAX6675_CLK2 9
#define MAX6675_CS2 8
#define MAX6675_DO2 7

// Konfigurasi modul SD Card
#define SD_CS 10

// Inisialisasi objek
MAX6675 thermocouple1(MAX6675_CLK1, MAX6675_CS1, MAX6675_DO1);
MAX6675 thermocouple2(MAX6675_CLK2, MAX6675_CS2, MAX6675_DO2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

File dataFile;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  
  // RTC
  if (!rtc.begin()) {
    Serial.println("RTC failed!");
    while (1);
  }

  // SD Card
  if (!SD.begin(SD_CS)) {
    lcd.setCursor(0, 1);
    lcd.print("SD Fail");
    while (1);
  }

  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
}

void loop() {
  // Baca suhu
  double celsius = thermocouple1.readCelsius();
  double kelvin = celsius+273;
  double cel = thermocouple2.readCelsius();
  double kel = cel+273;

  // Ambil timestamp dari RTC
  DateTime now = rtc.now();

  // Tampilkan di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T1: ");
  lcd.print(celsius, 1);
  lcd.print("C/");
  
lcd.print(kelvin, 1);
lcd.print("K");

  lcd.setCursor(0, 1);
  lcd.print("T2: ");
  lcd.print(cel, 1);
  lcd.print("C/");
  
lcd.print(kel, 1);
lcd.print("K");

  // Simpan ke SD Card setiap 10 menit
  static unsigned long lastSaveTime = 0;
 if (millis() - lastSaveTime >= 600000) {
    lastSaveTime = millis();
   dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(now.timestamp(DateTime::TIMESTAMP_FULL));
      dataFile.print(", ");
      dataFile.print(celsius, 1);
      dataFile.print(" C, ");
      dataFile.print(kelvin, 1);
      dataFile.println(" F");
      dataFile.close();
      dataFile.print(cel, 1);
      dataFile.print(" C, ");
      dataFile.print(kel, 1);
      dataFile.println(" F");
      dataFile.close();
    }
  }

  delay(1000); // Perbarui setiap detik
}
