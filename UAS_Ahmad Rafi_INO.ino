#include <EtherCard.h>
#include <Servo.h> // Tambahkan library Servo

// Konfigurasi IP statis
static byte myip[] = { 169, 254, 36, 216 }; // Alamat IP statis Arduino
static byte gwip[] = { 169, 254, 36, 215 }; // Alamat IP Gateway

// Alamat MAC Arduino
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

// Ukuran buffer Ethernet
byte Ethernet::buffer[700];

// Konten HTML halaman selamat datang (disimpan di PROGMEM)
const char welcomePage[] PROGMEM =
  "<!DOCTYPE html>"
  "<html lang='en'>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<title>UAS IOT</title>"
  "<style>"
  "body {"
  "font-family: Arial, sans-serif;"
  "background-color: #1A2130;"
  "text-align: center;"
  "padding: 20px;"
  "}"
  "h1 {"
  "color: #5A72A0;"
  "}"
  "p {"
  "color: #FDFFE2;"
  "}"
  ".danger {"
  "color: red;"
  "}"
  ".safe {"
  "color: green;"
  "}"
  "</style>"
  "</head>"
  "<body>"
  "<h1>SISTEM PENDETEKSI GAS</h1>"
  "<p>AHMAD RAFI | 2108096017</p>"
  "<p>Nilai Sensor MQ-2: %d</p>"
  "<p>Posisi Servo: %d Derajat</p>" // Tambahkan placeholder untuk posisi servo
  "<p class='%s'>Status: %s</p>" // Tambahkan placeholder untuk status LED
  "</body>"
  "</html>";

// Tentukan pin yang terhubung ke sensor MQ2
const int mq2Pin = A1; // Pin analog A1
// Tentukan pin yang terhubung ke servo
const int servoPin = A2; // Pin analog A2
// Tentukan pin yang terhubung ke LED
const int ledPinA3 = A3; // Pin analog A3
const int ledPinA4 = A4; // Pin analog A4

// Variabel untuk menyimpan nilai sensor sebelumnya
int previousValue = 0;
// Variabel untuk menyimpan posisi servo
int servoPosition = 0;
// Variabel untuk menyimpan status LED
String statusLED = "";

// Buat objek servo
Servo myServo;

void setup() {
  // Inisialisasi komunikasi serial
  Serial.begin(9600);

  // Lampirkan servo ke pin servo
  myServo.attach(servoPin);
  
  // Mulai komunikasi Ethernet dengan ukuran buffer dan alamat MAC
  ether.begin(sizeof Ethernet::buffer, mymac, SS);

  // Konfigurasi IP statis dan IP gateway
  ether.staticSetup(myip, gwip);
  
  // Atur pin LED sebagai OUTPUT
  pinMode(ledPinA3, OUTPUT);
  pinMode(ledPinA4, OUTPUT);
}

void loop() {
  // Baca nilai analog dari sensor MQ2
  int sensorValue = analogRead(mq2Pin);
  
  // Periksa apakah nilai sensor telah berubah
  if (sensorValue != previousValue) {
    // Cetak nilai sensor ke monitor serial
    Serial.print("Nilai sensor: ");
    Serial.println(sensorValue);
    
    // Perbarui nilai sensor sebelumnya
    previousValue = sensorValue;
    
    // Jika nilai sensor melebihi 500, pindahkan servo ke 120 derajat
    if (sensorValue > 500) {
      myServo.write(120);
      servoPosition = 120; // Perbarui variabel posisi servo
      // Nyalakan LED di pin A3
      digitalWrite(ledPinA3, HIGH);
      // Matikan LED di pin A4
      digitalWrite(ledPinA4, LOW);
      // Set status LED ke "bahaya"
      statusLED = "danger";
    } else {
      // Jika tidak, pindahkan servo ke 0 derajat
      myServo.write(0);
      servoPosition = 0; // Perbarui variabel posisi servo
      // Matikan LED di pin A3
      digitalWrite(ledPinA3, LOW);
      // Nyalakan LED di pin A4
      digitalWrite(ledPinA4, HIGH);
      // Set status LED ke "aman"
      statusLED = "safe";
    }
    
    // Cetak posisi servo ke monitor serial
    Serial.print("Posisi servo: ");
    Serial.print(servoPosition);
    Serial.println(" derajat");
  }
  
  // Tangani paket Ethernet masuk dan dapatkan posisi data
  word pos = ether.packetLoop(ether.packetReceive());

  // Jika data diterima
  if (pos) {
    // Ekstrak data dari buffer Ethernet
    char *data = (char *)Ethernet::buffer + pos;

    // Salin konten halaman selamat datang ke buffer Ethernet
    // dan ganti %d dengan nilai sensor, %d dengan posisi servo, dan %s dengan status LED
    sprintf_P(ether.tcpOffset(), welcomePage, sensorValue, servoPosition, statusLED.c_str(), statusLED.c_str());

    // Kirim tanggapan HTTP dengan halaman selamat datang ke klien
    ether.httpServerReply(strlen_P(welcomePage));
  }
}
