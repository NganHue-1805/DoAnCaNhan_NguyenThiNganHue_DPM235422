#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ThingSpeak.h"

// 1. Cấu hình Wi-Fi ảo của Wokwi 
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// 2. Thông tin kênh ThingSpeak 
unsigned long myChannelNumber = 3539563;          // Channel ID
const char* myWriteAPIKey = "DN75IH4H6BBUPYAE";   // Write API Key

// 3. Khai báo các chân đã nối trên mạch
#define MQ2_PIN     35  
#define RELAY_PIN   26  
#define LED_RED_PIN 4   
#define BUZZER_PIN  2   

#define GAS_THRESHOLD 2000 // Ngưỡng báo động (0 - 4095)

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClient client;

unsigned long lastTime = 0;
unsigned long timerDelay = 20000; // Gửi lên ThingSpeak mỗi 20 giây

void setup() {
  Serial.begin(115200);

  pinMode(MQ2_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Khởi động màn hình OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Khong tim thay man hinh OLED!"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.println("Dang ket noi WiFi...");
  display.display();

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nDa ket noi WiFi thanh cong!");

  ThingSpeak.begin(client);
}

void loop() {
  // Đọc giá trị cảm biến khí Gas
  int gasValue = analogRead(MQ2_PIN);
  int alarmStatus = 0;

  // Kiểm tra vượt ngưỡng
  if (gasValue >= GAS_THRESHOLD) {
    alarmStatus = 1;
    digitalWrite(LED_RED_PIN, HIGH); // Bật LED đỏ
    digitalWrite(RELAY_PIN, HIGH);   // Đóng Relay -> Sáng LED xanh
    tone(BUZZER_PIN, 1000);          // Kêu còi Buzzer
  } else {
    alarmStatus = 0;
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    noTone(BUZZER_PIN);              // Tắt còi
  }

  // Hiển thị lên màn hình OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CANH BAO KHI GAS");
  display.drawLine(0, 12, 128, 12, WHITE);

  display.setCursor(0, 25);
  display.print("Gia tri ADC: ");
  display.println(gasValue);

  display.setCursor(0, 45);
  display.print("Trang thai: ");
  if (alarmStatus == 1) {
    display.println("NGUY HIEM!");
  } else {
    display.println("AN TOAN");
  }
  display.display();

  // Gửi dữ liệu lên ThingSpeak mỗi 20 giây
  if ((millis() - lastTime) > timerDelay) {
    ThingSpeak.setField(1, gasValue);
    ThingSpeak.setField(2, alarmStatus);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Gui ThingSpeak thanh cong! Gia tri: " + String(gasValue));
    } else {
      Serial.println("Loi gui ThingSpeak, ma HTTP: " + String(x));
    }
    lastTime = millis();
  }

  delay(200);
}